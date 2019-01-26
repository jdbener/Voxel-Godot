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
#include <cstdlib>

using namespace std;

/*
GENERATION: https://github.com/UnknownShadow200/ClassicalSharp/wiki/Minecraft-Classic-map-generation-algorithm
CAVES: https://www.youtube.com/watch?v=Df4Hidvq11M (cellular (worley) noise)
*/

// Constant representing a path to the folder where the world is stored
static const std::string worldPath = "worlds/Test/";
const int seed = 12345;

/*
NAME:          setSolidityTest (Block& b)
DESCRIPTION:   Prototype chunk generator which uses layered simplex noise to generate a
				max block height within the interval [8, 20]
*/
void generateBlock (Block& b){
	// Seed RNG
	srand(seed);

	// Get the center of the block
	Vector3 center = b.getCenter() / 2;
	// Get the humidty, temperature, and magicalness of the block
	float humidty = ((BiomeList::noiseGen.fractal(HUMIDITY_OCTAVES, center.x / HUMIDITY_SCALE + seed + HUMIDITY_OFFSET,
				center.z / HUMIDITY_SCALE + seed + HUMIDITY_OFFSET) + 1) / 2) * 100,
		  temperature = ((BiomeList::noiseGen.fractal(TEMPERATURE_OCTAVES, center.x / TEMPERATURE_SCALE + seed + TEMPERATURE_OFFSET,
			  	center.z / TEMPERATURE_SCALE + seed + TEMPERATURE_OFFSET) + 1) / 2) * 150 - 30,
		  magicalness = ((BiomeList::noiseGen.fractal(MAGICALNESS_OCTAVES, center.x / MAGICALNESS_SCALE + seed + MAGICALNESS_OFFSET,
			  center.z / MAGICALNESS_SCALE + seed + MAGICALNESS_OFFSET) + 1) / 2) * 100;
	//SurfaceRef* biome = BiomeList::getSurfaceBiome(humidty, temperature, magicalness);

	/*int surfaceLevel = SEE_LEVEL + biome->minLevel + (biome->generator->fractal(8, center.x / biome->noiseScale + seed,
								center.z / biome->noiseScale + seed) + 1) /2 * (biome->maxLevel - biome->minLevel);
	int transitionLevel = surfaceLevel - biome->transitionMinDepth - (biome->generator->fractal(TRANSITION_OCTAVES,
								 center.x / TRANSITION_SCALE + seed + TRANSITION_OFFSET, center.z / TRANSITION_SCALE + seed + TRANSITION_OFFSET) + 1) /
								  2 * (biome->transitionMaxDepth - biome->transitionMinDepth);*/

	SurfaceWeight closest = BiomeList::getSurfaceBiomes(humidty, temperature, magicalness);
	/*Godotize::print(to_string(closest.biome1->ID) + ": " + to_string(closest.weight1) + " - " +
					to_string(closest.biome2->ID) + ": " + to_string(closest.weight2) + " - " +
					to_string(closest.biome3->ID) + ": " + to_string(closest.weight3));*/



	float surfaceLevel = closest.getSurfaceLevel(center.x, center.z, seed);
	float transitionLevel = closest.getTransitionLevel(center.x, center.z, surfaceLevel, seed);
	SurfaceRef* biome = closest.biome1;


	if(center.y <= surfaceLevel){
		if(biome->surfaceBlocks.size() == 1){
			b.setBlockRef(biome->surfaceBlocks[0].block);
		} else if(!biome->surfaceBlocks.empty()){
			int block = rand() % biome->surfaceBlocks[biome->surfaceBlocks.size() - 1].chance;
			for(BlockChance cur: biome->surfaceBlocks)
				if(cur.chance <= block){
					b.setBlockRef(cur.block);
					break;
				}
		}
		if(center.y <= surfaceLevel - 1)
			if(center.y >= transitionLevel && biome->transitionMinDepth > 0){
				if(biome->transitionBlocks.size() == 1){
					b.setBlockRef(biome->transitionBlocks[0].block);
				} else if(!biome->transitionBlocks.empty()){
					int block = rand() % biome->transitionBlocks[biome->transitionBlocks.size() - 1].chance;
					for(BlockChance cur: biome->transitionBlocks)
						if(cur.chance <= block){
							b.setBlockRef(cur.block);
							break;
						}
				}
			} else {
				b.setBlockRef(2);
			}
	} else {
		b.setBlockRef(0);
	}


	/*if(b.x > -8 && b.x < -6)
		if(b.y > -4 && b.y < -2)
			if(b.z > -8 && b.z < -6)
				b.setBlockRef(2);

	if(b.x > -4 && b.x < -2)
		if(b.y > -4 && b.y < -2)
			if(b.z > -8 && b.z < -6)
				b.setBlockRef(2);

	Vector3 s = Vector3(-5, -5, -7);
		if(b.x > s.x-1 && b.x < s.x+1)
			if(b.y > s.y-1 && b.y < s.y+1)
				if(b.z > s.z-1 && b.z < s.z+1)
					b.setBlockRef(0);


	s = Vector3(-5, -5, -9);
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
        out->runOnBlocks(generateBlock);
        saveChunk(filePath.c_str(), *out);
		// Debug
	    Godot::print("Saved chunk to: " + godotize(filePath));
    }

	Godotize::print( "ID of biome: " + to_string(BiomeList::getSurfaceBiome(BiomeList::noiseGen.fractal(HUMIDITY_OCTAVES, center.x / HUMIDITY_SCALE + seed + HUMIDITY_OFFSET,
								center.z / HUMIDITY_SCALE + seed + HUMIDITY_OFFSET),
							BiomeList::noiseGen.fractal(TEMPERATURE_OCTAVES, center.x / TEMPERATURE_SCALE + seed + TEMPERATURE_OFFSET,
								center.z / TEMPERATURE_SCALE + seed + TEMPERATURE_OFFSET),
							BiomeList::noiseGen.fractal(MAGICALNESS_OCTAVES, center.x / MAGICALNESS_SCALE + seed + MAGICALNESS_OFFSET,
								center.z / MAGICALNESS_SCALE + seed + MAGICALNESS_OFFSET))->ID) );

	// Return the allocated chunk
    return out;
}
