#ifndef CHUNK_H
#define CHUNK_H

#include <cmath>
#include <map>

struct vec3 {
	float x, y, z;

	vec3 operator+(vec3& other){
		return vec3 {other.x + x, other.y + y, other.z + z};
	}

	vec3 operator+(vec3&& other){
		return vec3 {other.x + x, other.y + y, other.z + z};
	}

	vec3 operator-(vec3& other){
		return vec3 {x - other.x, y - other.y, z - other.z};
	}

	vec3 operator-(vec3&& other){
		return vec3 {x - other.x, y - other.y, z - other.z};
	}

	std::string to_string(){
		return "{" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "} ";
	}
};

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
	enum VoxelFlags {
		/* Surface Visibility */
		TOP_VISIBLE = 1,
		BOTTOM_VISIBLE = 2,
		NORTH_VISIBLE = 4,
		SOUTH_VISIBLE = 8,
		EAST_VISIBLE = 16,
		WEST_VISIBLE = 32


	};

	unsigned char flags = 0;
};

class VoxelInstance {
public:
	BlockData* data;
	unsigned int blockID = 0;
	unsigned char level;
	vec3 center = {0, 0, 0};

	VoxelInstance* subVoxels = nullptr;
	VoxelInstance* parent = nullptr;

	VoxelInstance(BlockData* data = new BlockData()){
		this->data = data;
	}

	// Copy constructor
	VoxelInstance(VoxelInstance& origin){
		data = new BlockData();
		*data = *origin.data;

		blockID = origin.blockID;
		level = origin.level;
		center = origin.center;
		subVoxels = origin.subVoxels;
		parent = origin.parent;
	}

	// Move constructor
	VoxelInstance(VoxelInstance&& origin){
		data = new BlockData();
		*data = *origin.data;

		blockID = origin.blockID;
		level = origin.level;
		center = origin.center;
		subVoxels = origin.subVoxels;
		parent = origin.parent;
	}

	~VoxelInstance(){
		if(subVoxels){
			/*for(int i = 0; i < 8; i++){
				delete subVoxels[i];
			}*/
			delete [] subVoxels;
			subVoxels = nullptr;
		}

		if(data){
			delete data;
			data = nullptr;
		}
	}

	void init(int blocks[], int level = SUBCHUNK_LEVELS){
		this->level = level; // Mark which sublevel this instance is

		// If there is data to initalize and we aren't dealing with leaf nodes...
		if(blocks && level > 0){
				subVoxels = new VoxelInstance [8]; // Create the next sublevels

			// Determine how many blocks are in the source array at this level
			int numLeft = pow(2, level);
			numLeft *= numLeft * numLeft;

			// Distribute the blocks over the child sublevels
			for(int i = 0; i < 8; i++){
				subVoxels[i].parent = this; // Mark this sublevel as the parent of the child sublevels
				subVoxels[i].init(&blocks[i * (numLeft / 8)], level - 1);
			}
		// If we are dealing with leaf nodes...
		} else
			blockID = blocks[0];

		// If this was the original call recalculate centers
		if(level == SUBCHUNK_LEVELS)
			calculateCenters();
	}

	// Function which merges sublevels containing all of the same blockID into the same level
	bool prune(){
		// Variable which ensures that small data changes not large enouph to change the mode of a sublevel
		// don't get pruned
		bool canPrune = true;
		if(level > 1 && subVoxels)
			for(size_t i = 0; i < 8; i++)
				if(!subVoxels[i].prune())
					canPrune = false;

		uint count = 0;	// Number of times the most common blockID appears in the sublevels
		if(subVoxels){ // Make sure there are sublevels before finding the mode of the sublevels
			std::map<uint, uint> m; // Map used to sort blockIDs
			// Store the blockIDs sorted by occurence in a map
			for(size_t i = 0; i < 8; i++) {
				uint key = subVoxels[i].blockID;
				std::map<uint, uint>::iterator it = m.find(key);
				if(it == m.end())
					m.insert(std::make_pair(key, 1));
				else
					it->second++;
			}
			// Find the most common blockID in the sublevels and store it as this level's blockID
			for(auto& it: m)
				if(it.second > count) {
					count = it.second;
					blockID = it.first;
				}
		} else
			return true; // If we have already pruned this branch we are safe to prune higher

		// If the sublevels all have the same blockID, they can be removed
		if(count == 8 && canPrune){
			delete [] subVoxels;
			subVoxels = nullptr;
			return true; // We are safe to prune higher up
		}
		return false; // We are not safe to prune higher up
	}

