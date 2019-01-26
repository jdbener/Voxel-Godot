/*
FILE:              ChunkGenerator.hpp
DESCRIPTION:       Procedures for generating new chunks

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
Joshua Dahl		   2010-01-08		  0.1 - Split code into cpp file
*/

#ifndef CHUNKGENERATOR_H
#define CHUNKGENERATOR_H

#include "world/Chunk.hpp"

#include "helpers/Godotize.hpp"

Chunk* generateChunk(Vector3 center, bool forceRegenerate = false, bool worldSpace = true);

#endif
