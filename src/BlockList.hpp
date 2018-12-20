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
*/

#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include "./GlobalDefs.hpp"

#include <cereal/types/map.hpp>
#include <map>

struct BlockRef {
    // Material IDS
    unsigned short up, down, left, right, front, back;
    // Whether or not the block is solid
    bool solid;

    BlockRef(unsigned short u, unsigned short d, unsigned short l, unsigned short r, unsigned short f, unsigned short b, bool _solid = true)
        : up(u), down(d), left(l), right(r), front(f), back(b), solid(_solid) { }
    BlockRef() : up(0), down(0), left(0), right(0), front(0), back(0), solid(true) { }

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
        blocks[blockID++] = BlockRef(0, 0, 0, 0, 0, 0, false); // null
        blocks[blockID++] = BlockRef(1, 2, 3, 4, 5, 6, true); // debug
    }

    BlockRef* operator[](refID ID){
        return &blocks[ID];
    }
};

namespace BlockList {
    // A reference to the block list manager
    static _BlocksManager blocks;
    // Name block[0] null
    static BlockRef* null = &blocks.blocks[0];

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
    NAME:           getID(BlockRef* ref)
    DESCRIPTION:    Converts a pointer to a blockRef into a refID
    */
    static refID getID(BlockRef* ref){
        // For every block in the blocklist
        for (auto const& cur : blocks.blocks)
            // If the memory location of the value stored in the map is the same
            // as the requested memory location
            if(&cur.second == ref)
                // Return the key (ID) stored in the map
                return cur.first;
        // If we didn't find it, return the ID of null
        return 0;
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
