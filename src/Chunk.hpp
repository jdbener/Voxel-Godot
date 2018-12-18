/*
FILE:               Chunk.hpp
DESCRIPTION:        Header file for Chunk.cpp

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented Chunk, Subchunk8/4/2, and serialization
*/

#ifndef CHUNK_H
#define CHUNK_H

#include "Defs.hpp"
#include "Vector3Temp.hpp"
#include "Block.hpp"
#include <ostream>
#include <string>

class ChunkBase: public Voxel {
public:
    static const short SCALE = 0;
    // Material IDS
    unsigned short up, down, left, right, front, back;

    ChunkBase(Vector3 center) : Voxel(center) { }
    ChunkBase() { }

    virtual Vector3 getCenter(bool worldScale = true) { return Vector3(); }

    template <class Archive> void serialize( Archive & ar );
};

class SubChunk2: public ChunkBase {
public:
    static const short SCALE = 2; // Scale (in World Scale) of this sub-chunks
    Block blocks[SUB_PER_WHOLE]; // Array storing the blocks which this sub-chunk approximates
    Chunk* chunk; // Pointer to the owner chunk

    SubChunk2(Chunk* _chunk) : chunk(_chunk), blocks({Block(_chunk), Block(_chunk), Block(_chunk), Block(_chunk), Block(_chunk), Block(_chunk), Block(_chunk), Block(_chunk)}) { }

    Vector3 getCenter(bool worldScale = true);

    template <class Archive> void serialize( Archive & ar );
};

class SubChunk4: public ChunkBase {
public:
    static const short SCALE = 4; // Scale (in World Scale) of this sub-chunk
    SubChunk2 subChunks[SUB_PER_WHOLE]; // Array storing the sub-chunks which this sub-chunk approximates
    Chunk* chunk;

    SubChunk4(Chunk* _chunk) : chunk(_chunk), subChunks({SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk), SubChunk2(_chunk)}) { }

    Vector3 getCenter(bool worldScale = true);

    template <class Archive> void serialize( Archive & ar );
};

class SubChunk8: public ChunkBase {
public:
    static const short SCALE = 8; // Scale (in World Scale) of this sub-chunk
    SubChunk4 subChunks[SUB_PER_WHOLE]; // Array storing the sub-chunks which this sub-chunk approximates
    Chunk* chunk;

    SubChunk8(Chunk* _chunk) : chunk(_chunk), subChunks({SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk), SubChunk4(_chunk)}) { }

    Vector3 getCenter(bool worldScale = true);

    template <class Archive> void serialize( Archive & ar );
};

class Chunk: public ChunkBase {
public:
    static const short SCALE = 16; // Scale (in World Scale) of this chunk
    SubChunk8 subChunks[SUB_PER_WHOLE]; // Array storing the tree of sub0chunks this chunk owns

    bool solid = false; // Variable storing whether or not this chunk is solid

    Chunk(Vector3 center) : ChunkBase(center), subChunks({SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this)}) { }
    Chunk() : subChunks({SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this), SubChunk8(this)}) { }

    void setSoldity(short solidBlocks);
    Vector3 getCenter(bool worldScale = true);

    template <class Archive> void serialize( Archive & ar );
};

// Tell cereal which of the inherited serialization functions to use
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( ChunkBase, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Chunk, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk8, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk4, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk2, cereal::specialization::member_serialize )

void writeChunk(char const* fileName, Chunk & chunk);
void loadChunk(char const* fileName, Chunk & chunk);

std::ostream& operator<< (std::ostream& stream, Vector3 vec);
std::ostream& operator<< (std::ostream& stream, ChunkBase chunk);
std::ostream& operator<< (std::ostream& stream, SubChunk2 chunk);
std::ostream& operator<< (std::ostream& stream, SubChunk4 chunk);
std::ostream& operator<< (std::ostream& stream, SubChunk8 chunk);
std::ostream& operator<< (std::ostream& stream, Chunk chunk);

#endif
