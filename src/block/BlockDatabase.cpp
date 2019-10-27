#include "BlockDatabase.h"

BlockDatabase* BlockDatabase::getSingleton(){
    static BlockDatabase db;
    return &db;
}

// Function which adds the built in blocks
BlockDatabase::BlockDatabase(){
    // TODO loading blocks from file
    BlockData* air = new BlockData(BlockData::INVISIBLE);
    addBlock(air);

    BlockData* debug = new BlockData();
    addBlock(debug);
}

// Function which adds a block to the database
void BlockDatabase::addBlock(BlockData* d){
    blocks.push_back(d);
    blocks[blocks.size() - 1]->blockID = blocks.size() - 1;
}

// Function which gets a copy of one of the blocks in the database
BlockData* BlockDatabase::getBlock(Identifier id){
    // If the id is outside of the array... return a nullptr
    if(id > blocks.size()) return nullptr;

    // TODO: support derived classes?
    BlockData* out = new BlockData;
    *out = *blocks[id];
    return out;
}

// Function which cleans up after the BlockManager
BlockDatabase::~BlockDatabase(){
    for(BlockData* p: blocks)
        delete p;
}
