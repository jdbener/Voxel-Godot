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
class Face;
typedef void (*IterationFunction)(VoxelInstance* me, int index);

struct BlockData {
	enum Flags{
			TRANSPARENT = 1
	};
	unsigned char flags = 0;
	unsigned int blockID = 0;

	bool checkFlags(int mask){
		return flags & mask > 0;
	}
};

class VoxelInstance {
public:
	enum Flags {
		/* Surface Visibility */
		TOP_VISIBLE = 1,
		BOTTOM_VISIBLE = 2,
		NORTH_VISIBLE = 4,
		SOUTH_VISIBLE = 8,
		EAST_VISIBLE = 16,
		WEST_VISIBLE = 32


	};
	BlockData* blockData;
	unsigned char flags = 0;

	//unsigned int blockID = 0;
	// variable storing which level of subchunk this instance is
	unsigned char level;
	Vector3 center = {0, 0, 0};

	VoxelInstance* subVoxels = nullptr;
	VoxelInstance* parent = nullptr;

	VoxelInstance(BlockData* data = new BlockData()){
		this->blockData = data;
	}

	// Copy constructor
	VoxelInstance(VoxelInstance& origin);
	// Move constructor
	VoxelInstance(VoxelInstance&& origin);

	~VoxelInstance(){
		if(subVoxels)
			delete [] subVoxels;
		if(blockData)
			delete blockData;
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
	VoxelInstance* find(int lvl, Vector3&& position) { return find(lvl, position); }
	// Function which determines if an arbitrary point in space is within this voxel
	bool within(Vector3& position);
	//Function which gets the visible faces from a voxel instance
	void getFaces(std::vector<Face>& out);
	std::vector<Face> getFaces(){
		std::vector<Face> out;
		getFaces(out);
		return out;
	}
	bool checkInstanceFlags(int mask){
		return flags & mask > 0;
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
	void calculateVisibility();

};

class Chunk: public VoxelInstance, public MeshInstance {
	GODOT_CLASS(Chunk, MeshInstance)
public:
	#warning TODO: replace itterater class with old recursive itteration code
	class Itterater {
	public:
		#define validate() if (index > pow(pow(2, level - 1) * 2,3) || index < 0) return false; \
			return true
		int index, level;
		Chunk* owner;

		Itterater(Chunk* o, int i = 0, int lvl = 0) : owner(o), index(i), level(lvl) {}

		bool operator+=(int dist){
			index += dist;
			validate();
		}

		bool operator++(){ return operator+=(1); }
		bool operator++(int){ return operator++(); }

		bool operator==(Itterater& other){ return index == other.index && owner == other.owner && level == other.level; }
		bool operator==(Itterater&& other){ return *this == other; }
		bool operator!=(Itterater& other){ return !(*this == other); }
		bool operator!=(Itterater&& other){ return *this != other; }

		VoxelInstance& operator*(){
			// Variable storing how far across a one voxel at this level is
			const int DIMENSIONS = pow(2, level);
			// Variable storing how long each dimmension of the "array" at this level is
			int ARRAY_LENGTH = pow(2, SUBCHUNK_LEVELS - level);
			if (level < 3) ARRAY_LENGTH++;
			const int UNMAPED_LENGTH = SUBCHUNK_LEVELS - level;

			int idx = index;
			gout << idx << " - " << DIMENSIONS << " - " << ARRAY_LENGTH << " <- ";
			int x = idx / (ARRAY_LENGTH * ARRAY_LENGTH);
    		idx -= (x * ARRAY_LENGTH * ARRAY_LENGTH);
			//#define remap(x) x = (ARRAY_LENGTH - x * ARRAY_LENGTH - ARRAY_LENGTH / 2) * DIMENSIONS
			#define remap(x) x = (UNMAPED_LENGTH - x - UNMAPED_LENGTH / 2); if(level < 3) x--; x *= DIMENSIONS;
			//#define remap(x) x = (ARRAY_LENGTH - x - ARRAY_LENGTH / 2)
			//#define remap(x) x = x;
			remap(x);
    		int y = idx / ARRAY_LENGTH;
			remap(y);
    		int z = idx % ARRAY_LENGTH;
			remap(z);
			gout << Vector3(x, y, z) << endl;
			return *owner->find(level, Vector3(x, y, z));
		}

		operator VoxelInstance(){
			return operator*();
		}

	};
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
		for(int i = 0; i < CHUNK_ARRAY_SIZE; i++)
			nullData[i] = 0;
		VoxelInstance::init(nullData);
		// Shouldn't be nessicary?
		VoxelInstance::calculateCenters();
	}

	Itterater begin(int level = 0){
		return Itterater(this, 0, level);
	}

	Itterater end(int level = 0){
		int DIMENSIONS = pow(2, SUBCHUNK_LEVELS - level);
		if (level < 3) DIMENSIONS++;
		return Itterater(this, DIMENSIONS * DIMENSIONS * DIMENSIONS, level);
	}

	void tick(){
		// do something here!
	}
};

#endif // CHUNK_H
