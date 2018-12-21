/*
FILE:              ChunkMap.hpp
DESCRIPTION:       Header file for ChunkMap.cpp

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Godotized file (as part of that processes it became necessary to split off a header)
Joshua Dahl		   2018-12-17		  1.1 - Bug fixes on getSphere, exposed getInternalArray for use in GDScript, and split off the index fetching
                                        code in getPoint to getPointIndex
Joshua Dahl		   2018-12-19		  1.2 - Implemented getShereRim and getCubeRim, split the cube/sphere getters into two functions (one returning
                                        A vector and the other returning a Godot Array)
*/
#include <Godot.hpp>
#include <Spatial.hpp>
#include <Vector3.hpp>
#include <Array.hpp>

//#include <Chunk.hpp>

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

    inline Array getInternalArray();

	inline int getIndex(int x, int y);
    int getPointIndex(Vector3 search);
	inline Vector3 getPoint(Vector3 search);

    std::vector<Vector3> getCube(int radius, Vector3 origin = Vector3(0, 0, 0));
    std::vector<Vector3> getCubeRim(int radius, Vector3 origin = Vector3(0, 0, 0));
    std::vector<Vector3> getSphere(int radius, Vector3 origin = Vector3(0, 0, 0));
    std::vector<Vector3> getSphereRim(int radius, Vector3 origin = Vector3(0, 0, 0));

    Array getCubeExport(int radius, Vector3 origin = Vector3(0, 0, 0));
    Array getCubeRimExport(int radius, Vector3 origin = Vector3(0, 0, 0));
    Array getSphereExport(int radius, Vector3 origin = Vector3(0, 0, 0));
    Array getSphereRimExport(int radius, Vector3 origin = Vector3(0, 0, 0));

    ChunkMap(){

    }
private:
	void initSphere();
	void initIndex();
};
