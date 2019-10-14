#ifndef CHUNK_H
#define CHUNK_H

#include <vector>

#include "SurfaceOptimization.h"

#include "Godot.hpp"
#include "MeshInstance.hpp"

using namespace godot;

/*
	data starts at +x, +y, +z corner,
	+y = up
	+x = north
	+z = east
*/

const int CHUNK_DIMENSIONS = 16; // The number of blocks which will make up one side of a chunk (16 ripped from minecraft ;)
const int CHUNK_ARRAY_SIZE = CHUNK_DIMENSIONS * CHUNK_DIMENSIONS * CHUNK_DIMENSIONS; // The total number of blocks in a chunk
const int SUBCHUNK_LEVELS = log(CHUNK_DIMENSIONS)/log(2); // The maximum number of octree levels in a chunk

class VoxelInstance;
typedef void (*IterationFunction)(VoxelInstance* me, int index);

struct BlockData {
	unsigned int blockID = 0;
	unsigned char flags = 0;
};

class VoxelInstance {
public:
	enum VoxelFlags {
		/* Surface Visibility */
		TOP_VISIBLE = 1,
		BOTTOM_VISIBLE = 2,
		NORTH_VISIBLE = 4,
		SOUTH_VISIBLE = 8,
		EAST_VISIBLE = 16,
		WEST_VISIBLE = 32


	};
	BlockData* data;
	unsigned char instanceData = 0;

	//unsigned int blockID = 0;
	// variable storing which level of subchunk this instance is
	unsigned char level;
	Vector3 center = {0, 0, 0};

	VoxelInstance* subVoxels = nullptr;
	VoxelInstance* parent = nullptr;

	VoxelInstance(BlockData* data = new BlockData()){
		this->data = data;
	}

	// Copy constructor
	VoxelInstance(VoxelInstance& origin);
	// Move constructor
	VoxelInstance(VoxelInstance&& origin);

	~VoxelInstance(){
		if(subVoxels){
			delete [] subVoxels;
			subVoxels = nullptr;
		}

		if(data){
			delete data;
			data = nullptr;
		}
	}

	// Function which recursiveley converts an array of blockIDs into an octree
	void init(int blocks[], int level = SUBCHUNK_LEVELS, bool originalCall = true);
	// Function which merges sublevels containing all of the same blockID into the same level
	bool prune();
	// Function which takes a pruned tree and rebuilds the lower levels of the tree down to the block level
	// The purpose of this function is to rebuild a branch of the tree when we need to modify a blockID in that branch
	void unprune(bool originalCall = true);
	// Function which given an arbitrary point in 3D space within the voxel
	// finds the subvoxel of the requested <lvl> which contains the point
	VoxelInstance* find(int lvl, Vector3& position);
	// Function which determines if an arbitrary point in space is within this voxel
	bool within(Vector3& position);

	std::vector<Face> getFaces(){
		/*if(data->flags & BlockData::VoxelFlags::TOP_VISIBLE){

		}*/
		return std::vector<Face>();
	}



	int iteraterate(int lvl, IterationFunction func_ptr){
		int index = 0;
		iteraterate(lvl, func_ptr, index);
		return index;
	}

	int iteraterateBlocks(IterationFunction func_ptr){
		return iteraterate(0, func_ptr);
	}

	// Function which preforms all of the nessicary chunk calculations
	void recalculate(){
		calculateCenters();
		calculateVisibility();
	}

	// Debug functions
	int count();
	String dump();

protected:
	void iteraterate(int lvl, IterationFunction func_ptr, int& index){
		if(subVoxels && lvl != level)
			for(int i = 0; i < 8; i++)
				subVoxels[i].iteraterate(lvl, func_ptr, index);
		else
			func_ptr(this, index++);
	}

	// Function which recursively calculates the center of all of the sub voxels
	void calculateCenters();
	// Function which recursively calculates the visibility of all the subVoxels
	void calculateVisibility(){
		/*if(level != SUBCHUNK_LEVELS){
			float distance =
		}*/


	}
};

class Chunk: public VoxelInstance, public MeshInstance {
	GODOT_CLASS(Chunk, MeshInstance)
public:
	bool loaded = false, rendered = false;

	static void _register_methods(){
        //register_method("_ready", &Chunk::_ready);
		register_method("initalize", &Chunk::initalize);
		register_method("recenter", &Chunk::recenter);
    }

	void _init(){}
	//void _ready(){}

	void recenter(){
		center = get_translation();
		VoxelInstance::calculateCenters();
	}

	void initalize(){
		int nullData[CHUNK_ARRAY_SIZE];
		for(int i = 0; i < CHUNK_ARRAY_SIZE; i++){
			nullData[i] = 0;
		}
		VoxelInstance::init(nullData);
	}

	/*Chunk(Vector3 center, bool initalize = false) {
		this->center = center;
		if(initalize){
			int nullData[CHUNK_ARRAY_SIZE];
			for(int i = 0; i < CHUNK_ARRAY_SIZE; i++){
				nullData[i] = 0;
			}
			VoxelInstance::init(nullData);
		}
	}

	Chunk(Vector3 center, int blocks[]){
		this->center = center;
		VoxelInstance::init(blocks);
		VoxelInstance::prune();
	}*/

	void tick(){
		// do something here!
	}
};

#endif // CHUNK_H
