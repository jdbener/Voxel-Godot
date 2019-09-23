#include "ChunkManager.hpp"
#include "timer.h"

using namespace std;

void countBlocks(VoxelInstance* block, int index){
	//cout << index << " - " << block->center.to_string() << "\n";
}

int main(){
	cout << "Chunk Dimension: " << CHUNK_DIMENSIONS << ", Blocks per Chunk: " << CHUNK_ARRAY_SIZE << ", Maximum Octree Levels: " << SUBCHUNK_LEVELS << endl;

	Timer t;
	ChunkManager map(0, 0, 0, 3);
	repeat(500)
		map.tick();

	cout << int(map.chunkMap[0][0][0]->find(0, {.5, .5, .5})->level) << endl;
	cout << map.findBlock({-7.5, 16.5, -7.5})->center.to_string() << endl;
	map.unloadChunk({-7.5, 16.5, -7.5});
	cout << "Blocks: " << map.findChunk({-7.5, 6.5, -7.5})->iteraterate(0, countBlocks) << endl;
}
