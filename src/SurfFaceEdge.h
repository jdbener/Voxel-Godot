#ifndef _SURF_FACE_EDGE_H_
#define _SURF_FACE_EDGE_H_
#include <ArrayMesh.hpp>
#include <MeshInstance.hpp>

#include <vector>

#include "godot/Gstream.hpp"

enum Direction {NORTH, SOUTH, EAST, WEST, TOP, BOTTOM};

// Add ranged based for loop support to PoolVector3Array and PoolIntArray
namespace godot{
	#define __RANGED_BASED_FOR__(x) inline auto begin(godot::x& what){\
		return what.write().ptr();\
	}\
	inline auto end(godot::x& what){\
		return &what.write()[what.size()];\
	}

	__RANGED_BASED_FOR__(PoolVector3Array)
	__RANGED_BASED_FOR__(PoolVector2Array)
	__RANGED_BASED_FOR__(PoolColorArray)
	__RANGED_BASED_FOR__(PoolIntArray)
};

using namespace godot;

class Face;
class Edge;

struct Vertex {
	Vector3 point;
	Vector2 uv;
	Color color;

	Vertex(Vector3 p = Vector3(0, 0, 0), Vector2 u = Vector2(-1, -1), Color c = Color(-1, -1, -1, -1))
		: point(p), uv(u), color(c) {}

	operator Vector3(){ return point; }
	operator Vector2(){ return uv; }
	operator Color(){ return color; }
	bool operator==(Vertex& other) const {
		return point == other.point && uv == other.uv && color == other.color;
	}
};

class Surface{
public:
	// Positions
	PoolVector3Array verts;
  	// Normals
	PoolVector3Array norms;
	// UVs
	PoolVector2Array uvs;
	// Color
	PoolColorArray colors;
	// Indecies
	PoolIntArray indecies;

	// Adds another surface to this one
	void append(Surface& other);
	void append(Surface&& other){ append(other); }
	Surface& operator +=(Surface& other){ this->append(other); return *this; }
	Surface& operator +=(Surface&& other) {return *this += other; }

	// Constructs a surface from a list of contiguous, coplanar, faces
	static Surface GreedyMeshCoplanar(std::vector<Face> faces, Direction dir, Vector3 center);
	// Converts the surface into a mesh
	ArrayMesh* getMesh(ArrayMesh* mesh = nullptr);
	// Converts the surface into a wireframe representation
	Spatial* getWireframe();
};

class Face{
public:
	enum Type {TRIANGLE, QUAD};
	Face::Type type;

	// Vertecies
	Vertex a, b, c, d;
	// Surface Normal
	Vector3 normal;
	// Marker to differentiate faces of different block types
	int blockID;

	// Gets a list of edges representing the outline of this face
	std::vector<Edge> getOutlineEdges();
	// Determines weather or not two faces are coplanar and contiguious
	bool checkContiguiousCoplanar(Face& other);
	bool checkContiguiousCoplanar(Face&& other){ return checkContiguiousCoplanar(other); }
	// Determines if two faces have the same blockID
	bool checkType(Face& other){ return blockID == other.blockID; }
	bool checkType(Face&& other){ return checkType(other); }
	Face reverse(){
		if(type == Type::TRIANGLE)
			return Face(c, b, a, blockID);
		else
			return Face(d, c, b, a, blockID);
	}
	// Constructs a surface from this face
	Surface getSurface();

	bool operator==(Face& o) const {
		return a == o.a && b == o.b && c == o.c && d == o.d && blockID == o.blockID;
	}

	Face(Vertex _a, Vertex _b, Vertex _c, int bid = 0) : a(_a), b(_b), c(_c), blockID(bid) {
		type = Face::Type::TRIANGLE;
		calculateNormal();
	}

	Face(Vector3 _a, Vector3 _b, Vector3 _c, int bid = 0) : blockID(bid) {
		type = Face::Type::TRIANGLE;
		a = Vertex(_a);
		b = Vertex(_b);
		c = Vertex(_c);
		calculateNormal();
	}

	Face(Vertex _a, Vertex _b, Vertex _c, Vertex _d, int bid = 0) : a(_a), b(_b), c(_c), d(_d), blockID(bid){
		type = Face::Type::QUAD;
		calculateNormal();
	}

	Face(Vector3 _a, Vector3 _b, Vector3 _c, Vector3 _d, int bid = 0) : blockID(bid) {
		type = Face::Type::QUAD;
		a = Vertex(_a);
		b = Vertex(_b);
		c = Vertex(_c);
		d = Vertex(_d);
		calculateNormal();
	}

	void calculateNormal(){
		normal = (c.point - a.point).cross(b.point - a.point).normalized();
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
	Vertex tail, tip;

	Edge(Vector3& _a, Vector3& _b){
		tail.point = _a;
		tip.point = _b;
	}

	Edge(Vertex& a, Vertex& b){
		tail = a;
		tip = b;
	}

	// Function which returns a node containing a visualization of the edge
	MeshInstance* visualize();

	// Creates a direction vector centered at the origin parralel to this edge
	Vector3 direction(){
		return tip.point - tail.point;
	}

	// Checks if a point lies on this edge
	bool onEdge(const Vector3& p){
		if (tail.point.x <= std::max(p.x, tip.point.x) && tail.point.x >= std::min(p.x, tip.point.x) &&
		tail.point.y <= std::max(p.y, tip.point.y) && tail.point.y >= std::min(p.y, tip.point.y) &&
		tail.point.z <= std::max(p.z, tip.point.z) && tail.point.z >= std::min(p.z, tip.point.z))
			return true;
		return false;
	}

	// Checks if two Edges are equal
	bool operator==(const Edge& other) const {
		if ((tail.point == other.tail.point && tip.point == other.tip.point) || (tail.point == other.tip.point && tip.point == other.tail.point))
			return true;
		return false;
	}

	bool operator==(const Edge&& other) const {
		return *this == other;
	}

	// Function used to check if two edges share a point
	bool sharePoint(const Edge& other) const {
		if(tail.point == other.tail.point || tail.point == other.tip.point || tip.point == other.tail.point || tip.point == other.tip.point)
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
