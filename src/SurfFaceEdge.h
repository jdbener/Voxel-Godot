#ifndef _SURF_FACE_EDGE_H_
#define _SURF_FACE_EDGE_H_
#include "Godot.hpp"

#include "ArrayMesh.hpp"
#include "MeshInstance.hpp"

#include <vector>

// Add ranged based for loop support to PoolVector3Array and PoolIntArray
namespace godot{
	inline auto begin(godot::PoolVector3Array& what){
		return what.write().ptr();
	}

	inline auto end(godot::PoolVector3Array& what){
		return &what.write()[what.size()];
	}

// TODO: fix range based for loops to support PoolIntArrays
	inline auto begin(godot::PoolIntArray& what){
		return what.write().ptr();
	}

	inline auto end(godot::PoolIntArray& what){
		return &what.write()[what.size()];
	}
};

using namespace godot;

class Face;
class Edge;

class Surface{
public:
	// Positions
	PoolVector3Array verts;
  	// Normals
	PoolVector3Array norms;
	// Indecies
	PoolIntArray indecies;

	// Constructs a surface from a list of contiguous, coplanar, faces
	static Surface fromContiguousCoplanarFaces(std::vector<Face> faces);
	// Converts the surface into a mesh
	ArrayMesh* getMesh(ArrayMesh* mesh = nullptr);

	// Adds another surface to this one
	void append(Surface& other){
		int maxIndex = verts.size();

		for (Vector3& vert: other.verts)
			verts.push_back(vert);
		for (Vector3& normal: other.norms)
			norms.push_back(normal);
		for (int index: other.indecies)
			indecies.push_back(index + maxIndex);
	}

	void append(Surface&& other){
		append(other);
	}
};

class Face{
public:
	enum Type {TRIANGLE, QUAD};
	Face::Type type;

	// Vertecies
	Vector3 a, b, c, d;
	// Surface Normal
	Vector3 normal;

	// Gets a list of edges representing the outline of this face
	std::vector<Edge> getOutlineEdges();
	// Determines weather or not two faces are coplanar and contiguious
	bool checkContiguiousCoplanar(Face& other);
	// Constructs a surface from this face
	Surface getSurface();

	Face(Vector3 _a, Vector3 _b, Vector3 _c) : a(_a), b(_b), c(_c){
		type = Face::Type::TRIANGLE;
		calculateNormal();
	}

	Face(Vector3 _a, Vector3 _b, Vector3 _c, Vector3 _d) : a(_a), b(_b), c(_c), d(_d){
		type = Face::Type::QUAD;
		calculateNormal();
	}

	void calculateNormal(){
		normal = (c - a).cross(b - a).normalized();
	}

	bool checkContiguiousCoplanar(Face&& other){
		return checkContiguiousCoplanar(other);
	}

  /*PoolVector3Array getVertecies(){
  	PoolVector3Array verts;
  	if (type == Face::Type.TRIANGLE){
  		verts.push_back(a);
  		verts.push_back(b);
      verts.push_back(c);
    } else if (type == Face::Type.QUAD){
    	verts.push_back(a)
  		verts.push_back(b)
  		verts.push_back(c)
  		verts.push_back(d)
    }
    return verts;
  }*/
};

class Edge{
public:
	// Points representing the start and end of the edge
	Vector3 tail, tip;

	Edge(Vector3& _a, Vector3& _b){
		tail = _a;
		tip = _b;
	}

	// Function which returns a node containing a visualization of the edge
	MeshInstance* visualize();

	// Creates a direction vector centered at the origin parralel to this edge
	Vector3 direction(){
		return tip - tail;
	}

	// Checks if a point lies on this edge
	bool onEdge(const Vector3& p){
		if (tail.x <= std::max(p.x, tip.x) && tail.x >= std::min(p.x, tip.x) &&
		tail.y <= std::max(p.y, tip.y) && tail.y >= std::min(p.y, tip.y) &&
		tail.z <= std::max(p.z, tip.z) && tail.z >= std::min(p.z, tip.z))
			return true;
		return false;
	}

	// Checks if two Edges are equal
	bool operator==(const Edge& other) const {
		if ((tail == other.tail && tip == other.tip) || (tail == other.tip && tip == other.tail))
			return true;
		return false;
	}

	bool operator==(const Edge&& other) const {
		return *this == other;
	}

	// Function used to check if two edges share a point
	bool sharePoint(const Edge& other) const {
		if (tail == other.tail || tail == other.tip || tip == other.tail || tip == other.tip)
			return true;
		return false;
	}

	bool sharePoint(const Edge&& other) const {
		return sharePoint(other);
	}

  /*func overlap(other):
		# If the edges are parralel
		print(direction().normalized(), " - ", other.direction().normalized())
		if direction().normalized() == other.direction().normalized() or direction().normalized() == -other.direction().normalized():
			var connection = Edge.new(other.tail, tail)
			# If the edges are colinear
			if direction().normalized() == connection.direction().normalized() or direction().normalized() == -connection.direction().normalized():
				# If the edges overlap
				if onEdge(other.tip) or onEdge(other.tail) or other.onEdge(tip) or other.onEdge(tail):
					return true
		return false*/
};

#endif // _SURF_FACE_EDGE_H_
