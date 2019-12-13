#include "ChunkMap.h"
#include <OpenSimplexNoise.hpp>
#include <fstream>

void ChunkMap::_ready(){
	// Zero out all of the chunks
	chunk = new Chunk* [CHUNK_MAP_SIZE];
	for(int i = 0; i < CHUNK_MAP_SIZE; i++) chunk[i] = nullptr;
	load(Vector3());
}

void ChunkMap::save(Vector3& position){
	for(int i = 0; i < CHUNK_MAP_SIZE; i++)
		if(chunk[i])
			if(chunk[i]->center == position){
				std::ofstream os(("world/" + String(chunk[i]->center) + ".chunk.json").utf8().get_data(), std::ios::binary);
		        oarchive save(os);
		        save(*chunk[i]);
				return;
			}
}

void ChunkMap::load(Vector3& position){ 
	std::ifstream is(("world/" + String(position) + ".chunk.json").utf8().get_data(), std::ios::binary);
	// TODO: Calculate proper index
	int i = 1;
	// If the file doesn't exist generate the chunk
	if(!is){
		chunk[i] = generateChunk(position);
		save(position);
		return;
	}

	// If the file does exist load the chunk from the file
	iarchive load(is);
	chunk[i] = Chunk::_new();
	load(*chunk[i]);
}


Chunk* ChunkMap::generateChunk(Vector3& position){
	OpenSimplexNoise* noise = OpenSimplexNoise::_new();
	noise->set_octaves(4);
	noise->set_period(20.0);
	noise->set_persistence(0.8);


	Chunk* out = Chunk::_new();
	out->center = position;
    out->initalize(this);
    out->iterate(BLOCK_LEVEL, [noise](VoxelInstance* v, int) {
        //2x + 3y + 5z = 5; y = 5/3 - 2/3x - 5/3z
        int y = 5.0/3.0 - 2.0 / 3.0 * v->center.z - 5.0/3.0 * v->center.z;
        if(noise->get_noise_3dv(v->center) > 0)
        //if (v->center.y < y)
        //if(v->center.y > 0)
            v->blockData = BlockDatabase::getSingleton()->getBlock(1);
        else
            v->blockData = BlockDatabase::getSingleton()->getBlock(0);
    });
    //out->recalculate();
    out->prune();
    out->recalculate();
	return out;
}
