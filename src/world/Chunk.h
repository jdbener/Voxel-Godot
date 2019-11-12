#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <functional>

#include <MeshInstance.hpp>

#include "../godot/CerealGodot.h"
#include "../block/BlockDatabase.h"
#include "../SurfFaceEdge.h"

#include "../godot/Gstream.hpp"

using namespace godot;

/*
	data starts at +x, +y, +z corner,
	+y = up
	+x = north
	+z = east
*/

const int CHUNK_DIMENSIONS = 16; // The number of blocks which will make up one side of a chunk
const int CHUNK_ARRAY_SIZE = CHUNK_DIMENSIONS * CHUNK_DIMENSIONS * CHUNK_DIMENSIONS; // The total number of blocks in a chunk
const int SUBCHUNK_LEVELS = log(CHUNK_DIMENSIONS)/log(2); // The maximum number of octree levels in a chunk
const int BLOCK_LEVEL = 0;

class VoxelInstance;
//class Face;
class ChunkMap;

enum Direction {NORTH, SOUTH, EAST, WEST, TOP, BOTTOM};
typedef std::function<void(VoxelInstance*, int)> IterationFunction;

class VoxelInstance {
public:
	enum Flags {
		/* Surface Visibility */
		null = 0,
		TOP_VISIBLE = 1,
		BOTTOM_VISIBLE = 2,
		NORTH_VISIBLE = 4,
		SOUTH_VISIBLE = 8,
		EAST_VISIBLE = 16,
		WEST_VISIBLE = 32


	};
	BlockData* blockData;
	flag_t flags = Flags::null;

	//unsigned int blockID = 0;
	// variable storing which level of subchunk this instance is
	unsigned char level;
	Vector3 center = {0, 0, 0};

	VoxelInstance* subVoxels = nullptr;
	//VoxelInstance* parent = nullptr;
	ChunkMap* map = nullptr;

	VoxelInstance(ChunkMap* map = nullptr, BlockData* data = new BlockData()){
		this->map = map;
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
	void init(int level = SUBCHUNK_LEVELS, bool originalCall = true);
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
	bool within(Vector3&& position){ return within(position); }
	// Function which gets a single face
	Face getFace(Direction d);
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

	// Function which gets all of the faces in one layer coming from a specified direction
	std::vector<Face> getLayerFaces(const Direction direction, const int whichLevel);

	//Function which runs the provided function for every instance recursively
	int iterate(int level, IterationFunction func_ptr, bool threaded = false){
		int index = 0;
		iterate(level, func_ptr, index, threaded);
		return index;
	}
	// Function which returns true if the proived mask can be found in this instance's flags
	bool checkFlag(Flags mask){
		return (flags & mask) == mask;
	}

	// Serialization
	template<class Archive>
	void save(Archive& archive) const {
		// Save instance
		archive(
			cereal::make_nvp("blockID", blockData->blockID),
			cereal::make_nvp("features", *blockData),
			CEREAL_NVP(flags),
			CEREAL_NVP(level),
			CEREAL_NVP(center)
		);

		// Save children
		if(subVoxels){
			archive(cereal::make_nvp("hasChildren", true));
			for(int i = 0; i < 8; i++)
				archive( cereal::make_nvp((std::string("child-") + std::to_string(i)).c_str(), subVoxels[i]) );
		} else
			archive(cereal::make_nvp("hasChildren", false));
	}
	template<class Archive>
	void load(Archive& archive) {
		// Load block data
		Identifier blockID;
		archive(cereal::make_nvp("blockID", blockID));
		if(blockData) delete blockData;
		blockData = BlockDatabase::getSingleton()->getBlock(blockID);

		// Load block instance
		archive(
			cereal::make_nvp("features", *blockData),
			CEREAL_NVP(flags),
			CEREAL_NVP(level),
			CEREAL_NVP(center)
		);

		// Load children
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
	void iterate(int lvl, IterationFunction func_ptr, int& index, bool threaded);
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
		//register_method("initalize", &Chunk::initalize);
		register_method("recenter", &Chunk::recenter);
		register_method("_process", &Chunk::_process);
    }

	static const bool DONT_INTIALIZE = false;
	Chunk() : VoxelInstance(nullptr) {}

	void _init(){}

	void _process(float delta){
		// do something here!
		//gout << delta << endl;
	}

	void initalize(ChunkMap* map){
		this->map = map;
		VoxelInstance::init();
	}

	// Implemented so that in the future we may generate chunks on the gpu?
	void loadFromArray(const std::vector<int>& array){
		if(!map) throw "Chunk Map not found";
		iterate(BLOCK_LEVEL, [&array](VoxelInstance* v, int i){
			int x = v->center.x + CHUNK_DIMENSIONS / 2;
			int y = v->center.y + CHUNK_DIMENSIONS / 2;
			int z = v->center.z + CHUNK_DIMENSIONS / 2;

			int index = x * CHUNK_DIMENSIONS * CHUNK_DIMENSIONS + z * CHUNK_DIMENSIONS + y;
			v->blockData = BlockDatabase::getSingleton()->getBlock(array[index]);
		});
		prune();
	}
	void loadFromArray(const std::vector<int>&& array){ loadFromArray(array); }

	void recenter(){
		center = get_translation();
		VoxelInstance::calculateCenters();
	}

	void rebuildMesh(int levelOfDetail = 0);
	void buildWireframe(int levelOfDetail = 0);
};

#endif // CHUNK_H
