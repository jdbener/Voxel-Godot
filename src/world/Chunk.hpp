/*
FILE:               Chunk.hpp
DESCRIPTION:        Header file for Chunk.cpp

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented Chunk, Subchunk8/4/2, and serialization
Joshua Dahl        2018-12-18         0.1 - Reworked initialization to propagate position
Joshua Dahl        2018-12-19         1.0 - Godotized file
*/

#ifndef CHUNK_H
#define CHUNK_H

#include "Block.hpp"

#include <Vector3.hpp>

using namespace godot;

const Vector3 i0 = Vector3(1, 1, 1),
        i1 = Vector3(-1, 1, 1),
        i2 = Vector3(-1, 1, -1),
        i3 = Vector3(1, 1, -1),
        i4 = Vector3(1, -1, 1),
        i5 = Vector3(-1, -1, 1),
        i6 = Vector3(-1, -1, -1),
        i7 = Vector3(1, -1, -1);

class ChunkBase: public Voxel {
public:
    static const short SCALE = 0;
    // Material IDS
    unsigned short up, down, left, right, front, back;

    ChunkBase(Vector3 center) : Voxel(center) { }
    ChunkBase() { }

    virtual Vector3 getCenter(bool worldScale = true) { return Vector3(); }

    /*
    NAME:           serialize( Archive & ar )
    DESCRIPTION:    Function used to tell the serialization library what variables to serialize
    */
    template <class Archive>
    void serialize( Archive & ar ) {
        // First serialize the inhertience hierachy
        Voxel::serialize( ar );
        // Then serialize this class's members
        ar ( cereal::make_nvp("MatID-up", up), cereal::make_nvp("MatID-down", down),
             cereal::make_nvp("MatID-left", left), cereal::make_nvp("MatID-right", right),
             cereal::make_nvp("MatID-front", front), cereal::make_nvp("MatID-back", back) );
    }
};

class SubChunk2: public ChunkBase {
public:
    static const short SCALE = 2; // Scale (in World Scale) of this sub-chunks
    Block blocks[SUB_PER_WHOLE]; // Array storing the blocks which this sub-chunk approximates
    Chunk* chunk; // Pointer to the owner chunk

    SubChunk2(Chunk* _chunk, Vector3 center) : ChunkBase(center), chunk(_chunk), blocks({Block(_chunk, center + i0), Block(_chunk, center + i1), Block(_chunk, center + i2),
                                Block(_chunk, center + i3), Block(_chunk, center + i4), Block(_chunk, center + i5), Block(_chunk, center + i6), Block(_chunk, center + i7)})
                                { }

    Vector3 getCenter(bool worldScale = true);

    /*
    NAME:           serialize( Archive & ar )
    DESCRIPTION:    Function used to tell the serialization library what variables to serialize
    */
    template <class Archive>
    void serialize( Archive & ar ) {
        // First serialize the inhertience hierachy
        ChunkBase::serialize( ar );
        /*
            This may be removed in the future if I decide to commit to binary chunks instead of switchablity
        */
        // The array of sub-chunks is serialized in steps so that human identifiable names can be generated
        for(int i = 0; i < SUB_PER_WHOLE; i++)
            ar (cereal::make_nvp("SubChunk8-" + std::to_string(i), blocks[i]));
    }
};

class SubChunk4: public ChunkBase {
public:
    static const short SCALE = 4; // Scale (in World Scale) of this sub-chunk
    SubChunk2 subChunks[SUB_PER_WHOLE]; // Array storing the sub-chunks which this sub-chunk approximates
    Chunk* chunk; // Pointer to the owner chunk

    SubChunk4(Chunk* _chunk, Vector3 center) : ChunkBase(center), chunk(_chunk), subChunks({SubChunk2(_chunk, center + Vector3(2, 2, 2) * i0),
                        SubChunk2(_chunk, center + Vector3(2, 2, 2) * i1), SubChunk2(_chunk, center + Vector3(2, 2, 2) * i2), SubChunk2(_chunk, center + Vector3(2, 2, 2) * i3),
                        SubChunk2(_chunk, center + Vector3(2, 2, 2) * i4), SubChunk2(_chunk, center + Vector3(2, 2, 2) * i5), SubChunk2(_chunk, center + Vector3(2, 2, 2) * i6),
                        SubChunk2(_chunk, center + Vector3(2, 2, 2) * i7)}) { }

    Vector3 getCenter(bool worldScale = true);

