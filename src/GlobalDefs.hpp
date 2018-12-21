/*
FILE:               GlobalDefs.hpp
DESCRIPTION:        A list of global constants which the whole program can use

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Implemented constants used by the chunk system
Joshua Dahl		   2018-12-14		  1.1 - Renamed file to fix conflict with Godot
*/

#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

// Define refID to be a unsigned short int
typedef unsigned short refID;

// Whether or not we export JSON debug chunks
#define JSON_DEBUG true

// The fraction of blocks in a sub/chunk which must be solid for the sub/chunk to be considered solid
const float SOLIDITY_FRACTION = 1.0 / 2;
// Constant storing how many sub-chunks each chunk/sub-chunk has
const short SUB_PER_WHOLE = 8;
// Constant storing how many blocks total are in a chunk
const short BLOCKS_PER_CHUNK = 16 * 16 * 16;

#endif
