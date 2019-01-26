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

#include <cfloat>

using namespace godot;

struct SurfaceWeight {
    SurfaceRef* biome1;
    float      weight1;
    SurfaceRef* biome2;
    float      weight2;
    SurfaceRef* biome3;
    float      weight3;

    float getSurfaceLevel(float x, float z, int seed){
        return biome1->getSurfaceLevel(x, z, seed) * weight1 +
            biome2->getSurfaceLevel(x, z, seed) * weight2 +
            biome3->getSurfaceLevel(x, z, seed) * weight3;
    }

    float getTransitionLevel(float x, float z, float surfaceLevel, int seed){
        return biome1->getTransitionLevel(x, z, surfaceLevel, seed) * weight1 +
            biome2->getTransitionLevel(x, z, surfaceLevel, seed) * weight2 +
            biome3->getTransitionLevel(x, z, surfaceLevel, seed) * weight3;
    }
};

namespace BiomeList {
    // Array storing the list of biomes
    static std::vector<BiomeRef*> biomes;
    // Array storing the list of SurfaceVectors
    static std::vector<SurfaceVector> surfaceVectors;
    // Variable storing the default noise generator
    static SimplexNoise noiseGen = SimplexNoise(1, 1, 2, .5);

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

        static bool first = true; // Variable storing if this is the first run
        if(first){
            // StoneWorld, default, all stone
            initSurface(stoneWorld, noiseGen);
            stoneWorld->setSurfaceBlock(Blocks::stone);
            BiomeList::addBiome(stoneWorld); // 0, all stone,

            // Debug
            initSurface(debug, noiseGen);
            debug->setSurfaceBlock(1);
            debug->minLevel = 0;
            debug->maxLevel = 50;

            debug->temperature = 0;
            debug->humidity = 0;
            debug->magicalness = 100;

            BiomeList::addBiome(debug); // 1, debug

            // Plains
            initSurface(plains, noiseGen);
            plains->setSurfaceBlock(Blocks::grass);
            plains->setTransitionBlock(Blocks::dirt);
            plains->transitionMinDepth = 4;
            plains->transitionMaxDepth = 7;
            plains->minLevel = 4;
            plains->maxLevel = 10;

            plains->temperature = 50;
            plains->humidity = 60;

            BiomeList::addBiome(plains); // 1, plains

            // Forest
            initSurface(forest, noiseGen);
            forest->setSurfaceBlock(Blocks::grass);
            forest->setTransitionBlock(Blocks::dirt);
            forest->transitionMinDepth = 5;
            forest->transitionMaxDepth = 10;
            forest->minLevel = 0;
            forest->maxLevel = 10;

            forest->temperature = 40;
            forest->humidity = 60;

            BiomeList::addBiome(forest); // 2, forest



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
        /*
        TODO: make this return the 3? closest biomes and weight them (BiomeChance class?)
                add a biome to a vector, sort and drop any elements after the third?
        */
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

    /*
    NAME:           getSurfaceBiome(float temperature, float humidity, float magicalness)
    DESCRIPTION:    Gets the SurfaceRef from the list closest to the provided parameters
    */
    static SurfaceWeight getSurfaceBiomes(float temperature, float humidity, float magicalness){
        struct BiomeWeightTemp {
            biomeID id;
            float dist;

            BiomeWeightTemp(biomeID i, float d) : id(i), dist(d) { }

            bool operator<= (BiomeWeightTemp other){
                return dist <= other.dist;
            }
        };
        /*
        TODO: make this return the 3? closest biomes and weight them (BiomeChance class?)
                add a biome to a vector, sort and drop any elements after the third?
        */
        std::vector<BiomeWeightTemp> temp;

        // For every SurfaceVector in the list...
        for(SurfaceVector cur: surfaceVectors){
            // Calculate the distance from that point in "abstract 3D space" to the provided point
            float distSquared = cur.distSquared(temperature, humidity, magicalness);
            //
            temp.push_back(BiomeWeightTemp(cur.id, distSquared));
        }

        quickSort(temp);

        float total = temp[0].dist + temp[1].dist + temp[2].dist;
        SurfaceWeight out;
        /*
            The weights are one-minused since we want biome1 to have the most weight
        */
        // Add references and weights (inversed) to struct
        out.biome1 = getSurfaceBiome(temp[0].id); out.weight1 = temp[0].dist / total;
        out.biome2 = getSurfaceBiome(temp[1].id); out.weight2 = temp[1].dist / total;
        out.biome3 = getSurfaceBiome(temp[2].id); out.weight3 = temp[2].dist / total;
        return out;
    }
}

#endif
