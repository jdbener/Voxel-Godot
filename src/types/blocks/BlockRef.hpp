/*
FILE:               BlockRef.hpp
DESCRIPTION:        Base type representing a type of block

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2019-01-04		  0.0 - Split BlockRef into it's own file
Joshua Dahl		   2019-01-08		  0.1 - Implemented BlockTypes
*/
#ifndef BLOCK_REF_H
#define BLOCK_REF_H

#include "../../GlobalDefs.hpp"

enum BlockType {
    cube, liquid, model
};

struct BlockRef {
    // The refID this block represents
    refID ID;
    // Variable storing what kind of block rendering to use for this block
    BlockType type = BlockType::cube;
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

    /*
    NAME:           ==(BlockRef other)
    DESCRIPTION:    Checks if the provided BlockRef is the same as this one
    */
    bool operator==(BlockRef other){
        if(up != other.up) return false;
        if(down != other.down) return false;
        if(left != other.left) return false;
        if(right != other.right) return false;
        if(front != other.front) return false;
        if(back != other.back) return false;
        return true;
    }

    /*
    NAME:           !=(BlockRef other)
    DESCRIPTION:    Checks if the provided BlockRef is not the same as this one
    */
    bool operator!=(BlockRef other){
        return !(*this == other);
    }
};

#endif