	// Function which takes a pruned tree and rebuilds the lower levels of the tree down to the block level
	// The purpose of this function is to rebuild a branch of the tree when we need to modify a blockID in that branch
	void unprune(bool top = true){
		// If we don't have children and we aren't at the block level
		if(level > 0 && !subVoxels){
			subVoxels = new VoxelInstance [8]; // Create the next sublevels

			// Pass down the blockID to all the new children and unprune them
			for(int i = 0; i < 8; i++){
				subVoxels[i].parent = this;
				subVoxels[i].blockID = blockID;
				subVoxels[i].level = level - 1;
				*subVoxels[i].data = *data;
				subVoxels[i].unprune(false);
			}
		// If we do have children and we aren't at the block level
		} else if (level > 0)
			// Unprune the children
			for(int i = 0; i < 8; i++)
				subVoxels[i].unprune(false);

		// If this is the original function call... recalculate the centers of the newly expanded tree
		if(top)
			calculateCenters();
	}

	void calculateCenters(){
		if(subVoxels){
			float change = .5 * pow(2, level-1);

			subVoxels[0].center = center + vec3 {change, change, change};
			subVoxels[1].center = center + vec3 {change, change, -change};
			subVoxels[2].center = center + vec3 {-change, change, -change};
			subVoxels[3].center = center + vec3 {-change, change, change};
			subVoxels[4].center = center + vec3 {change, -change, change};
			subVoxels[5].center = center + vec3 {change, -change, -change};
			subVoxels[6].center = center + vec3 {-change, -change, -change};
			subVoxels[7].center = center + vec3 {-change, -change, change};

			for(int i = 0; i < 8; i++)
				subVoxels[i].calculateCenters();
		}
	}

	VoxelInstance* find(int lvl, vec3 position){
		// If we are at the desired level... return a reference to this node
		if(lvl == level){
			return this;
		}

		// If this node has children...
		if(subVoxels)
			for(int i = 0; i < 8; i++)
				// Check if each child encloses the search position...
				if(subVoxels[i].within(position))
					// If it does recursively search into that child
					return subVoxels[i].find(lvl, position);
		// If we are at a leaf node... return a reference to this node
		return this;
	}

	int iteraterate(int lvl, IterationFunction func_ptr){
		int index = 0;
		iteraterate(lvl, func_ptr, index);
		return index;
	}

	int iteraterateBlocks(IterationFunction func_ptr){
		return iteraterate(0, func_ptr);
	}

	bool within(vec3 position){
		int bounds = pow(2, level - 1);
		if(bounds < 1) bounds = 1;
		return !(position.x <= center.x - bounds || position.x >= center.x + bounds
			|| position.y <= center.y - bounds || position.y >= center.y + bounds
			|| position.z <= center.z - bounds || position.z >= center.z + bounds);
	}

	// Debug functions
	/*int count(){
		int count = 0;
		if(subVoxels){
			for(size_t i = 0; i < 8; i++)
				count += subVoxels[i].count();
			return count + 1;
		}
		return 1;
	}

	string copy(char what, size_t times){
		string out = "";
		for(int i = 0; i < times; i++)
			out += what;
		return out;
	}

	string dump(){
		string out = copy('\t', SUBCHUNK_LEVELS - level) + to_string(level) + " - " + to_string(blockID) + " - " + center.to_string() + "\n";
		if(subVoxels)
			for(size_t i = 0; i < 8; i++)
				out += subVoxels[i].dump();
		return out;
	}*/

protected:
	void iteraterate(int lvl, IterationFunction func_ptr, int& index){
		if(subVoxels && lvl != level)
			for(int i = 0; i < 8; i++)
				subVoxels[i].iteraterate(lvl, func_ptr, index);
		else
			func_ptr(this, index++); 
	}
};

class Chunk: public VoxelInstance {
public:
	bool loaded = false, rendered = false;

	Chunk(vec3 center, bool initalize = false) {
		this->center = center;
		if(initalize){
			int nullData[CHUNK_ARRAY_SIZE];
			for(int i = 0; i < CHUNK_ARRAY_SIZE; i++){
				nullData[i] = 0;
			}
			VoxelInstance::init(nullData);
		}
	}

	Chunk(vec3 center, int blocks[]){
		this->center = center;
		VoxelInstance::init(blocks);
		VoxelInstance::prune();
	}

	void tick(){
		// do something here!
	}
};

#endif // CHUNK_H
