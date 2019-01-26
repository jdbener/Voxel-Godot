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
// Define matID to be a unsigned short int
typedef unsigned short matID;
// Define matID to be a unsigned short int
typedef unsigned short biomeID;

// Whether or not we export JSON debug chunks
#define JSON_DEBUG true

// The fraction of blocks in a sub/chunk which must be solid for the sub/chunk to be considered solid
const float SOLIDITY_FRACTION = 1.0 / 2;
// Constant storing how many sub-chunks each chunk/sub-chunk has
const short SUB_PER_WHOLE = 8;
// Constant storing how many blocks total are in a chunk
const short BLOCKS_PER_CHUNK = 16 * 16 * 16;

// Constant storing the lowest block of the world (based on accuracy of a float)
const int BOTTOM_OF_WORLD = -9999999;
// Constant storing the base see level of the world
const int SEE_LEVEL = 0;

// Constants storing the octaves/offset/scale for transition block noise
const int TRANSITION_OCTAVES = 5;
const int TRANSITION_OFFSET = 31241;
const float TRANSITION_SCALE = 20;
// Constants storing the octaves/offset/scale for temperature noise
const int TEMPERATURE_OCTAVES = 5;
const int TEMPERATURE_OFFSET = 15616;
const float TEMPERATURE_SCALE = 200;
// Constants storing the octaves/offset/scale for humidity noise
const int HUMIDITY_OCTAVES = 5;
const int HUMIDITY_OFFSET = 41896;
const float HUMIDITY_SCALE = 400;
// Constants storing the octaves/offset/scale for magicalness noise
const int MAGICALNESS_OCTAVES = 2;
const int MAGICALNESS_OFFSET = 96531;
const float MAGICALNESS_SCALE = 200;

#endif
