/*
FILE:              ChunkMap.cpp
DESCRIPTION:	   Provides an implementation of a Random Access sphere (any point
					can be accessed in less than 20 microsec), which is used to
					quickly interface with an array of voxel chunks.

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-09         0.0 - Implemented sphere generation, indexing, and access
Joshua Dahl		   2018-12-10		  0.1 - Implemented subcube/sphere access
Joshua Dahl		   2018-12-11		  0.2 - Rewritten initialization code to support origins other than (0, 0, 0)
										Additionally the sphere can now be reinitialized with a new origin,
										all of the points shared between the old and new spheres are transfered
										as opposed to being recreated.
Joshua Dahl		   2018-12-13		  0.3 - Optimized Indexing and re-factored getPoint() to not use a search algorithm
Joshua Dahl		   2018-12-14		  1.0 - Godotized FILE
Joshua Dahl		   2018-12-17		  1.1 - Bug fixes on getSphere, exposed getInternalArray for use in GDScript, and split off the index fetching
                                        code in getPoint to getPointIndex
Joshua Dahl		   2018-12-19		  1.2 - Implemented getShereRim and getCubeRim, split the cube/sphere getters into two functions (one returning
										A vector and the other returning a Godot Array)
Joshua Dahl		   2018-12-26		  2.0 - Degodotized file for integration with ChunkRenderer
*/
#include "ChunkGenerator.hpp"

#include "ChunkMap.hpp"
#include "Godotize.hpp"
#include <algorithm>

/*
FUNCTION:          CONSTRUCTOR()
DESCRIPTION:       Creates the sphere (centered at <origin> of radius <radius>)
					and indexes the elements
*/
ChunkMap::ChunkMap(int _radius, Vector3 _origin, bool _regen) : radius(_radius), origin(_origin), regen(_regen) {
	initSphere();
	initIndex();
}

/*
FUNCTION:          DESTRUCTOR()
DESCRIPTION:       Cleans up the array and index
*/
ChunkMap::~ChunkMap() {
	sphere.clear();
	//for(int i = -radius; i <= radius; i++)
	//	index[i].clear();
	index.clear();
}

/*
FUNCTION:          reinitSphere(Vector3 translation)
DESCRIPTION:       Regenerates the array, changing the <origin> to reflect the
 					provided <translation>, reusing as many points as possible
*/
void ChunkMap::reinitSphere(Vector3 translation){
	// Create (and reserve) a new temporary array to hold the new sphere
	std::vector<Chunk*> tempSphere;
	/*
		Considered pre-reserving the required space but... Any benefit is well
		within the bounds of experimentation error
	*/
	// Generate cube
	for(int x = radius; x >= -radius; x--)
		for(int y = radius; y >= -radius; y--)
			for(int z = radius; z >= -radius; z--){
				/*
					We are comparing radius squared to the distance with no sqrt because
					square rooting is more expensive than multiplying
				*/
				// Variable storing the current position of the point (compensating for the origin)
				Vector3 search = Vector3(x, y, z) + origin + translation;
				Chunk* chunk = getChunk(search);
				// Ignore points not inside sphere
				if ((radius * radius) >= search.distance_squared_to(origin + translation)){
					// See if the point is already in the sphere...
					if(chunk != nullptr)
						// If it is in the sphere add the point from the sphere
						tempSphere.push_back(chunk);
					else
						// Otherwise load the the chunk
						tempSphere.push_back( generateChunk(search, regen) );
				// If the point isn't in the sphere (and exists), free it's memory
				} else if (chunk != nullptr)
					delete chunk;
			}
	// Change the origin to reflect the new translation
	origin += translation;
	// Replace the current sphere with the newly generated one
	sphere.swap(tempSphere);
}

/*
FUNCTION:          getIndex(int x, int y)
DESCRIPTION:       Gets the number of elements which appear in the array before the given x, y point
RETURNS: 		   The element count
*/
int ChunkMap::getIndex(int x, int y){
	// If the point is in the index, return the count
	if(index.find(x) != index.end())
		if(index[x].find(y) != index[x].end())
			return index[x][y];
	// Otherwise, return null
	return NULL_INDEX;
}

/*
FUNCTION:          getPointIndex(Vector3 search)
DESCRIPTION:       Gets a element stored at the provided search point
RETURNS: 		   The element's index in the array
*/
int ChunkMap::getChunkIndex(Vector3 search){
	/*
		This algorithm narrows its results to a single z-axis column (from the index)
		and then remaps the z-axis point to the [0, axis.size()] range
	*/
	int max = getIndex(-(search.x - origin.x), -(search.y - origin.y)), // Element index of the end of search range
		min = getIndex(-(search.x - origin.x), -(search.y - origin.y)-1), // Element index of the beginning of search range
		range = (max - min - 1) / 2 + 1;

	// Return NULL if the search is out of array bounds
	if(min < 0) return NULL_INDEX;
	// Return NULL if the search is out of slice bounds
	if(int(-(search.z - origin.z)) <= -range || int(search.z - origin.z) >= range) return NULL_INDEX;
	// Since we know the index is in the array, get it
	return max - (int(search.z - origin.z) + range);
}

