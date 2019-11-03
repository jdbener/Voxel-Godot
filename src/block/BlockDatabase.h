#ifndef __BLOCK_MANAGER_H__
#define __BLOCK_MANAGER_H__
#include <Material.hpp>
#include <vector>

#include "../godot/CerealGodot.h"
#include "BlockFeatureDatabase.h"

typedef size_t Identifier;
typedef unsigned short flag_t;

using namespace godot;

class BlockData {
public:
	// Variable storing the identification of the block for storage purposes
	Identifier blockID = 0;

	enum Flags {
			null = 0,
			TRANSPARENT = 1,
			DONT_RENDER = 2,
			INVISIBLE = 3 // 2 and 1
	};
	// Variable storing the flags for this block
	flag_t flags = 0;
	// Variable storing the loaded features of this block
	std::map<godot::String, Feature*> features;
//	godot::Material mat;

	// Constructer
    BlockData(flag_t f = Flags::null, const std::initializer_list<godot::String> features = {}) : flags(f) {
		this->features = BlockFeatureDatabase::getSingleton()->getFeatures(features);
	}

    // Function which compares the provided mask to the bitfield
	bool checkFlag(Flags mask) const { return (flags & mask) == mask; }
	// Function which checks if a block data instance has features which can't be pruned
	bool hasUnprunableFeature() {
		for(auto feature: features)
			if (!feature.second->pruneable)
				return true;
		return false;
	}

	// Functions which allow blocks to preform some action when their chunk is saved/loaded
	template <class Archive>
	void save(Archive& archive) const {
		archive( CEREAL_NVP(flags) ); // Debug
		for(auto feature: features)
			archive(cereal::make_nvp(feature.first.utf8().get_data(), *feature.second));
	}
	template <class Archive>
	void load(Archive& archive){
		for(auto feature: features)
			archive(cereal::make_nvp(feature.first.utf8().get_data(), *feature.second));
	}
};

class BlockDatabase {
public:
	// Array storing the nessicary blocks
	std::vector<BlockData*> blocks;
	// Function which gets a reference to the singleton for the database
	static BlockDatabase* getSingleton();

	// Function which adds the built in blocks
    BlockDatabase();
	// Function which cleans up after the BlockManager
	~BlockDatabase();
    // Function which adds a block to the database
    Identifier addBlock(BlockData* d);
    // Function which gets a copy of one of the blocks in the database
    BlockData* getBlock(Identifier id, bool loadFeatures = true);
};

// List of blocks... may remove it this proves unworthy of maintience
namespace Blocks {
	static Identifier AIR;
} // Blocks

#endif // __BLOCK_MANAGER_H__
