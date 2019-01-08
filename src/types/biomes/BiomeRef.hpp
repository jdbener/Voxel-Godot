/*
FILE:              BiomeRef.hpp
DESCRIPTION:       Data type representing a base biome

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2019-01-04		  0.0 - Initial implementation
*/

#ifndef BIOME_REF_H
#define BIOME_REF_H

#include "../../GlobalDefs.hpp"

#include <vector>

#include <SimplexNoise/src/SimplexNoise.h>

/*
ENUM:           BiomeType
DESCRIPTION:    Represents what part of the world this biome will contribute to
*/
enum BiomeType {
    surface, underground, sky
};

/*
CLASS:          BiomeRef
DESCRIPTION:    Parent class for the different types of biomes
*/
struct BiomeRef {
    // ID of this biome
    biomeID ID;
    // Type of this biome
    BiomeType type;

    // Variables controlling when this biome should be used
    int minimumElevation = BOTTOM_OF_WORLD; // Might not be used?


    // Instance of SimplexNoise library for the biome
    SimplexNoise* generator;        // Noise generator
    float noiseScale = 200;         // Noise scale

    BiomeRef(biomeID id, SimplexNoise* gen, BiomeType t) : ID(id), generator(gen), type(t) {}
};

/*
CLASS:          BlockChance
DESCRIPTION:    Class holding a block and its weight compared to other blocks in the category
*/
struct BlockChance {
    refID block;    // The ID of the block
    int chance;     // The chance of it occurring (all of the chances are added up, the percentage chance is <chance>/<totalChance>)

    BlockChance(refID b, int c) : block(b), chance(c) {}
};

#endif
