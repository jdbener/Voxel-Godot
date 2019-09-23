#include "Chunk.hpp"
#include "repeat.h"
#include <iostream>
#include <queue>
#include <tuple>
#include <mutex>

using namespace std;

// Thread safe queue
template <class T>
class squeue: public queue<T> {
private:
	// Fence used to make a few functions threadsafe
	mutex m;
public:
	T pop_front(){
		m.lock();
		T tmp = queue<T>::front();
		queue<T>::pop();
		m.unlock();

		return tmp;
	}

	void pop(){
		m.lock();
		queue<T>::pop();
		m.unlock();
	}

	void push(const T& val){
		m.lock();
		queue<T>::push(val);
		m.unlock();
	}

	void push(T&& val){
		push(val);
	}


};

ostream& operator<<(ostream& stream, tuple<int&, int&, int&> chords){
	stream << get<0>(chords) << " - " << get<1>(chords) <<  " - " << get<2>(chords);
	return stream;
}



class ChunkManager {
public:
	Chunk**** chunkMap;
	int centerX, centerY, centerZ, radius;
	squeue<Chunk**> deletionQueue;
	squeue<tuple<int, int, int>> loadQueue;

	ChunkManager(int xInitial, int yInitial, int zInitial, int radius = 0){
		tie(centerX, centerY, centerZ, this->radius) = tie(xInitial, yInitial, zInitial, radius);
		{
			const int dimensions = radius * 2 + 1;
			chunkMap = new Chunk*** [dimensions];
			repeat(dimensions, i){
				chunkMap[i] = new Chunk** [dimensions];
				repeat(dimensions, j)
					chunkMap[i][j] = new Chunk* [dimensions];
			}
		}

		for(int x = xInitial - radius; x <= xInitial + radius; x++){
			for(int z = zInitial - radius; z <= zInitial + radius; z++)
				for(int y = yInitial - radius; y <= yInitial + radius; y++)
					loadQueue.emplace(x, y, z);
			cout << endl;
		}

		// Load all of the chunks
		// TODO: implement actual loader
		//while(loadOne());
	}

	~ChunkManager(){
		// Unload all of the chunks left in the deletionQueue
		while(unloadOne());

		// Unload all of the chunks not marked for deletion
		repeat(radius * 2 + 1, x)
			if(chunkMap[x]){
				repeat(radius * 2 + 1, z)
					if(chunkMap[x][z]){
						repeat(radius * 2 + 1, y)
							if(chunkMap[x][z][y])
								delete chunkMap[x][z][y];
						delete [] chunkMap[x][z];
					}
				delete [] chunkMap[x];
			}
		delete [] chunkMap;
	}

	bool loadOne(){
		if(loadQueue.empty())
			return false;

		int x, y, z;
		tie(x, y, z) = loadQueue.pop_front();
		at(x, y, z) = new Chunk({float(x * CHUNK_DIMENSIONS), float(y * CHUNK_DIMENSIONS), float(z * CHUNK_DIMENSIONS)}, true);
		cout << x << " - " << y << " - " << z << '\n';
		return true;
	}

	bool unloadOne(){
		if(deletionQueue.empty())
			return false;

		Chunk** toDelete = deletionQueue.pop_front();
		if(*toDelete){
			delete *toDelete;
			*toDelete = nullptr;
		}
		return true;
	}

	void tick(){
		// Load and unload a chunk if nessicary
		loadOne();
		unloadOne();
	}

	Chunk***& at(int x){
		x = x + radius - centerX;
		return chunkMap[x];
	}

	Chunk**& at(int x, int z){
		z = z + radius - centerZ;
		return at(x)[z];
	}

	Chunk*& at(int x, int y, int z){
		y = y + radius - centerY;
		return at(x, z)[y];
	}

	Chunk* findChunk(vec3 position){
		world2chunkmap(position);
		return at(position.x, position.z, position.y);
	}

	VoxelInstance* findSubChunk(const int level, vec3 position){
		Chunk* chunk = findChunk(position);
		if(chunk) return chunk->find(level, position);
		return nullptr;
	}

	VoxelInstance* findBlock(vec3 position){
		return findSubChunk(0, position);
	}

	void unloadRow(int x){
		// Add all of the unloaded chunks to the deletion queue
		if(at(x))
			repeat(radius * 2 + 1, z)
				if(at(x)[z]){
					repeat(radius * 2 + 1, y)
						if(at(x)[z][y])
							deletionQueue.push(&at(x)[z][y]);
					// Additionally... delete the columns
					delete [] at(x)[z];
				}

		// Delete the row
		delete [] at(x);
		at(x) = nullptr;

	}

	void unloadRow(vec3 position){
		world2chunkmap(position);
		unloadRow(position.x);
	}

	void unloadColumn(int x, int z){
		// Add all of the unloaded chunks to the deletion queue
		if(at(x))
			if(at(x, z))
				repeat(radius * 2 + 1, y)
					if(at(x, z)[y])
						deletionQueue.push(&at(x, z)[y]);

		// Delete the column
		delete [] at(x, z);
		at(x, z) = nullptr;
	}

	void unloadColumn(vec3 position){
		world2chunkmap(position);
		unloadColumn(position.x, position.z);
	}

	void unloadChunk(int x, int y, int z){
		// Add all of the unloaded chunks to the deletion queue
		if(at(x))
			if(at(x, z))
				if(at(x, y, z))
					deletionQueue.push(&at(x, y, z));
	}

	void unloadChunk(vec3 position){
		world2chunkmap(position);
		unloadChunk(position.x, position.y, position.z);
	}

private:
	// Remaps general block/subchunk choordinates to integer chunkMap indicies
	void world2chunkmap(vec3& position){
		if(position.x < 0) position.x -= CHUNK_DIMENSIONS / 2;
		else position.x += CHUNK_DIMENSIONS / 2;

		if(position.y < 0) position.y -= CHUNK_DIMENSIONS / 2;
		else position.y += CHUNK_DIMENSIONS / 2;

		if(position.z < 0) position.z -= CHUNK_DIMENSIONS / 2;
		else position.z += CHUNK_DIMENSIONS / 2;

		position.x = int(position.x / CHUNK_DIMENSIONS);
		position.y = int(position.y / CHUNK_DIMENSIONS);
		position.z = int(position.z / CHUNK_DIMENSIONS);
	}
};
