/*
FILE:              BiomeList.hpp
DESCRIPTION:       Functions for storing a list of biomes

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2019-01-04		  0.0 - Initial implementation
*/

#ifndef BIOME_LIST_H
#define BIOME_LIST_H

#include "../helpers/QuickSort.hpp"

#include "biomes/SurfaceRef.hpp"
#include "BlockList.hpp"

#include <map>
#include <Vector2.hpp>

#include <cfloat>

using namespace godot;

namespace BiomeList {
    // Array storing the list of biomes
    static std::vector<BiomeRef*> biomes;
    // Array storing the list of SurfaceVectors
    static std::vector<SurfaceVector> surfaceVectors;

    /*
    NAME:           addBiome(BiomeRef* ref)
    DESCRIPTION:    Function for adding blocks to the biomelist
    */
    static void addBiome(BiomeRef* ref){
        // Add it to the array
        biomes.push_back(ref);
        // If this is a surface biome add its SurfaceVector to the array
        if(ref->type == BiomeType::surface)
            surfaceVectors.push_back(((SurfaceRef*)ref)->getSurfaceVector());
    }

    /*
    MACRO:          initSurface(name, gen)
    DESCRIPTION:    Creates a new SurfaceRef pointer named <name> with noise generator <gen>
    */
    #define initSurface(name, gen) SurfaceRef* name = new SurfaceRef(bID, &gen); bID++

    /* ----------------------------------------------------------------------------
        This is where the MaterialList is defined
    ---------------------------------------------------------------------------- */
    static void initBiomeList(){
        static biomeID bID = 0; // Variable storing the index of the next biome
        // Variable storing the default noise generator
        static SimplexNoise noise = SimplexNoise(1, 1, 2, .5);

        static bool first = true; // Variable storing if this is the first run
        if(first){
            // StoneWorld, default, all stone
            initSurface(stoneWorld, noise);
            stoneWorld->setSurfaceBlock(Blocks::stone);
            BiomeList::addBiome(stoneWorld); // 0, all stone, lowest priority

            // Plains
            initSurface(plains, noise);
            plains->setSurfaceBlock(Blocks::grass);
            plains->setTransitionBlock(Blocks::dirt);
            plains->transitionMinDepth = 4;
            plains->transitionMaxDepth = 7;

            plains->temperature = 50;
            plains->humidity = 60;

            BiomeList::addBiome(plains); // 1, plains

            first = false;
        }
    }

    /*
    NAME:           getSurfaceBiome(biomeID ID)
    DESCRIPTION:    Gets the SurfaceRef out of the list
    */
    static SurfaceRef* getSurfaceBiome(biomeID ID){
        if(biomes.size() < ID) return nullptr;
        if(biomes[ID]->type != BiomeType::surface) return nullptr;
        return ((SurfaceRef*) biomes[ID]);
    }

    /*
    NAME:           getSurfaceBiome(float temperature, float humidity, float magicalness)
    DESCRIPTION:    Gets the SurfaceRef from the list closest to the provided parameters
    */
    static SurfaceRef* getSurfaceBiome(float temperature, float humidity, float magicalness){
        // The SurfaceVector associated with the shortest distance
        SurfaceVector least = SurfaceVector(0, 0, 0, 0);
        // The shortest distance (starts at the max value storeable in a float)
        float leastDistance = FLT_MAX;

        // For every SurfaceVector in the list...
        for(SurfaceVector cur: surfaceVectors){
            // Calculate the distance from that point in "abstract 3D space" to the provided point
            float distSquared = cur.distSquared(temperature, humidity, magicalness);
            // If that distance is smaller than the current smallest distance...
            if(distSquared < leastDistance){
                // Update the smallest distance and associated SurfaceVector
                leastDistance = distSquared;
                least = cur;
            }
        }

        // Returns the biome associated with the biomeID associated with the smallest distance
        return getSurfaceBiome(least.id);
    }
}

#endif
