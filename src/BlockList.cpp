/*
FILE:               BlockList.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-19		  0.2 - Split code into BlockList.cpp
Joshua Dahl        2018-12-19         1.0 - Godotized file
Joshua Dahl        2018-12-21         1.1 - Simplified BlockList system and added visibility to BlockRef
*/

#include "BlockList.hpp"

/* ----------------------------------------------------------------------------
    Block Ref
---------------------------------------------------------------------------- */
/*
NAME:           ==(BlockRef other)
DESCRIPTION:    Checks if the provided BlockRef is the same as this one
*/
bool BlockRef::operator==(BlockRef other){
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
bool BlockRef::operator!=(BlockRef other){
    return !(*this == other);
}
