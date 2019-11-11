#include "ChunkMap.h"

#include <OpenSimplexNoise.hpp>

void ChunkMap::_ready(){
    OpenSimplexNoise* noise = OpenSimplexNoise::_new();
    noise->set_octaves(4);
    noise->set_period(20.0);
    noise->set_persistence(0.8);

    chunk = Chunk::_new();
    chunk->initalize(this);
    chunk->iterate(BLOCK_LEVEL, [noise](VoxelInstance* v, int) {
        //2x + 3y + 5z = 5; y = 5/3 - 2/3x - 5/3z
        int y = 5.0/3.0 - 2.0 / 3.0 * v->center.z - 5.0/3.0 * v->center.z;
        if(noise->get_noise_3dv(v->center) > 0)
        //if (v->center.y < y)
        //if(v->center.y > 0)
            v->blockData = BlockDatabase::getSingleton()->getBlock(1);
        else
            v->blockData = BlockDatabase::getSingleton()->getBlock(0);
    });
    chunk->recalculate();
    chunk->prune();
    chunk->recalculate();
}
