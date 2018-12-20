/*
FILE:               Block.hpp
DESCRIPTION:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-19		  0.2 - Split code off into Block.cpp
Joshua Dahl        2018-12-19         1.0 - Godotized file
*/
#include "Block.hpp"

/*
    The comparison operators compare a block to a block ref, this is because a
    block is just a manager for a blockRef
*/
// Comparison Operators
bool Block::operator==(BlockRef other){
    return *blockRef == other;
}
bool Block::operator==(BlockRef* other){
    return blockRef == other;
}
bool Block::operator!=(BlockRef other){
    return *blockRef != other;
}
bool Block::operator!=(BlockRef* other){
    return blockRef != other;
}
