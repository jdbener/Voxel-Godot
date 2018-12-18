/*
FILE:               Block.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented BlockRef, Block, and an initial version of
                                        the block system
*/

/*
    TODO: merge into cpp file
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "Voxel.hpp"
#include <cereal/access.hpp>

// Prototype Chunk
class Chunk;

struct BlockRef {
    // Material IDS
    unsigned short up, down, left, right, front, back;

    BlockRef(unsigned short u, unsigned short d, unsigned short l, unsigned short r, unsigned short f, unsigned short b) : up(u), down(d), left(l), right(r), front(f), back(b) { }

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
};

/*
    TODO: set up automated system: reading JSON file?
*/
namespace Blocks {
    // 0 - empty/air block
    static BlockRef null = BlockRef(0, 0, 0, 0, 0, 0);

    static BlockRef* getReference(short ID){
        switch(ID){
        case 0: return &null; break;


        }
        return &null;
    }

    static short getID(BlockRef* ref){
        if(ref == &null)
            return 0;


        else
            return 0;
    }
};

struct Block: public Voxel {
    Chunk* chunk;
    BlockRef* block;

    Block(Chunk* _chunk, Vector3 _center, BlockRef* _block = &Blocks::null) : Voxel(_center), block(_block) { chunk = _chunk; }
    Block(Chunk* _chunk, BlockRef* _block = &Blocks::null) : Voxel(), block(_block) { chunk = _chunk; }

    bool operator==(BlockRef other){
        return *block == other;
    }
    bool operator!=(BlockRef other){
        return *block != other;
    }

    template <class Archive>
    void save(Archive & ar) const {
        Voxel::save(ar);
        ar( cereal::make_nvp("BlockID", Blocks::getID(block)) );
    }
    template <class Archive>
    void load(Archive & ar) {
        Voxel::load(ar);
        {
            short ID;
            ar( cereal::make_nvp("BlockID", ID) );
            block = Blocks::getReference(ID);
        }
    }
};

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Block, cereal::specialization::member_load_save )

#endif