/*
FUNCTION:          getChunk(const Vector3& search)
DESCRIPTION:       Gets a element stored at the provided search point
RETURNS: 		   The element
*/
Chunk* ChunkMap::getChunk(Vector3 search){
	// Get the index from the array
	int index = getChunkIndex(search);
	// If the index value is in the array return the point stored there
	if(index != NULL_INDEX)
		return sphere[index];
	// Otherwise, return null
	return nullptr;
}

/*
FUNCTION:          getChunkCenter(const Vector3& search)
DESCRIPTION:       Gets the center of the chunk stored at the provided search point
RETURNS: 		   The the Chunk's center if it exists or NULL_VECTOR otherwise
*/
Vector3 ChunkMap::getChunkCenter(Vector3 search, bool worldSpace){
	Chunk* chunk = getChunk(search);
	if(chunk != nullptr)
		if(worldSpace){
			return chunk->getCenter();
		} else {
			return chunk->getCenter() / 32;
		}
	// Otherwise, return null
	return NULL_VECTOR;
}

/*
FUNCTION:          block2chunk(Vector3 in)
DESCRIPTION:       Converts a search from the [-15, 15] + k block space to the 0 + k chunk space
RETURNS: 		   The new search in chunk space
*/
Vector3 ChunkMap::block2chunk(Vector3 in){
	//[-15, 15) = 0
    //[15, 45) = 1
    //[45, 75) = 2
    //[75, 105) = 3
    //[105, 135) = 4
	in = Vector3( (in.x < 0 ? in.x + 1 : in.x -1),
			(in.y < 0 ? in.y + 1 : in.y -1),
			(in.z < 0 ? in.z + 1 : in.z -1) );
	return integize( Vector3((in.x - 15) / 30 + (in.x > 0 ? 1 : 0),
                (in.y - 15) / 30 + (in.y > 0 ? 1 : 0),
                (in.z - 15) / 30 + (in.z > 0 ? 1 : 0)) );
}

/*
FUNCTION:          getBlock(Vector3 search)
DESCRIPTION:       Extracts a block from the chunkMap
*/
Block* ChunkMap::getBlock(Vector3 search, bool fuzzy){
	Vector3 cSearch = block2chunk(search);
	Chunk* chunk = getChunk(cSearch);
	/*if(chunk == nullptr)
		Godotize::print(to_string(search) + " = " + to_string(cSearch));*/
	if(chunk)
		return chunk->getBlock(search, fuzzy);
	return nullptr;
}

/*
FUNCTION:          getSubChunk2(Vector3 search)
DESCRIPTION:       Extracts a SubChunk2 from the chunkMap
*/
SubChunk2* ChunkMap::getSubChunk2(Vector3 search, bool fuzzy){
	Chunk* chunk = getChunk(block2chunk(search));
	if(chunk)
		return chunk->getSubChunk2(search, fuzzy);
	return nullptr;
}

/*
FUNCTION:          getSubChunk4(Vector3 search)
DESCRIPTION:       Extracts a SubChunk4 from the chunkMap
*/
SubChunk4* ChunkMap::getSubChunk4(Vector3 search, bool fuzzy){
	Chunk* chunk = getChunk(block2chunk(search));
	if(chunk)
		return chunk->getSubChunk4(search, fuzzy);
	return nullptr;
}

/*
FUNCTION:          getSubChunk8(Vector3 search)
DESCRIPTION:       Extracts a SubChunk8 from the chunkMap
*/
SubChunk8* ChunkMap::getSubChunk8(Vector3 search, bool fuzzy){
	Chunk* chunk = getChunk(block2chunk(search));
	if(chunk)
		return chunk->getSubChunk8(search, fuzzy);
	return nullptr;
}

/*
FUNCTION:          getCube(Vector3 origin, int radius)
DESCRIPTION:       Gets a cube of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the cube
*/
std::vector<Chunk*> ChunkMap::getCube(int radius, Vector3 origin){
	std::vector<Chunk*> out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x > -radius; x--)
		for(int y = radius; y > -radius; y--)
			for(int z = radius; z > -radius; z--){
				// Get the point from the main array (compensating for origin)
				Chunk* chunk = getChunk(Vector3(x, y, z) + origin);
				// If the point exists add it to the output array
				if(chunk != nullptr)
					out.push_back(chunk);
			}
	return out;
}

