#ifndef __CHUNK_MAP_H__
#define __CHUNK_MAP_H__
#include "Chunk.h"

class ChunkMap {
public:
    // Just a single chunk for now TODO: make it an actual chunk map
    Chunk* chunk;

    // TODO: change to _on_enter_tree()
    void init(){
        chunk = Chunk::_new();
        chunk->initalize(this);
        chunk->iterate(BLOCK_LEVEL, [](VoxelInstance* v, int) {
            //2x + 3y + 5z = 5; y = 5/3 - 2/3x - 5/3z
            int y = 5.0/3.0 - 2.0 / 3.0 * v->center.z - 5.0/3.0 * v->center.z;
            if (v->center.y < y)
            //if(v->center.y > 0)
                v->blockData = BlockDatabase::getSingleton()->getBlock(1);
            else
                v->blockData = BlockDatabase::getSingleton()->getBlock(0);
        });
        chunk->recalculate();
        chunk->prune();
        chunk->recalculate();
    }

    VoxelInstance* find(int lvl, Vector3& position){
        return chunk->find(lvl, position);
    }
	VoxelInstance* find(int lvl, Vector3&& position) { return find(lvl, position); }
};

#endif //__CHUNK_MAP_H__
