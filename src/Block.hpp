/*
FILE:               Block.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented BlockRef, Block, and an initial version of
                                        the block system
Joshua Dahl		   2018-12-14		  0.1 - Moved BlockRef and the BlockList to BlockList.hpp
*/

/*
    TODO: merge into cpp file
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "Voxel.hpp"
#include "BlockList.hpp"

#include <cereal/access.hpp>
#include <iostream>

// Prototype Chunk
class Chunk;

struct Block: public Voxel {
    Chunk* chunk; // A reference to the owning chunk
    BlockRef* blockRef; // A reference to the block reference this block represents

    Block(Chunk* _chunk, Vector3 _center, BlockRef* _block = BlockList::null) : Voxel(_center), blockRef(_block) { chunk = _chunk; }
    Block(Chunk* _chunk, BlockRef* _block = BlockList::null) : Voxel(), blockRef(_block) { chunk = _chunk; }

    bool operator==(BlockRef other){
        return *blockRef == other;
    }
    bool operator==(BlockRef* other){
        return blockRef == other;
    }
    bool operator!=(BlockRef other){
        return *blockRef != other;
    }
    bool operator!=(BlockRef* other){
        return blockRef != other;
    }

    template <class Archive>
    void save(Archive & ar) const {
        Voxel::save(ar);
        ar( cereal::make_nvp("BlockID", BlockList::getID(blockRef)) );
        //std::cout << "Block " << BlockList::getID(block) << " found at " << block << std::endl;
    }
    template <class Archive>
    void load(Archive & ar) {
        Voxel::load(ar);
        {
            bID ID;
            ar( cereal::make_nvp("BlockID", ID) );
            blockRef = BlockList::getReference(ID);
        }
    }
};

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Block, cereal::specialization::member_load_save )

#endif
