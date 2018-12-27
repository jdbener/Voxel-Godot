/*
FILE:              ChunkGenerator.hpp
DESCRIPTION:       Procedures for generating new chunks

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
*/

#ifndef CHUNKGENERATOR_H
#define CHUNKGENERATOR_H

#include "world/Chunk.hpp"
#include <SimplexNoise/src/SimplexNoise.h>

#include <string>

#include "ChunkGenerator.hpp"
#include "Godotize.hpp"

#include <fstream>

using namespace std;


// Constant representing a path to the folder where the world is stored
static const std::string worldPath = "worlds/Test/";
// Instance of SimplexNoise library for temp world gen
static SimplexNoise noiseGen = SimplexNoise(1, 1, 2, .5);

/*
NAME:          setSolidityTest (Block& b)
DESCRIPTION:   Prototype chunk generator which uses layered simplex noise to generate a
				max block height within the interval [8, 20]
*/
static void setSolidityTest (Block& b){
	const int max = 10,
            min = -2,
			seed = 12345;
    const float scale = 200;

	Vector3 center = b.getCenter();
    if(b.y < noiseGen.fractal(5, center.x / scale + seed, center.z / scale + seed) * max - (max - min) + 10)
        b.setBlockRef(2);

	//if(center.y >= 14 && center.z < 15)
	//	b.setBlockRef(1);
}

/*
NAME:          generateChunk(Vector3 center, bool forceRegenerate, bool worldSpace)
DESCRIPTION:   Either loads the chunk centered at <center> from disc or generates a new one if loading isn't possible
NOTES:			Setting forceRegenerate to true will cause the function to never load from disc
*/
static Chunk* generateChunk(Vector3 center, bool forceRegenerate = false, bool worldSpace = true){
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
	// Return the allocated chunk
    return out;
}
;
#endif
