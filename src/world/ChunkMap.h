#ifndef __CHUNK_MAP_H__
#define __CHUNK_MAP_H__
#include "Chunk.h"
#include <Spatial.hpp>

class ChunkMap: public Spatial {
    GODOT_CLASS(ChunkMap, Spatial)
public:
    static void _register_methods(){
		register_method("_ready", &ChunkMap::_ready);
    }
    void _init() {}

    // Just a single chunk for now TODO: make it an actual chunk map
    Chunk* chunk = nullptr;

    void _ready();

    VoxelInstance* find(int lvl, Vector3& position){
        return chunk->find(lvl, position);
    }
	VoxelInstance* find(int lvl, Vector3&& position) { return find(lvl, position); }
};

#endif //__CHUNK_MAP_H__
