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
Joshua Dahl        2018-12-24         1.2 - Implemented propagation of block state through the rest of the chunk
Joshua Dahl        2018-12-25         1.3 - Added code to get the levels of subChunks
Joshua Dahl		   2019-01-04		  1.4 - Added anyTransparent
*/

#include "Chunk.hpp"
#include "../helpers/Godotize.hpp"

#include <String.hpp>

#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <exception>

#include <cereal/archives/json.hpp>

#include <iostream>

/*
NAME:           setOpacity(short opaqueBlocks)
DESCRIPTION:    Calculates the solidity of the chunk based on the number of solid blocks within the chunk
NOTES:          This is used for chunk occlusion
*/
void Chunk::setOpacity(short opaqueBlocks){
    opaque = opaqueBlocks > (BLOCKS_PER_CHUNK * SOLIDITY_FRACTION);
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
int getBlockIndex(Vector3 search, Vector3 center, short scale, bool fuzzy){
    // Variable storing the search location normalized to (-15, -15, -15) <= searchCenter <= (15, 15, 15)
    Vector3 searchCenter = search - center;
    // If the index is really in the normalized range (or we are doing fuzzy matching)
    if((searchCenter > expand(-scale) && searchCenter < expand(scale)) || fuzzy) {
        // Pick the index based on x, y, z values
        if(searchCenter.x > 0){ // 0, 3, 4, 7
            if(searchCenter.y > 0) { // 0, 3
                if(searchCenter.z > 0){ // 0
                    return 0;
                } else { // 3
                    return 3;
                }
            } else { // 4, 7
                if(searchCenter.z > 0){ // 4
                    return 4;
                } else { // 7
                    return 7;
                }
            }
        } else { // 1, 2, 5, 6
            if(searchCenter.y > 0) { // 1, 2
                if(searchCenter.z > 0){ // 1
                    return 1;
                } else { // 2
                    return 2;
                }
            } else { // 5, 6
                if(searchCenter.z > 0){ // 5
                    return 5;
                } else { // 6
                    return 6;
                }
            }
        }
    }
    // Return the calculated index
    return -1;
}

/*
NAME:           getSubChunk8(Vector3 search)
DESCRIPTION:    Gets a SubChunk8 out of the chunk
*/
SubChunk8* Chunk::getSubChunk8(Vector3 search, bool fuzzy){
    // Get the index for every element in the array
    int c8 = getBlockIndex(search, getCenter(), Chunk::SCALE, fuzzy);
    if(c8 < 0 || c8 > 7) return nullptr;
    // Ensure the indexes are all correct
    return &subChunks[c8];
}

/*
NAME:           getSubChunk4(Vector3 search)
DESCRIPTION:    Gets a SubChunk4 out of the chunk
*/
SubChunk4* Chunk::getSubChunk4(Vector3 search, bool fuzzy){
    if( fuzzyEquals(search, Vector3(-76, -52, -76)) )
        return nullptr;
    // Get the index for every element in the array
    int c8 = getBlockIndex(search, getCenter(), Chunk::SCALE, fuzzy);
    if(c8 < 0 || c8 > 7) return nullptr;
    int c4 = getBlockIndex(search, subChunks[c8].getCenter(), SubChunk8::SCALE, fuzzy);
    if(c4 < 0 || c4 > 7) return nullptr;
    // Ensure the indexes are all correct
    return &subChunks[c8].subChunks[c4];
}

/*
NAME:           getSubChunk2(Vector3 search)
DESCRIPTION:    Gets a SubChunk2 out of the chunk
*/
SubChunk2* Chunk::getSubChunk2(Vector3 search, bool fuzzy){
    // Get the index for every element in the array
    int c8 = getBlockIndex(search, getCenter(), Chunk::SCALE, fuzzy);
    if(c8 < 0 || c8 > 7) return nullptr;
    int c4 = getBlockIndex(search, subChunks[c8].getCenter(), SubChunk8::SCALE, fuzzy);
    if(c4 < 0 || c4 > 7) return nullptr;
    int c2 = getBlockIndex(search, subChunks[c8].subChunks[c4].getCenter(), SubChunk4::SCALE, fuzzy);
    if(c2 < 0 || c2 > 7) return nullptr;
    // Ensure the indexes are all correct
    return &subChunks[c8].subChunks[c4].subChunks[c2];
}

/*
NAME:           getBlock(Vector3 search)
DESCRIPTION:    Gets a block out of the chunk
*/
Block* Chunk::getBlock(Vector3 search, bool fuzzy){
    // Get the index for every element in the array
    int c8 = getBlockIndex(search, getCenter(), Chunk::SCALE, fuzzy);
    if(c8 < 0 || c8 > 7) return nullptr;
    //Godot::print("c8");
    int c4 = getBlockIndex(search, subChunks[c8].getCenter(), SubChunk8::SCALE, fuzzy);
    if(c4 < 0 || c4 > 7) return nullptr;
    //Godot::print("c4");
    int c2 = getBlockIndex(search, subChunks[c8].subChunks[c4].getCenter(), SubChunk4::SCALE, fuzzy);
    if(c2 < 0 || c2 > 7) return nullptr;
    //Godot::print("c2");
    int block = getBlockIndex(search, subChunks[c8].subChunks[c4].subChunks[c2].getCenter(), SubChunk2::SCALE, fuzzy);
    if(block < 0 || block > 7) return nullptr;
    //Godot::print("block");
    // Ensure the indexes are all correct
    return &subChunks[c8].subChunks[c4].subChunks[c2].blocks[block];
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
NAME:           mode(matID array[])
DESCRIPTION:    Returns the matID which appears most often in the array
CREDITS:        ~Hesham Attia (https://stackoverflow.com/questions/42194494/find-the-mode-of-an-unsorted-array-and-if-that-array-has-more-than-one-mode-or-n)
*/
matID mode(matID array[]){
    // Map storing the occurances
    std::map<matID, short> histogram;

    // Variable storing the number of times the element which appeared most appeared (read that statement three times fast!)
    short mode_count = 0;
    // For every element in the array
    for (int i = 0; i < 8; i++) {
        // Increment the count in the histogram associated with that matID
        histogram[array[i]]++;
        // Update the <mode_count> (if applicable)
        mode_count = std::max(mode_count, histogram[array[i]]);
    }

    // second = value, first = key
    // Return the matID (non 0) which appeared most often
    for (auto element : histogram)
        if (element.second == mode_count && element.first != 0)
            return element.first;
    // If nothing tied or beat 0, then this face must not render!
    return 0;
}

bool mode(bool array[]){
    // Variable storing the number of times the element which appeared most appeared (read that statement three times fast!)
    short true_count = 0;
    // For every element in the array
    for (int i = 0; i < 8; i++)
        if(array[i])
            true_count++;

    if(true_count >= 4)
        return true;
    return false;
}

bool areAnyTransparent(bool array[]){
    for(int i = 0; i < 8; i++)
        if(!array[i]) return true;
    return false;
}

/*
NAME:           updateSubChunks()
DESCRIPTION:    Updates matIDs and opacity of the chunk/subchunk
*/
void Chunk::updateSubChunks(){
    /*
        The logic in this function isn't hard, it is just repetitive,
        thus only the logic for the top layer will be commented

        Just remember that the information is all getting pulled up from the subChunk/Block below
    */
    // Variable storing how many total blocks are opaque
    int opaque = 0,
        // Counter storing which iteration this is
        i = 0;
    // Variables storing all of the matIDs of the subChunks
    matID cup[8], cdown[8], cleft[8], cright[8], cfront[8], cback[8];
    bool copaque[8];
    // Loop through all of the subChunks and fill in these variables
    for(SubChunk8& c8: subChunks){ // c8
        int c8I = 0;
        matID c8up[8], c8down[8], c8left[8], c8right[8], c8front[8], c8back[8];
        bool c8opaque[8];
        for(SubChunk4& c4: c8.subChunks){ // c4
            int c4I = 0;
            matID c4up[8], c4down[8], c4left[8], c4right[8], c4front[8], c4back[8];
            bool c4opaque[8];
            for(SubChunk2& c2: c4.subChunks){ // c2
                int c2I = 0;
                matID c2up[8], c2down[8], c2left[8], c2right[8], c2front[8], c2back[8];\
                bool c2opaque[8];
                for(Block& block: c2.blocks){ // block
                    if(block.blockRef->opaque)
                        c2opaque[c2I] = true;
                    else
                        c2opaque[c2I] = false;

                    c2up[c2I] = block.blockRef->up; c2down[c2I] = block.blockRef->down;
                    c2left[c2I] = block.blockRef->left; c2right[c2I] = block.blockRef->right;
                    c2front[c2I] = block.blockRef->front; c2back[c2I] = block.blockRef->back;

                    c2I++;
                } // Block
                c2.up = mode(c2up); c2.down = mode(c2down);
                c2.left = mode(c2left); c2.right = mode(c2right);
                c2.front = mode(c2front); c2.back = mode(c2back);
                c2.opaque = mode(c2opaque);
                c2.anyTransparent = areAnyTransparent(c2opaque);

                c4up[c4I] = c2.up; c4down[c4I] = c2.down;
                c4left[c4I] = c2.left; c4right[c4I] = c2.right;
                c4front[c4I] = c2.front; c4back[c4I] = c2.back;
                c4opaque[c4I] = c2.opaque;

                c4I++;
            } // c2
            c4.up = mode(c4up); c4.down = mode(c4down);
            c4.left = mode(c4left); c4.right = mode(c4right);
            c4.front = mode(c4front); c4.back = mode(c4back);
            c4.opaque = mode(c4opaque);
            c4.anyTransparent = areAnyTransparent(c4opaque);

            c8up[c8I] = c4.up; c8down[c8I] = c4.down;
            c8left[c8I] = c4.left; c8right[c8I] = c4.right;
            c8front[c8I] = c4.front; c8back[c8I] = c4.back;
            c8opaque[c8I] = c4.opaque;

            c8I++;
        } // c4
        c8.up = mode(c8up); c8.down = mode(c8down);
        c8.left = mode(c8left); c8.right = mode(c8right);
        c8.front = mode(c8front); c8.back = mode(c8back);
        c8.opaque = mode(c8opaque);
        c8.anyTransparent = areAnyTransparent(c8opaque);

        cup[i] = c8.up; cdown[i] = c8.down;
        cleft[i] = c8.left; cright[i] = c8.right;
        cfront[i] = c8.front; cback[i] = c8.back;
        copaque[i] = c8.opaque;

        i++;
    } // c8
    // Set the matIDs to be the mode of all of the subChunk matIDs for that face
    up = mode(cup); down = mode(cdown);
    left = mode(cleft); right = mode(cright);
    front = mode(cfront); back = mode(cback);
    opaque = mode(copaque);
    anyTransparent = areAnyTransparent(copaque);
}

/*
NAME:           saveChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Serializes the provided <chunk> to the provided <fileName>
*/
void saveChunk(char const* fileName, Chunk & chunk){
    // Propagate changes to blocks through the chunk
    chunk.updateSubChunks();

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

    // Close the connection to the file
    os.close();
}

/*
NAME:           loadChunk(char const* fileName, Chunk & chunk)
DESCRIPTION:    Loads the chunked stored at <fileName> into the variable <chunk>
*/
void loadChunk(char const* fileName, Chunk & chunk){
    // Open a connection to the file
    std::ifstream is(fileName, std::ios::binary);

    // Load from the file
    cereal::PortableBinaryInputArchive archive( is );
    archive( CEREAL_NVP(chunk) );

    // Close the connection to the file
    is.close();
}
