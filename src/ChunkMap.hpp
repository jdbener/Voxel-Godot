/*
FILE:              ChunkMap.hpp
DESCRIPTION:       Header file for ChunkMap.cpp

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Godotized file (as part of that processes it became necessary to split off a header)
*/
#include <Godot.hpp>
#include <Spatial.hpp>
#include <Vector3.hpp>
#include <Array.hpp>

#include <map>
#include <vector>

using namespace godot;

// Constant representing a number getIndex returns when the searched for index doesn't exist
const int NULL_INDEX = -999;
const Vector3 NULL_VECTOR(-999, -999, -999);

class ChunkMap: public Spatial {
  GODOT_CLASS(ChunkMap, Spatial);
public:
  void _init();
	static void _register_methods();

	int radius;	// Variable storing the radius of the sphere
	Vector3 origin; // Variable storing the origin of the sphere (defaults to (0, 0, 0))
	std::vector<Vector3> sphere; // Array storing the elements which make up the sphere
	std::map<short, std::map<short, int>> index; // Stores the number of elements at the end of each x/y location

	void _enter_tree();
	void _exit_tree();

	void reinitSphere(Vector3 translation);

	int getIndex(int x, int y);
	Vector3 getPoint(Vector3 search);

	Array getCube(int radius, Vector3 origin = Vector3(0, 0, 0));
	Array getSphere(int radius, Vector3 origin = Vector3(0, 0, 0));

private:
	void initSphere();
	void initIndex();
};