/*
FUNCTION:          getCubeRim(Vector3 origin, int radius)
DESCRIPTION:       Gets a the outer rim of a cube of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the rim
*/
std::vector<Chunk*> ChunkMap::getCubeRim(int radius, Vector3 origin){
	std::vector<Chunk*> out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x > -radius; x--)
		for(int y = radius; y > -radius; y--)
			for(int z = radius; z > -radius; z--)
				// Remove everything not on the rim
				if(x == radius || x == -radius + 1 || y == radius || y == -radius + 1 || z == radius || z == -radius + 1) {
					// Get the point from the main array (compensating for origin)
					Chunk* chunk = getChunk(Vector3(x, y, z) + origin);
					// If the point exists add it to the output array
					if(chunk != nullptr)
						out.push_back(chunk);
				}
	return out;
}

/*
FUNCTION:          getSphere(Vector3 origin, int radius)
DESCRIPTION:       Gets a sphere of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the sphere
*/
std::vector<Chunk*> ChunkMap::getSphere(int radius, Vector3 origin){
	std::vector<Chunk*> out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x >= -radius; x--)
		for(int y = radius; y >= -radius; y--)
			for(int z = radius; z >= -radius; z--){
				// Get the point from the main array (compensating for origin)
				Chunk* chunk = getChunk(Vector3(x, y, z) + origin);
				// If the point exists...
				if(chunk != nullptr)
					// And is inside the sphere, add it to the array
					if ((radius * radius) >= getChunkCenter(Vector3(x, y, z) + origin).distance_squared_to(origin))
							out.push_back(chunk);
			}
	return out;
}

/*
FUNCTION:          getSphereRim(Vector3 origin, int radius)
DESCRIPTION:       Gets the outer rim of a sphere of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the rim
*/
std::vector<Chunk*> ChunkMap::getSphereRim(int radius, Vector3 origin){
	std::vector<Chunk*> out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x >= -radius; x--)
		for(int y = radius; y >= -radius; y--)
			for(int z = radius; z >= -radius; z--){
				// Get the point from the main array (compensating for origin)
				Chunk* chunk = getChunk(Vector3(x, y, z) + origin);
				// If the point exists...
				if(chunk != nullptr){
					Vector3 point = getChunkCenter(Vector3(x, y, z) + origin);
					// And is inside the sphere
					if ((radius * radius) >= point.distance_squared_to(origin))
						// But is not inside a sphere of radius <radius> - 1
						if(((radius - 1) * (radius - 1)) < point.distance_squared_to(origin))
							// Add it to the array
							out.push_back(chunk);
				}
			}
	return out;
}

/*
FUNCTION:          initSphere()
DESCRIPTION:       Generates an array representing a sphere of integer points with radius <radius
NOTES:			   All points are integer offsets from the origin
					Array is sorted by x's, then y's, then z's
					Sphere begins at top-left and goes to bottom-right
*/
void ChunkMap::initSphere(){
	// Clear sphere (reallocating)
	{ std::vector<Chunk*>().swap(sphere); }
	/*
		Considered pre-reserving the required space but... Any benefit is well
		within the bounds of experimentation error
	*/
	// Generate cube
	for(int x = radius; x >= -radius; x--){
		for(int y = radius; y >= -radius; y--){
			int total = 0; // Variable storing the z value for the x,y pair
			for(int z = radius; z >= -radius; z--){
				/*
					We are comparing radius squared to the distance with no sqrt because
					square rooting is more expensive than multiplying
				*/
				// Variable storing the current position of the point (compensating for the origin)
				Vector3 temp = Vector3(x, y, z) + origin;
				// Ignore points not inside sphere
				if ((radius * radius) >= temp.distance_squared_to(origin)){
					// Add the current point to the output array
					sphere.push_back( generateChunk(temp, regen) );
					// For every z we add to the array, increment our total
					total++;
				}
			}
			/*
				This moving this calculation allows initIndex to not have to loop through the entire
				array looking for elements. Instead it happens inside of loops which would have occurred either way
			*/
			// If we added anything to the array for this pair, set the index to the count
			if (total > 0){
				index[x][y] = total;
			}
		}
	}
}

/*
FUNCTION:          initIndex()
DESCRIPTION:       Indexes the array
*/
void ChunkMap::initIndex(){
	// Loop through the index, ensuring that an index value for the given x,y pair has been initialized
	for(int x = -radius; x <= radius; x++)
		if(index.find(x) != index.end()){
			bool first = true; // Variable storing if this is the first element of the x (row)
			for(int y = -radius; y <= radius; y++)
				if (index[x].find(y) != index[x].end()){
					// If this is the first element of the row...
					if(first){
						// And this isn't the first row
						if(x != -radius)
							// Wrap back to the last value of the previous row
							index[x][y] += index[x-1][(index[x-1].size()-1) / 2];
						first = false;
					// Otherwise just add the previous column
					} else
						index[x][y] += index[x][y-1];
				}
		}
}
