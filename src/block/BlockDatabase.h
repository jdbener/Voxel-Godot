#ifndef __BLOCK_MANAGER_H__
#define __BLOCK_MANAGER_H__
#include <Material.hpp>
#include <vector>

#include "../godot/CerealGodot.h"

typedef size_t Identifier;
typedef unsigned short flag_t;

using namespace godot;

class BlockData{//: public Resource {
	// TODO make BlockData an actual resource which can be loaded off the disk
public:
	enum Flags{
			TRANSPARENT = 1,
			DONT_RENDER = 2,
			INVISIBLE = 3 // 2 and 1
	};
	flag_t flags = 0;
	Identifier blockID = 0;
//	godot::Material mat;

    BlockData(flag_t f = 0) : flags(f) {}
    virtual ~BlockData() {}

    // Function which compares the provided mask to the bitfield
	virtual bool checkFlag(Flags mask){
		return (flags & mask) == mask;
	}

	// Functions which allow blocks to preform some action when their chunk is saved/loaded
	virtual void save(){ /* Stub */ }
	virtual void load(){ /* Stub */ }
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
    void addBlock(BlockData* d);
    // Function which gets a copy of one of the blocks in the database
    BlockData* getBlock(Identifier id);
};



#endif // __BLOCK_MANAGER_H__
