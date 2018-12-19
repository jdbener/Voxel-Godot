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
*/

/*
    TODO: merge into cpp file
*/

#ifndef BLOCKLIST_H
#define BLOCKLIST_H

#include "Defs.hpp"

#include <cereal/types/map.hpp>
#include <map>

// Define bID to be a unsigned sshort int
typedef unsigned short bID;

struct BlockRef {
    // Material IDS
    unsigned short up, down, left, right, front, back;
    // Whether or not the block is solid
    bool solid;

    BlockRef(unsigned short u, unsigned short d, unsigned short l, unsigned short r, unsigned short f, unsigned short b, bool _solid = true)
        : up(u), down(d), left(l), right(r), front(f), back(b), solid(_solid) { }
    BlockRef() : up(0), down(0), left(0), right(0), front(0), back(0), solid(true) { }

    bool operator==(BlockRef other){
        if(up != other.up) return false;
        if(down != other.down) return false;
        if(left != other.left) return false;
        if(right != other.right) return false;
        if(front != other.front) return false;
        if(back != other.back) return false;
        return true;
    }

    bool operator!=(BlockRef other){
        return !(*this == other);
    }

    template <class Archive>
    void serialize( Archive & ar ) {
        ar( CEREAL_NVP(up), CEREAL_NVP(down), CEREAL_NVP(left), CEREAL_NVP(right), CEREAL_NVP(front), CEREAL_NVP(back), CEREAL_NVP(solid) );
    }
};

/*
    TODO: set up automated system: reading JSON file?
*/
struct _BlocksManager {
    std::map<bID, BlockRef> blocks;
    bID blockID = 0;

    _BlocksManager(){
        blocks[blockID++] = BlockRef(0, 0, 0, 0, 0, 0, false); // null
        blocks[blockID++] = BlockRef(1, 2, 3, 4, 5, 6, true); // debug
    }

    BlockRef* operator[](bID ID){
        return &blocks[ID];
    }
};
namespace BlockList {
    static _BlocksManager blocks;
    static BlockRef* null = &blocks.blocks[0];

    static inline BlockRef* getReference(short ID){
        if(blocks.blocks.find(ID) == blocks.blocks.end()) return null;
        return blocks[ID];
    }

    static bID getID(BlockRef* ref){
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

    static inline bID addBlock(BlockRef ref){
        blocks.blocks[blocks.blockID] = ref;
        return blocks.blockID++;
    }
};

#endif