    /*
    NAME:           serialize( Archive & ar )
    DESCRIPTION:    Function used to tell the serialization library what variables to serialize
    */
    template <class Archive>
    void serialize( Archive & ar ) {
        // First serialize the inhertience hierachy
        ChunkBase::serialize( ar );
        /*
            This may be removed in the future if I decide to commit to binary chunks instead of switchablity
        */
        // The array of sub-chunks is serialized in steps so that human identifiable names can be generated
        for(int i = 0; i < SUB_PER_WHOLE; i++)
            ar (cereal::make_nvp("SubChunk8-" + std::to_string(i), subChunks[i]));
    }
};

class SubChunk8: public ChunkBase {
public:
    static const short SCALE = 8; // Scale (in World Scale) of this sub-chunk
    SubChunk4 subChunks[SUB_PER_WHOLE]; // Array storing the sub-chunks which this sub-chunk approximates
    Chunk* chunk; // Pointer to the owner chunk

    SubChunk8(Chunk* _chunk, Vector3 center) : ChunkBase(center), chunk(_chunk), subChunks({SubChunk4(_chunk, center + Vector3(4, 4, 4) * i0),
                        SubChunk4(_chunk, center + Vector3(4, 4, 4) * i1), SubChunk4(_chunk, center + Vector3(4, 4, 4) * i2), SubChunk4(_chunk, center + Vector3(4, 4, 4) * i3),
                        SubChunk4(_chunk, center + Vector3(4, 4, 4) * i4), SubChunk4(_chunk, center + Vector3(4, 4, 4) * i5), SubChunk4(_chunk, center + Vector3(4, 4, 4) * i6),
                        SubChunk4(_chunk, center + Vector3(4, 4, 4) * i7)}) { }

    Vector3 getCenter(bool worldScale = true);

    /*
    NAME:           serialize( Archive & ar )
    DESCRIPTION:    Function used to tell the serialization library what variables to serialize
    */
    template <class Archive>
    void serialize( Archive & ar ) {
        // First serialize the inhertience hierachy
        ChunkBase::serialize( ar );
        /*
            This may be removed in the future if I decide to commit to binary chunks instead of switchablity
        */
        // The array of sub-chunks is serialized in steps so that human identifiable names can be generated
        for(int i = 0; i < SUB_PER_WHOLE; i++)
            ar (cereal::make_nvp("SubChunk8-" + std::to_string(i), subChunks[i]));
    }
};

class Chunk: public ChunkBase {
public:
    static const short SCALE = 16; // Scale (in World Scale) of this chunk
    SubChunk8 subChunks[SUB_PER_WHOLE]; // Array storing the tree of sub0chunks this chunk owns

    bool solid = false; // Variable storing whether or not this chunk is solid

    Chunk(Vector3 center) : ChunkBase(center), subChunks({SubChunk8(this, center + Vector3(8, 8, 8) * i0), SubChunk8(this, center + Vector3(8, 8, 8) * i1),
                            SubChunk8(this, center + Vector3(8, 8, 8) * i2), SubChunk8(this, center + Vector3(8, 8, 8) * i3), SubChunk8(this, center + Vector3(8, 8, 8) * i4),
                            SubChunk8(this, center + Vector3(8, 8, 8) * i5), SubChunk8(this, center + Vector3(8, 8, 8) * i6), SubChunk8(this, center + Vector3(8, 8, 8) * i7)}) { }
    Chunk() : subChunks({SubChunk8(this, Vector3(8, 8, 8) * i0), SubChunk8(this, Vector3(8, 8, 8) * i1),
                SubChunk8(this, Vector3(8, 8, 8) * i2), SubChunk8(this, Vector3(8, 8, 8) * i3), SubChunk8(this, Vector3(8, 8, 8) * i4),
                SubChunk8(this, Vector3(8, 8, 8) * i5), SubChunk8(this, Vector3(8, 8, 8) * i6), SubChunk8(this, Vector3(8, 8, 8) * i7)}) { }

    void setSoldity(short solidBlocks);
    Vector3 getCenter(bool worldScale = true);

    /*
    NAME:           serialize( Archive & ar )
    DESCRIPTION:    Function used to tell the serialization library what variables to serialize
    */
    template <class Archive>
    void serialize( Archive & ar ) {
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
};

// Tell cereal which of the inherited serialization functions to use
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( ChunkBase, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Chunk, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk8, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk4, cereal::specialization::member_serialize )
CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( SubChunk2, cereal::specialization::member_serialize )

// Serialization Procedures
void writeChunk(char const* fileName, Chunk & chunk);
void loadChunk(char const* fileName, Chunk & chunk);

#endif