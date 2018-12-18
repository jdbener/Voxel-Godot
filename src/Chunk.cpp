/*
FILE:               Chunk.cpp
DESCRIPTION:	    Implements a 16 block^3 chunk which contains several layers of
                    sub-chunk. Additionally serialization has been implemented.

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-18         0.0 - Implemented Chunk, Subchunk8/4/2, and serialization
*/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

#include <cereal/archives/portable_binary.hpp>

#include "Chunk.hpp"

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
NAME:           ChunkBase::serialize( Archive & ar )
DESCRIPTION:    Function used to tell the serialization library what variables to serialize
*/
template <class Archive>
void ChunkBase::serialize( Archive & ar ) {
    // First serialize the inhertience hierachy
    Voxel::serialize( ar );
    // Then serialize this class's members
    ar ( cereal::make_nvp("MatID-up", up), cereal::make_nvp("MatID-down", down),
         cereal::make_nvp("MatID-left", left), cereal::make_nvp("MatID-right", right),
         cereal::make_nvp("MatID-front", front), cereal::make_nvp("MatID-back", back) );
}

/*
NAME:           Chunk::serialize( Archive & ar )
DESCRIPTION:    Function used to tell the serialization library what variables to serialize
*/
template <class Archive>
void Chunk::serialize( Archive & ar ) {
    // First serialize the inhertience hierachy
    ChunkBase::serialize( ar );
    // Then serialize this class's members
    ar (cereal::make_nvp("Solid", solid));
    /*
        This may be removed in the future if I decide to commit to binary chunks instead of switchablity
    */
    // The array of sub-chunks is serialized in steps so that human identifiable names can be generated
    for(int i = 0; i < SUB_PER_WHOLE; i++)
        ar (cereal::make_nvp("SubChunk8-" + std::to_string(i), subChunks[i]));
}
// Minified versions of the function for the sub-chunks
template <class Archive> void SubChunk8::serialize( Archive & ar ) { ChunkBase::serialize( ar ); for(int i = 0; i < SUB_PER_WHOLE; i++) ar (cereal::make_nvp("SubChunk4-" + std::to_string(i), subChunks[i])); }
template <class Archive> void SubChunk4::serialize( Archive & ar ) { ChunkBase::serialize( ar ); for(int i = 0; i < SUB_PER_WHOLE; i++) ar (cereal::make_nvp("SubChunk2-" + std::to_string(i), subChunks[i])); }
template <class Archive> void SubChunk2::serialize( Archive & ar ) { ChunkBase::serialize( ar ); for(int i = 0; i < SUB_PER_WHOLE; i++) ar (cereal::make_nvp("Block-" + std::to_string(i), blocks[i])); }

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

int main(){
    srand (time(NULL));

    const int SIZE = 10;
    Chunk array[SIZE];

    for(Chunk& cur: array){
        cur = Chunk(Vector3(rand() % 5, rand() % 6, rand() % 7));
        cur.setSoldity(rand() % BLOCKS_PER_CHUNK);
    }

    for(Chunk cur: array)
        std::cout << cur << std::endl;

    writeChunk("test.chunk", array[0]);

    Chunk load;
    loadChunk("test.chunk", load);

    int total = 0;
    int nullTotal = 0;
    for(SubChunk8 c8: load.subChunks)
        for(SubChunk4 c4: c8.subChunks)
            for(SubChunk2 c2: c4.subChunks)
                for(Block b: c2.blocks){
                    if(b == Blocks::null) nullTotal++;
                    total++;
                }
    std::cout << "Total Blocks: " << total << std::endl
                << "Total NULL: " << nullTotal << std::endl
                << "Load Proof: " << load.getCenter(false) << std::endl;

    system("pause");

    return 0;
}








/*
    Stream insertion opperators for all of the types implemented here (for debug purposes)
*/
std::ostream& operator<< (std::ostream& stream, Vector3 pos){
    stream << "{" << pos.x << ", " << pos.y << ", " << pos.z << "}";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, ChunkBase chunk){
    stream << "{faces (UDLRFB) = " << chunk.up << ", " << chunk.down << ", " << chunk.left
            << ", " << chunk.right << ", " << chunk.front << ", " << chunk.back << "}";
    return stream;
}

std::ostream& operator<< (std::ostream& stream, Chunk chunk){
    stream << "{" << ChunkBase(chunk) << ", center = " << chunk.getCenter(false) << ", solidity = " << (chunk.solid ? " true" : "false") << ", scale = " << chunk.SCALE << "}";
    return stream;
}
// Minified versions of the function for the sub-chunks
std::ostream& operator<< (std::ostream& stream, SubChunk8 chunk){ stream << "{" << ChunkBase(chunk) << ", center = " << chunk.getCenter(false) << ", scale = " << chunk.SCALE << "}"; return stream; }
std::ostream& operator<< (std::ostream& stream, SubChunk4 chunk){ stream << "{" << ChunkBase(chunk) << ", center = " << chunk.getCenter(false) << ", scale = " << chunk.SCALE << "}"; return stream; }
std::ostream& operator<< (std::ostream& stream, SubChunk2 chunk){ stream << "{" << ChunkBase(chunk) << ", center = " << chunk.getCenter(false) << ", scale = " << chunk.SCALE << "}"; return stream; }
