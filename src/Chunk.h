#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <functional>

#include <MeshInstance.hpp>

#include "SurfaceOptimization.h"
#include "CerealGodot.h"

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
const int BLOCK_LEVEL = 0;

class VoxelInstance;
class Face;
typedef std::function<void(VoxelInstance*, int)> IterationFunction;

struct BlockData {
	enum Flags{
			TRANSPARENT = 1
	};
	unsigned char flags = 0;
	unsigned int blockID = 0;

	bool checkFlags(int mask){
		return (flags & mask) > 0;
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
		if(subVoxels) delete [] subVoxels;
		if(blockData) delete blockData;
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
	// Function which preforms all of the nessicary chunk calculations
	void recalculate(){
		calculateCenters();
		calculateVisibility();
	}

	//Function which runs the provided function for every instance recursively
	int iteraterate(int level, IterationFunction func_ptr, bool threaded = false){
		int index = 0;
		iteraterate(level, func_ptr, index, threaded);
		return index;
	}
	int iteraterateBlocks(IterationFunction func_ptr, bool threaded = false)
		{ return iteraterate(BLOCK_LEVEL, func_ptr, threaded); }
	// Function which returns true if the proived mask can be found in this instance's flags
	bool checkInstanceFlags(int mask){
		return (flags & mask) > 0;
	}

	// Serialization
	template<class Archive>
	void save(Archive& archive) const {
		archive(
			cereal::make_nvp("blockID", blockData->blockID),
			CEREAL_NVP(flags),
			CEREAL_NVP(level),
			CEREAL_NVP(center)
		);
		if(subVoxels){
			archive(cereal::make_nvp("hasChildren", true));
			for(int i = 0; i < 8; i++)
				archive( cereal::make_nvp((std::string("child-") + std::to_string(i)).c_str(), subVoxels[i]) );
		} else
			archive(cereal::make_nvp("hasChildren", false));
	}
	template<class Archive>
	void load(Archive& archive)
	{
		int blockID;
		archive(cereal::make_nvp("blockID", blockID));
		// TODO: load from database
		if(blockData) delete blockData;
		blockData = new BlockData();
		blockData->blockID = blockID;

		archive(
			CEREAL_NVP(flags),
			CEREAL_NVP(level),
			CEREAL_NVP(center)
		);

		bool hasChildren;
		archive(cereal::make_nvp("hasChildren", hasChildren));
		if(subVoxels) delete [] subVoxels;
		if(hasChildren){
			subVoxels = new VoxelInstance [8];
			for(int i = 0; i < 8; i++)
				archive( cereal::make_nvp((std::string("child-") + std::to_string(i)).c_str(), subVoxels[i]) );
		} else
			subVoxels = nullptr;
	}

	// Debug functions
	int count();
	String dump();

protected:
	// Function which loops over every block
	void iteraterate(int lvl, IterationFunction func_ptr, int& index, bool threaded);
	// Function which recursively calculates the center of all of the sub voxels
	void calculateCenters();
	// Function which recursively calculates the visibility of all the subVoxels
	void calculateVisibility();

};

class Chunk: public VoxelInstance, public MeshInstance {
	GODOT_CLASS(Chunk, MeshInstance)
public:
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

	void tick(){
		// do something here!
	}
};

#endif // CHUNK_H
