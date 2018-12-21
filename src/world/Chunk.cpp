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
Joshua Dahl        2018-12-21         1.1 - Implemented functions/macros to run code for every block,
                                        and changed serializer to only read load binary (JSON still
                                        available for debugging)
*/

#include "Chunk.hpp"
#include "../Vector3Extra.hpp"

#include <fstream>
#include <string>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <iostream>

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
NAME:           getBlockIndex(Vector3 search, Vector3 center, short scale)
DESCRIPTION:    Gets the index of a block, this is a helper function for getBlock
                which is used to calculate which index of the subChunk array to access
*/
int getBlockIndex(Vector3 search, Vector3 center, short scale){
    // Variable storing the search location normalized to (-15, -15, -15) <= searchCenter <= (15, 15, 15)
    Vector3 searchCenter = search - center;
    int index = -1; // Variable storing the output index (-1 if not found)
    // If the index is really in the normalized range
    if(searchCenter > expand(-scale) && searchCenter < expand(scale)){
        // Pick the index based on x, y, z values
        if(searchCenter.x > 0){ // 0, 3, 4, 7
            if(searchCenter.y > 0) { // 0, 3
                if(searchCenter.z > 0){ // 0
                    index = 0;
                } else { // 3, int&
                    index = 3;
                }
            } else { // 4, 7
                if(searchCenter.z > 0){ // 4
                    index = 4;
                } else { // 7
                    index = 7;
                }
            }
        } else { // 1, 2, 5, 6
            if(searchCenter.y > 0) { // 1, 2
                if(searchCenter.z > 0){ // 1
                    index = 1;
                } else { // 2
                    index = 2;
                }
            } else { // 5, 6
                if(searchCenter.z > 0){ // 5
                    index = 5;
                } else { // 6
                    index = 6;
                }
            }
        }
    }
    // Return the calculated index
    return index;
}

/*
NAME:           getBlock(Vector3 search)
DESCRIPTION:    Gets a block out of the chunk
*/
Block* Chunk::getBlock(Vector3 search){
    // Get the index for every element in the array
    int c8 = getBlockIndex(search, getCenter(), Chunk::SCALE),
        c4 = getBlockIndex(search, subChunks[c8].getCenter(), SubChunk8::SCALE),
        c2 = getBlockIndex(search, subChunks[c8].subChunks[c4].getCenter(), SubChunk4::SCALE),
        block = getBlockIndex(search, subChunks[c8].subChunks[c4].subChunks[c2].getCenter(), SubChunk2::SCALE);
    // Ensure the indexes are all correct
    if(c8 != -1 && c4 != -1 && c2 != -1 && block != -1)
        // Gets the memmory address of the requested block
        return &subChunks[c8].subChunks[c4].subChunks[c2].blocks[block];
    // If one of the indexes was wrong return nullptr
    return nullptr;
}

/*
NAME:           runOnBlocks(void (*func)(Block&, int&, int&))
DESCRIPTION:    Runs the provided function once for every block in the chunk,
                Additionally provides a result (which is returned) and the current index
                (calculated linearly) to the function
*/
int Chunk::runOnBlocks(void (*func)(Block& curBlock, int& result, int index)){
    int result = -1, i = 0;
    RUN_ON_BLOCKS(this->) func(block, result, i++);
    return result;
}

/*
NAME:           runOnBlocks(void (*func)(Block& curBlock, int& result))
DESCRIPTION:    Runs the provided function once for every block in the chunk,
                Additionally provides a result (which is return)
*/
int Chunk::runOnBlocks(void (*func)(Block& curBlock, int& result)){
    int result = -1;
    RUN_ON_BLOCKS(this->) func(block, result);
    return result;
}

/*
NAME:           runOnBlocks(void (*func)(Block& curBlock))
DESCRIPTION:    Runs the provided function once for every block in the chunk
*/
void Chunk::runOnBlocks(void (*func)(Block& curBlock)){
    RUN_ON_BLOCKS(this->) func(block);
}

/*
NAME:           saveChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Serializes the provided <chunk> to the provided <fileName>
*/
void saveChunk(char const* fileName, Chunk & chunk){
    // Open an connection to the file
    std::ofstream os(fileName, std::ios::binary);
    /*
        I am using PortableBinary since cereal documentation claims it will be cross platform.
        If this turns out not to be the case the added speed of plain Binary would be preferable
    */
    cereal::PortableBinaryOutputArchive archive( os );
    archive( CEREAL_NVP(chunk) );

    // If debug JSON is enabled
    #if JSON_DEBUG == true
        // Append .debug to the end of the file name
        std::string fileNameDebug = fileName; fileNameDebug += ".debug";
        // Open a connection to the new file
        std::ofstream jos(fileNameDebug.c_str(), std::ios::binary);
        // Archive it
        cereal::JSONOutputArchive jarchive ( jos );
        jarchive( CEREAL_NVP(chunk) );
        jos.close();
    #endif

    os.close();
}

/*
NAME:           loadChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Loads the chunked stored at <fileName> into the variable <chunk>
*/
void loadChunk(char const* fileName, Chunk & chunk){
    // Open a connection to the file
    std::ifstream is(fileName, std::ios::binary);

    cereal::PortableBinaryInputArchive archive( is );
    archive( CEREAL_NVP(chunk) );


    is.close();
}
