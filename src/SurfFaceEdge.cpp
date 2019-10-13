#include "SurfFaceEdge.h"

#include "SurfaceTool.hpp"
#include "Geometry.hpp"

#include "timer.h"

/*------------------------
        Surface
------------------------*/

// Converts the surface into a mesh
ArrayMesh* Surface::getMesh(ArrayMesh* mesh /* = nullptr*/){
    if (!mesh) mesh = ArrayMesh::_new();

    Array arrays;
    arrays.resize(Mesh::ArrayType::ARRAY_MAX);

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

// Constructs a surface from a list of contiguous, coplanar, faces
Surface Surface::fromContiguousCoplanarFaces(std::vector<Face> faces){
    typedef std::vector<Edge*> EdgeVect;
    Surface surf;

    // Make list of edges
    std::vector<Edge> edges;
    for (Face& face: faces)
        for (Edge& edge: face.getOutlineEdges())
            edges.push_back(edge);

    // Remove interior Edges
    EdgeVect borderEdges;
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

    // Sort edges
    EdgeVect sortedBorderEdges(borderEdges.size());
    sortedBorderEdges[0] = borderEdges[0];
    borderEdges.erase(borderEdges.begin());
    for (int i = 1; i < sortedBorderEdges.size(); i++)
        for (int j = 0; j < borderEdges.size(); j++)
            if (borderEdges[j]->sharePoint(*sortedBorderEdges[i - 1]) and !(*borderEdges[j] == *sortedBorderEdges[i - 1])){
                sortedBorderEdges[i] = borderEdges[j];
                borderEdges.erase(borderEdges.begin() + j);
                break;
            }

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
    Timer t;
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
