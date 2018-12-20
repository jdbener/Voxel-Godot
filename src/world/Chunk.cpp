/*
FILE:               Chunk.cpp
DESCRIPTION:	    Implements a 16 block^3 chunk which contains several layers of
                    sub-chunk. Additionally serialization has been implemented.

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-18         0.0 - Implemented Chunk, Subchunk8/4/2, and serialization
Joshua Dahl        2018-12-18         0.1 - Reworked initialization to propagate position
Joshua Dahl        2018-12-19         1.0 - Godotized file
*/

#include "Chunk.hpp"

#include <fstream>
#include <string>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

/*
NAME:           setSoldity(short solidBlocks)
DESCRIPTION:    Calculates the solidity of the chunk based on the number of solid blocks within the chunk
NOTES:          This is used for chunk occlusion
*/
void Chunk::setSoldity(short solidBlocks){
    solid = solidBlocks > (BLOCKS_PER_CHUNK * SOLIDITY_FRACTION);
}

/*
NAME:           getCenter(bool worldScale)
DESCRIPTION:    Gets the center of the chunk (in either chunk scale or world scale)
*/
Vector3 Chunk::getCenter(bool worldScale){
    if(worldScale)
        return Vector3(x * Chunk::SCALE, y * Chunk::SCALE, z * Chunk::SCALE);
    return Vector3(x, y, z);
}
// Minified versions of the function for the sub-chunks
Vector3 SubChunk8::getCenter(bool worldScale){ if(worldScale) return Vector3(x * SubChunk8::SCALE, y * SubChunk8::SCALE, z * SubChunk8::SCALE); return Vector3(x, y, z); }
Vector3 SubChunk4::getCenter(bool worldScale){ if(worldScale) return Vector3(x * SubChunk4::SCALE, y * SubChunk4::SCALE, z * SubChunk4::SCALE); return Vector3(x, y, z); }
Vector3 SubChunk2::getCenter(bool worldScale){ if(worldScale) return Vector3(x * SubChunk2::SCALE, y * SubChunk2::SCALE, z * SubChunk2::SCALE); return Vector3(x, y, z); }

/*
NAME:           writeChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Serializes the provided <chunk> to the provided <fileName>
*/
void writeChunk(char const* fileName, Chunk & chunk){
    // Open an connection to the file
    std::ofstream os(fileName, std::ios::binary);
    // If we are writing binary chunks, create a binary serializer and serialze <chunk>
    #if BINARY_OUTPUT == true
        /*
            I am using PortableBinary since cereal documentation claims it will be cross platform.
            If this turns out not to be the case the added speed of plain Binary would be perferable
        */
        cereal::PortableBinaryOutputArchive archive( os );
        archive( CEREAL_NVP(chunk) );
    // If we are writing JSON chunks, create a JSON serializer and serialze <chunk>
    #else
        cereal::JSONOutputArchive archive ( os );
        archive( CEREAL_NVP(chunk) );
    #endif
}

/*
NAME:           loadChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Loads the chunked stored at <fileName> into the variable <chunk>
*/
void loadChunk(char const* fileName, Chunk & chunk){
    // Open a connection to the file
    std::ifstream is(fileName, std::ios::binary);
    // Try to load the JSON file
    try {
        cereal::JSONInputArchive archive ( is );
        archive( CEREAL_NVP(chunk) );
    // If it fails due to a JSON error, load the file as a binary chunk instead
    } catch (cereal::RapidJSONException& e){
        cereal::PortableBinaryInputArchive archive( is );
        archive( CEREAL_NVP(chunk) );
    }
}
