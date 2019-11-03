#include "SurfFaceEdge.h"

#include <SurfaceTool.hpp>
#include <Geometry.hpp>

//#include "timer.h"

/*------------------------
        RemoveList
------------------------*/
// Linked list optimized for node removal
template <class T>
class RemoveList {
public:
    class Node {
    private:
        RemoveList* owner;
    public:
        T* data;
        Node* previous, *next;

        Node(RemoveList* owner){
            this->owner = owner;
            previous = nullptr;
            next = nullptr;
        }

        // Deletes this node and returns a pointer to the next node in the list
        Node* remove(){
            // Variable tracking a pointer to this node, it must be set in roundabout
            // ways since you can't delete a this pointer
            Node* toDelete;
            // Non-first node
            if(previous){
                toDelete = previous->next;
                previous->next = next;
                if(next) next->previous = previous;
            // First node
            } else {
                toDelete = owner->start;
                owner->start = next;
                if(next) next->previous = nullptr;
            }

            Node* out = next;
            if(toDelete) delete toDelete;
            // Return a pointer to the next node in the list
            return out;
        }

        // Inserts <what> into the list after this node. Returns a pointer to the
        // new node
        void insertAfter(T* what, Node* me = nullptr){
            // Save next node
            Node* oldNext = next;
            // Figure out current node address (if not specified)
            if(!me)
                if(next)
                    me = next->previous;
                else if(previous)
                    me = previous->next;

            // Replace next
            next = new Node(owner);
            next->data = what;
            next->previous = me;
            next->next = oldNext;

            if(owner->end == me) owner->end = next;
        }

        T& operator*(){ return *data; }
        // Seams unessisary but when removed code breaks
        operator T(){ return *data; }
    };

    Node *start, *end;
    RemoveList() : start(nullptr), end(nullptr) {}
    RemoveList(T* begin, T* end){
        start = new Node(this);
        Node* current = start;
        while(begin <= end){
            current->data = begin;
            begin++;
            if(begin <= end){
                current->next = new Node(this);
                current->next->previous = current;
                current = current->next;
            }
        }
        this->end = current;
    }

    ~RemoveList(){
        while(start){
            Node* toDelete = start;
            start = start->next;
            delete toDelete;
        }
    }

    void push_back(T* what){
        if(end)
            end->insertAfter(what, end);
        else {
            end = start = new Node(this);
            end->data = what;
        }
    }

    size_t size(){
        size_t count = 0;
        Node* current = start;
        if(current) while(current = current->next) count++;
        return count;
    }
};

/*------------------------
        Surface
------------------------*/

// Converts the surface into a mesh
ArrayMesh* Surface::getMesh(ArrayMesh* mesh /* = nullptr*/){
    if (!mesh) mesh = ArrayMesh::_new();

    Array arrays;
    arrays.resize(Mesh::ArrayType::ARRAY_MAX);

    if(verts.size() > 0)
        arrays[Mesh::ArrayType::ARRAY_VERTEX] = verts;
    if (norms.size() > 0)
        arrays[Mesh::ArrayType::ARRAY_NORMAL] = norms;
    if (uvs.size() > 0)
        arrays[Mesh::ArrayType::ARRAY_TEX_UV] = uvs;
    if (colors.size() > 0)
        arrays[Mesh::ArrayType::ARRAY_COLOR] = colors;
    if (indecies.size() > 0)
        arrays[Mesh::ArrayType::ARRAY_INDEX] = indecies;

    mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, arrays);
    return mesh;
}

// Adds another surface to this one
void Surface::append(Surface& other){
    int maxIndex = verts.size();

    for (Vector3& vert: other.verts)
        verts.push_back(vert);
    for (Vector3& normal: other.norms)
        norms.push_back(normal);
    for (Vector2& uv: other.uvs)
        uvs.push_back(uv);
    for (Color& color: other.colors)
        colors.push_back(color);
    for (int index: other.indecies)
        indecies.push_back(index + maxIndex);
}

