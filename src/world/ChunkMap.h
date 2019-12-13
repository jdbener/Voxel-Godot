#ifndef __CHUNK_MAP_H__
#define __CHUNK_MAP_H__
#include "Chunk.h"
#include <Spatial.hpp>

const int LOD_DISTANCE = 1; // The number of chunks before a chunk is reduced to a lower level of detail
const int VIEW_DISTANCE = LOD_DISTANCE * SUBCHUNK_LEVELS; // The number of chunks a player will be able to see
const int CHUNK_MAP_SIZE = VIEW_DISTANCE * VIEW_DISTANCE * VIEW_DISTANCE * 8;

class ChunkMap: public Spatial {
    GODOT_CLASS(ChunkMap, Spatial)
public:
    static void _register_methods(){
		register_method("_ready", &ChunkMap::_ready);
    }
    void _init() {}

    // Just a single chunk for now TODO: make it an actual chunk map
    Chunk** chunk = nullptr;

    void _ready();
	Chunk* generateChunk(Vector3& position);
	Chunk* generateChunk(Vector3&& position) { return generateChunk(position); }

	void save(Vector3& position);
	void save(Vector3&& position) { save(position); }
	void load(Vector3& position);
	void load(Vector3&& position) { load(position); }

    VoxelInstance* find(int lvl, Vector3& position){
		//for(int i = 0; i < VIEW_DISTANCE * VIEW_DISTANCE * VIEW_DISTANCE; i++)
		for(int i = 0; i < CHUNK_MAP_SIZE; i++) // Fix so that I am only looking at one chunk
			if(chunk[i])
				if(chunk[i]->within(position))
					return chunk[i]->find(lvl, position);
		return nullptr;
    }
	VoxelInstance* find(int lvl, Vector3&& position) { return find(lvl, position); }
};

#endif //__CHUNK_MAP_H__
