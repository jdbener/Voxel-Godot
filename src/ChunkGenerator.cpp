/*
FILE:              ChunkGenerator.cpp
DESCRIPTION:       Procedures for generating new chunks

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
Joshua Dahl		   2010-01-08		  0.1 - Split code into cpp file
*/

#include "ChunkGenerator.hpp"

#include "types/BiomeList.hpp"

#include <string>
#include <fstream>

using namespace std;

/*
GENERATION: https://github.com/UnknownShadow200/ClassicalSharp/wiki/Minecraft-Classic-map-generation-algorithm
CAVES: https://www.youtube.com/watch?v=Df4Hidvq11M (cellular (worley) noise)
*/

// Constant representing a path to the folder where the world is stored
static const std::string worldPath = "worlds/Test/";
static const SimplexNoise noiseGen = SimplexNoise(1, 1, 2, .5);

/*
NAME:          setSolidityTest (Block& b)
DESCRIPTION:   Prototype chunk generator which uses layered simplex noise to generate a
				max block height within the interval [8, 20]
*/
void setSolidityTest (Block& b){
	const int max = 10,
            min = -2,
			seed = 12345;
    const float scale = 200;

	Vector3 center = b.getCenter();
    if(b.y < noiseGen.fractal(5, center.x / scale + seed, center.z / scale + seed) * max - (max - min) + 10)
        b.setBlockRef(2);

	if(b.x > -8 && b.x < -6)
		if(b.y > -4 && b.y < -2)
			if(b.z > -8 && b.z < -6)
				b.setBlockRef(2);

	if(b.x > -4 && b.x < -2)
		if(b.y > -4 && b.y < -2)
			if(b.z > -8 && b.z < -6)
				b.setBlockRef(2);

	/*Vector3 s = Vector3(-5, -5, -7);
		if(b.x > s.x-1 && b.x < s.x+1)
			if(b.y > s.y-1 && b.y < s.y+1)
				if(b.z > s.z-1 && b.z < s.z+1)
					b.setBlockRef(0);*/


	/*s = Vector3(-5, -5, -9);
	if(b.x > s.x-1 && b.x < s.x+1)
		if(b.y > s.y-1 && b.y < s.y+1)
			if(b.z > s.z-1 && b.z < s.z+1)
				b.setBlockRef(0);*/



	//if(center.y >= 14 && center.z < 15)
	//	b.setBlockRef(1);
}

/*
NAME:          generateChunk(Vector3 center, bool forceRegenerate, bool worldSpace)
DESCRIPTION:   Either loads the chunk centered at <center> from disc or generates a new one if loading isn't possible
NOTES:			Setting forceRegenerate to true will cause the function to never load from disc
*/
Chunk* generateChunk(Vector3 center, bool forceRegenerate, bool worldSpace){
	// Checks if this is the first time the function is running...
	static bool first = true;
	if(first){
		// if so, initialize the biome list
		BiomeList::initBiomeList();
		first = false;
	}

	// Create the name of the chunk file to load
    string filePath = worldPath + to_string((int) center.x) + "|" + to_string((int) center.y) + "|" + to_string((int) center.z) + ".chunk";
	// If we are taking in chunk space coordinates convert them to world space
    if(worldSpace) center *= 32;

	// Create a pointer to the new chunk and heap allocate it
    Chunk* out = new Chunk(center);
	// If the chunk file exists, load it
    if(ifstream(filePath).good() && !forceRegenerate){
        loadChunk(filePath.c_str(), *out);
		// Debug
	    Godot::print("Loaded chunk from: " + godotize(filePath));
	// Otherwise, generate a new chunk and save it to the file
	} else {
        out->runOnBlocks(setSolidityTest);
        saveChunk(filePath.c_str(), *out);
		// Debug
	    Godot::print("Saved chunk to: " + godotize(filePath));
    }

	Godotize::print( "ID of biome: " + to_string(BiomeList::getSurfaceBiome(60, 50, 0)->ID) );

	// Return the allocated chunk
    return out;
}