// Constructs a surface from a list of contiguous, coplanar, faces
Surface Surface::fromContiguousCoplanarFaces(std::vector<Face> faces){
    // Don't spend resources if the vector only contains one face
    if(faces.size() == 1)
        return faces[0].getSurface();

    // Make list of edges
    std::vector<Edge> edges;
    for (Face& face: faces)
        for (Edge& edge: face.getOutlineEdges())
            edges.push_back(edge);

    // Remove interior Edges
    RemoveList<Edge> borderEdges; // RemoveList saves ~10-20μs
    // Lambda counting the number of times each edge occures
    auto edgeOccurences = [edges](Edge& what){
        int occurences = 0;
        for(const Edge& edge: edges)
            if (edge == what)
                occurences += 1;
        return occurences;
    };
    for (Edge& edge: edges)
        // An edge is a border edge only if it occures once (no other faces share the egde)
        if (edgeOccurences(edge) == 1)
            borderEdges.push_back(&edge);

    if(!borderEdges.size())
        return Surface();

    // Sort edges
    std::vector<Edge*> sortedBorderEdges(borderEdges.size());
    sortedBorderEdges[0] = borderEdges.start->data;
    borderEdges.start->remove();
    for (int i = 1; i < sortedBorderEdges.size(); i++){
        for (auto edge = borderEdges.start; edge; edge = edge->next)
            if (edge->data->sharePoint(*sortedBorderEdges[i - 1]) && !(*edge->data == *sortedBorderEdges[i - 1])){
                sortedBorderEdges[i] = edge->data;
                edge->remove();
                break;
            }
    }

    Surface surf;
    // Extract verticies, removing those breaking straight edges
    for (int i = 0; i < sortedBorderEdges.size(); i++){
        bool keep = false;
        // Keep the edge if the vertex of the previous edge is not colinear to this edge
        if (i > 0){
            if (sortedBorderEdges[i - 1]->direction().normalized() != sortedBorderEdges[i]->direction().normalized())
                keep = true;
        // For the first vertex, we assume a closed loop so the previous edge is the last edge
        } else if (sortedBorderEdges[sortedBorderEdges.size() - 1]->direction().normalized() != sortedBorderEdges[i]->direction().normalized())
                keep = true;
        // If we are keeping the edge... extract the vertex
        if (keep){
            surf.verts.push_back(sortedBorderEdges[i]->tail);
            surf.norms.push_back(faces[0].normal);
        }
    }

    // Flatten all of the vertecies in 3D space and drop their normal facing component
    // This is required since the built in triangulation algorithm only works in 2D space
    Basis transform = Basis(faces[0].b.point - faces[0].a.point, faces[0].normal, faces[0].c.point - faces[0].a.point);
    PoolVector2Array triangulationPoints;
    for (Vector3 p: surf.verts){
        p = transform.xform_inv(p);
        triangulationPoints.push_back(Vector2(p.x, p.z));
    }

    // Triangulate the surface based on the reprojected points
    surf.indecies = Geometry::get_singleton()->triangulate_polygon(triangulationPoints);
    // Try when 3.2 comes out:
    // surf.indecies = Geometry::get_singleton()->triangulate_delaunay_2d(triangulationPoints);
    return surf;
}

// Constructs a surface from an arbitrary list of faces
Surface Surface::fromFaces(std::vector<Face> _faces){
    Surface out;
    // Convert the input to a removal optimized linked list
    RemoveList<Face> faces(&_faces.front(), &_faces.back());
    auto face = faces.start;
    // For every input face...
    while(face){
        // Create a vector storing all of the faces contiguous to this face
        std::vector<Face> contiguous = {*face};
        // For every other face...
        for(auto faceCompare = face->next; faceCompare; faceCompare = faceCompare->next)
            // Check if this face is contiguous to one of our current faces of the same type
            for(Face& confirmedFace: contiguous){
                if(confirmedFace.checkContiguiousCoplanar(*faceCompare) && confirmedFace.checkType(*faceCompare)){
                    // If so add it to the vector
                    contiguous.push_back(*faceCompare);
                    // And remove it from the input set
                    faceCompare->remove();
                    break;
                }
            }

        // Remove the original face from the input
        face = face->remove();
        // Construct the contiguous surface and add it to the current one
        out.append(Surface::fromContiguousCoplanarFaces(contiguous));
    }
    return out;
}

/*------------------------
        Face
------------------------*/

// Gets a list of edges representing the outline of this face
std::vector<Edge> Face::getOutlineEdges(){
    std::vector<Edge> out;
    if (type == Face::Type::TRIANGLE) {
        out.push_back(Edge(a, b));
        out.push_back(Edge(b, c));
        out.push_back(Edge(c, a));
    } else if(type == Face::Type::QUAD) {
        out.push_back(Edge(a, b));
        out.push_back(Edge(b, c));
        out.push_back(Edge(c, d));
        out.push_back(Edge(d, a));
    }
    return out;
}

// Determines weather or not two faces are coplanar and contiguious
bool Face::checkContiguiousCoplanar(Face& other){
    // If the normals are different the faces can't be coplanar
    if(normal != other.normal) return false;

    std::vector<Edge> edges = getOutlineEdges();
    std::vector<Edge> otherEdges = other.getOutlineEdges();

    for (Edge& edge: edges)
        for(Edge& other: otherEdges)
            if(edge == other)
                return true;
    return false;
}

// Constructs a surface from this face
Surface Face::getSurface(){
    Surface surf;
    if (type == Face::Type::TRIANGLE){
        surf.verts.push_back(a);
        surf.norms.push_back(normal);
        surf.indecies.push_back(0);
        surf.verts.push_back(b);
        surf.norms.push_back(normal);
        surf.indecies.push_back(1);
        surf.verts.push_back(c);
        surf.norms.push_back(normal);
        surf.indecies.push_back(2);
    } else if (type == Face::Type::QUAD){
        surf.verts.push_back(a);
        surf.norms.push_back(normal);
        surf.verts.push_back(b);
        surf.norms.push_back(normal);
        surf.verts.push_back(c);
        surf.norms.push_back(normal);
        surf.verts.push_back(d);
        surf.norms.push_back(normal);

        surf.indecies.push_back(0);
        surf.indecies.push_back(1);
        surf.indecies.push_back(2);
        surf.indecies.push_back(2);
        surf.indecies.push_back(3);
        surf.indecies.push_back(0);
    }
    return surf;
}

/*------------------------
        Edge
------------------------*/

// Function which returns a node containing a visualization of the edge
MeshInstance* Edge::visualize(){
    SurfaceTool* line = SurfaceTool::_new();
    line->begin(Mesh::PRIMITIVE_LINES);
    line->add_vertex(tail);
    line->add_vertex(tip);
    MeshInstance* mi = MeshInstance::_new();
    mi->set_mesh(line->commit());
    return mi;
}
