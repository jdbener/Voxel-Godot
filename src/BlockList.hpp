/*
FILE:               BlockList.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented JSON based block definition system
Joshua Dahl		   2018-12-14		  0.1 - Reimplemented block definition system to work entirely in memory
                                        (I realized there was no benefit to the JSON system and it would
                                        just create another file I would have to ensure is in the right place)
Joshua Dahl		   2018-12-19		  0.2 - Split code into BlockList.cpp
Joshua Dahl        2018-12-19         1.0 - Godotized file
Joshua Dahl        2018-12-21         1.1 - Simplified BlockList system (ID is now stored in the blockRef) and added visibility to BlockRef
*/

#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include "./GlobalDefs.hpp"

#include <cereal/types/map.hpp>
#include <map>

struct BlockRef {
    // The refID this block represents
    refID ID;
    // Material IDS
    unsigned short up, down, left, right, front, back;
    bool solid, // Whether or not the block is collideable
        visible; // Whether or not the block is renderable


    BlockRef(refID _ID = 0, unsigned short u = 0, unsigned short d = 0, unsigned short l = 0, unsigned short r = 0,
        unsigned short f = 0, unsigned short b = 0, bool _solid = true, bool _visible = true)
        : ID(_ID), up(u), down(d), left(l), right(r), front(f), back(b), solid(_solid), visible(_visible) { }

    bool operator==(BlockRef other);
    bool operator!=(BlockRef other);
};

struct _BlocksManager {
    // Actual Block list
    std::map<refID, BlockRef> blocks;
    // ID used for adding blocks
    refID blockID = 0;

    /* ----------------------------------------------------------------------------
        This is where the BlockList is defined
    ---------------------------------------------------------------------------- */
    _BlocksManager(){
        blocks[blockID] = BlockRef(blockID, 0, 0, 0, 0, 0, 0, false, false); blockID++; // null
        blocks[blockID] = BlockRef(blockID, 1, 2, 3, 4, 5, 6, true, true); blockID++; // debug
    }

    BlockRef* operator[](refID ID){
        return &blocks[ID];
    }
};

namespace BlockList {
    // A reference to the block list manager
    static _BlocksManager blocks;
    // Name block[0] null
    static BlockRef* null = blocks[0];

    /*
    NAME:           getReference(refID ID)
    DESCRIPTION:    Converts the provided refID into a pointer to the BlockRef
    NOTES:          If the block ID can't be found returns a pointer to the null ref
    */
    static BlockRef* getReference(refID ID){
        if(blocks.blocks.find(ID) == blocks.blocks.end()) return null;
        return blocks[ID];
    }

    /*
    NAME:           addBlock(BlockRef ref)
    DESCRIPTION:    Function to for an external source to add a block to the manager
    */
    static refID addBlock(BlockRef ref){
        blocks.blocks[blocks.blockID] = ref;
        return blocks.blockID++;
    }
};

#endif
