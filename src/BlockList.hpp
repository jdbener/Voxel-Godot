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
Joshua Dahl        2018-12-24         1.2 - Replaced visibility with opacity
Joshua Dahl        2018-12-24         1.2 - Replaced the map with a vector
*/

#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include "./GlobalDefs.hpp"

#include <cereal/types/vector.hpp>
#include <vector>

struct BlockRef {
    // The refID this block represents
    refID ID;
    // Material IDS
    unsigned short up, down, left, right, front, back;
    bool solid, // Whether or not the block is collideable
        /*
            Replaced visibility with opacity, since any face with matID 0 will never be rendered
        */
        opaque; // Whether or not the block is opaque (can its edges be combined with other edges?)


    BlockRef(refID _ID = 0, unsigned short u = 0, unsigned short d = 0, unsigned short l = 0, unsigned short r = 0,
        unsigned short f = 0, unsigned short b = 0, bool _solid = true, bool _opaque = true)
        : ID(_ID), up(u), down(d), left(l), right(r), front(f), back(b), solid(_solid), opaque(_opaque) { }

    bool operator==(BlockRef other);
    bool operator!=(BlockRef other);
};

struct _BlocksManager {
    // Actual Block list
    std::vector<BlockRef> blocks;
    // ID used for adding blocks
    refID blockID = 0;

    /* ----------------------------------------------------------------------------
        This is where the BlockList is defined
    ---------------------------------------------------------------------------- */
    _BlocksManager(){
                                /*   ID   up do le ri fr ba  solid  opaque */
        blocks.push_back(BlockRef(blockID, 0, 0, 0, 0, 0, 0, false, false)); blockID++; // 0, null
        blocks.push_back(BlockRef(blockID, 1, 2, 1, 1, 1, 1, true,  false)); blockID++; // 1, debug
        blocks.push_back(BlockRef(blockID, 2, 2, 2, 2, 2, 2, true,  true));  blockID++; // 2, stone
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
        if(blocks.blocks.size() < ID) return null;
        return blocks[ID];
    }

    /*
    NAME:           addBlock(BlockRef ref)
    DESCRIPTION:    Function to for an external source to add a block to the manager
    */
    static refID addBlock(BlockRef ref){
        blocks.blocks.push_back(ref);
        return blocks.blockID++;
    }
};

#endif
