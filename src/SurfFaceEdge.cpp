#include "SurfFaceEdge.h"

#include <SurfaceTool.hpp>
#include <Geometry.hpp>

#include <algorithm>
#include <iomanip>

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
        if(current) while((current = current->next)) count++;
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
Surface Surface::GreedyMeshLayer(std::vector<Face> faces, Direction dir, Vector3 center){
	const int CHUNK_DIMENSIONS = 16;
	struct Quad { float x, y, w, h; int blockID, i = -1; };

	// Surface storing the optimized layer
	Surface out;

	// Convert the faces into 2D quads
	std::vector<Quad> maskQuads; // Variable storing the flatened faces

	// Reduce the faces to their 2D representation
	#define reduce(first, second) for(Face& f: faces)\
		if(f.type == Face::Type::QUAD){\
			/* Make a quad with the top corner at the first point */\
			Quad q = {f.a.point.first - center.first, f.a.point.second - center.second,\
				f.c.point.first - f.a.point.first, f.c.point.second - f.a.point.second, f.blockID};\
			/* move the first point of the quad so that w and h are always positive */\
			if(q.w < 0) { q.x = f.c.point.first - center.first; q.w = std::abs(q.w); }\
			if(q.h < 0) { q.y = f.c.point.second - center.second; q.h = std::abs(q.h); }\
			maskQuads.push_back(q);\
		/* Triangular faces are passed straight out without being optimized */\
		} else\
			out += f.getSurface();
	switch(dir){
	case TOP: reduce(x, z); break;
	case BOTTOM: reduce(x, z); break;
	case NORTH: reduce(y, z); break;
	case SOUTH: reduce(y, z); break;
	case EAST: reduce(x, y); break;
	case WEST: reduce(x, y); break;
	}

	/*std::sort(quads.begin(), quads.end(), [](const Quad& a, const Quad& b) -> bool {
        if ( a.y != b.y )   return a.y < b.y;
        if ( a.x != b.x )   return a.x < b.x;
        if ( a.w != b.w )   return a.w > b.w;
        return a.h >= b.h;
	});
	for(int i = 0; i < quads.size(); i++)
		quads[i].i = i;*/

	// If there are any faces to be optimized
	if(maskQuads.size()){
		// Function which determines if a point is inside a quad
		auto withinQuad = [](const Quad& q, float x, float y){
			if(x >= q.x && x < q.x + q.w
			  && y >= q.y && y < q.y + q.h)
				return true;
			return false;
		};

		// Compute the mask
		int mask[CHUNK_DIMENSIONS * CHUNK_DIMENSIONS];
		bool found;
		for(int x = 0; x < CHUNK_DIMENSIONS; x++){
			for(int y = 0; y < CHUNK_DIMENSIONS; y++){
				found = false;
				for(Quad& q: maskQuads)
					if(withinQuad(q, x - 8, y - 8)){
						mask[x * CHUNK_DIMENSIONS + y] = q.blockID;
						found = true;
						break;
					}
				if(!found)
					mask[x * CHUNK_DIMENSIONS + y] = -1;
			}
		}

		// Generate Mesh
		// Code from: https://github.com/roboleary/GreedyMesh/blob/master/src/mygame/Main.java
		//std::vector<Quad> output;
		int n = 0, w, h;
	    for(int y = 0; y < CHUNK_DIMENSIONS; y++) {
	        for(int x = 0; x < CHUNK_DIMENSIONS;) {
	            if(mask[n] != -1) {
					// We compute the width
	                for(w = 1; x + w < CHUNK_DIMENSIONS && mask[n + w] != -1 && mask[n + w] == mask[n]; w++) {}

	                // Then we compute height
	                bool done = false;
	                for(h = 1; y + h < CHUNK_DIMENSIONS; h++) {
	                    for(int k = 0; k < w; k++)
	                        if(mask[n + k + h * CHUNK_DIMENSIONS] == -1 || mask[n + k + h * CHUNK_DIMENSIONS] != mask[n]) { done = true; break; }
	                    if(done) break;
	                }

					// Compute face
					switch(dir){
					case TOP:
						out += Face(Vector3(y - 8, 0, x - 8) + center,
								Vector3(y - 8 + h, 0, x - 8) + center,
								Vector3(y - 8 + h, 0, x - 8 + w) + center,
								Vector3(y - 8, 0, x - 8 + w) + center, mask[n]).getSurface();
						break;
					case BOTTOM:
						out += Face(Vector3(y - 8, 0, x - 8) + center,
								Vector3(y - 8 + h, 0, x - 8) + center,
								Vector3(y - 8 + h, 0, x - 8 + w) + center,
								Vector3(y - 8, 0, x - 8 + w) + center, mask[n]).reverse().getSurface();
						break;
					#warning not building mesh for other orientations
					}
					//output.push_back({(float) y, (float) x, (float) h, (float) w, mask[n]}); // names were messed up so fixing the order here
					//gout << "(" << y << ", " << x << ") sized " << h << "x" << w << endl;

	                // We zero out the mask
	                for(int l = 0; l < h; ++l)
	                    for(int k = 0; k < w; ++k)
							mask[n + k + l * CHUNK_DIMENSIONS] = -1;

	                // And then finally increment the counters and continue
	                x += w;
	                n += w;

	            } else {
					// Simply increment the counters
					x++;
					n++;
	            }
	        }
	    }

		/*for(int x = 0; x < CHUNK_DIMENSIONS; x++){
			for(int y = 0; y < CHUNK_DIMENSIONS; y++){
				found = false;
				for(Quad& q: output)
					if(within(q, x - 8, y - 8)){
						gout << std::setw(4) << q.i;
						found = true;
						break;
					}
				if(!found)
					gout << std::setw(4) << '.';
			}
			gout << endl;
		}*/

		//gout << output.size() << endl;

		//for(Quad& q: output){
		//	q.x -= 8; q.y -= 8;
		//	out
		//}
	}
	return out;

	/*Surface out;
	if(faces.size())
		for (Face& f: faces)
			out.append(f.getSurface());
	return out;*/
}

// Converts the surface into a wireframe representation
Spatial* Surface::getWireframe(){
	Spatial* out = Spatial::_new();
	for(int i = 0; i < indecies.size(); i += 3){
		SurfaceTool* line = SurfaceTool::_new();
		Vector3 normal = norms[indecies[i]].normalized() / 10000;
		line->begin(Mesh::PRIMITIVE_LINES);
		line->add_vertex(verts.read()[indecies.read()[i]] + normal);
		line->add_vertex(verts.read()[indecies.read()[i + 1]] + normal);
		line->add_vertex(verts.read()[indecies.read()[i + 1]] + normal);
		line->add_vertex(verts.read()[indecies.read()[i + 2]] + normal);
		line->add_vertex(verts.read()[indecies.read()[i + 2]] + normal);
		line->add_vertex(verts.read()[indecies.read()[i]] + normal);
		MeshInstance* instance = MeshInstance::_new();
		instance->set_mesh(line->commit());
		out->add_child(instance);
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
