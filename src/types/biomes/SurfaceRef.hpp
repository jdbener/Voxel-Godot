/*
FILE:              SurfaceRef.hpp
DESCRIPTION:       Provides a data type representing a surface level biome

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2019-01-04		  0.0 - Initial implementation
Joshua Dahl		   2019-01-08		  0.1 - Swapped priority for another abstract map value called magicalness
*/

#ifndef SURFACE_BIOME_REF_H
#define SURFACE_BIOME_REF_H

#include "BiomeRef.hpp"

#include <initializer_list>

/*
CLASS:          SurfaceVector
DESCRIPTION:    An abstract Vector3 where x represents temperature, y represents
                 humidity, z represents magicalness, and a biomeID is attached
*/
struct SurfaceVector {
    //         x           y          z
    float temperature, humidity, magicalness;
    biomeID id;

    // Constructor
    SurfaceVector(float x, float y, float z, float _id) : temperature(x), humidity(y), magicalness(z), id(_id) {}

    /*
    NAME:       distSquared(float temp, float humid, float magic)
    RETURNS:    The Pythagorean distance squared from the provided point to this point
    */
    float distSquared(float temp, float humid, float magic){
        return (temp - temperature) * (temp - temperature) + (humid - humidity) *
                (humid - humidity) + (magic - magicalness) * (magic - magicalness);
    }
};

struct SurfaceRef: public BiomeRef {
    // Variables controlling when this biome should be used
    float temperature = 60;         // Measured in Fahrenheit
    float humidity = 50;            // Percentage
    float magicalness = 20;         // Latent magic in the area (Percentage)

    // What block is on the surface
    std::vector<BlockChance> surfaceBlocks;         // Block ID
    int minLevel = -10;             // Minimum height of top level of chunk
    int maxLevel = 20;              // Maximum height of top block of chunk

    // What block is below the surface block and how deep does it go
    std::vector<BlockChance> transitionBlocks;      // Block ID
    int transitionMinDepth = 0;   // Depth
    int transitionMaxDepth = 0;   // Random depth

    // Constructor
    SurfaceRef(biomeID id, SimplexNoise* gen) : BiomeRef(id, gen, BiomeType::surface)
        { setSurfaceBlock(0); setTransitionBlock(0); }

    /*
    NAME:           setSurfaceBlocks(std::initializer_list<BlockChance> list)
    DESCRIPTION:    Sets the surface block list to the provided list
    */
    void setSurfaceBlocks(std::initializer_list<BlockChance> list){
        surfaceBlocks = list;
    }

    /*
    NAME:           setSurfaceBlock(refID block)
    DESCRIPTION:    Sets the surface block list to only the provided block
    */
    void setSurfaceBlock(refID block){
        surfaceBlocks = { BlockChance(block, 0) };
    }

    /*
    NAME:           setTransitionBlocks(std::initializer_list<BlockChance> list)
    DESCRIPTION:    Sets the transition block list to the provided list
    */
    void setTransitionBlocks(std::initializer_list<BlockChance> list){
        transitionBlocks = list;
    }

    /*
    NAME:           setTransitionBlock(refID block)
    DESCRIPTION:    Sets the transition block list to only the provided block
    */
    void setTransitionBlock(refID block){
        transitionBlocks = { BlockChance(block, 0) };
    }

    /*
    NAME:       getSurfaceVector()
    RETURNS:    The SurfaceVector for this biome
    */
    SurfaceVector getSurfaceVector(){
        return SurfaceVector(temperature, humidity, magicalness, ID);
    }
};

#endif
