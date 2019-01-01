/*
FILE:               Block.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented BlockRef, Block, and an initial version of
                                        the block system
Joshua Dahl		   2018-12-18		  0.1 - Moved BlockRef and the BlockList to BlockList.hpp
Joshua Dahl		   2018-12-19		  0.2 - Split code off into Block.cpp
Joshua Dahl        2018-12-19         1.0 - Godotized file
Joshua Dahl        2018-12-21         1.1 - Refactored to support the new changes to BlockRefs
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "../BlockList.hpp"
#include "Voxel.hpp"

#include <cereal/access.hpp>
#include <iostream>

using namespace godot;

// Prototype Chunk
class Chunk;

struct Block: public Voxel {
    Chunk* chunk; // A reference to the owning chunk
    BlockRef* blockRef; // A reference to the block reference this block represents

    Block(Chunk* _chunk, Vector3 _center, BlockRef* _block = BlockList::null) : Voxel(_center), blockRef(_block) { chunk = _chunk; }
    Block(Chunk* _chunk, BlockRef* _block = BlockList::null) : Voxel(), blockRef(_block) { chunk = _chunk; }

    bool operator==(BlockRef other);
    bool operator==(BlockRef* other);
    bool operator!=(BlockRef other);
    bool operator!=(BlockRef* other);

    /*
    NAME:           getCenter()
    DESCRIPTION:    Returns a Vector3 containing this block's center
    */
    Vector3 getCenter(){
        return Vector3(x, y, z);
    }

    /*
    NAME:           setBlockRef(BlockRef* ref)
    DESCRIPTION:    Sets the block reference to the provided block reference
    */
    void setBlockRefbyRef(BlockRef* ref){
        blockRef = ref;
    }

    /*
    NAME:           setBlockRef(refID ID)
    DESCRIPTION:    Sets the block reference the one in the blocklist stored at index <ID>
    */
    void setBlockRef(refID ID){
        blockRef = BlockList::getReference(ID);
    }

    /*
    NAME:           save(Archive & ar)
    DESCRIPTION:    Tells the serializer what information to save for this class
    */
    template <class Archive>
    void save(Archive & ar) const {
        // Serialize the parents first
        Voxel::save(ar);
        // Serialize the blockRef into a refID and serialize it
        ar( cereal::make_nvp("BlockID", blockRef->ID) );
    }

    /*
    NAME:           load(Archive & ar)
    DESCRIPTION:    Tells the serializer what information to load into this class
    */
    template <class Archive>
    void load(Archive & ar) {
        // Unserialize the parents first
        Voxel::load(ar);
        {
            /*
                This system allows a chunk file generated on any computer to create
                pointers on any other computer
            */
            // Load the refId and convert it back into a pointer
            refID ID;
            ar( cereal::make_nvp("BlockID", ID) );
            blockRef = BlockList::getReference(ID);
        }
    }
};

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Block, cereal::specialization::member_load_save )

#endif
