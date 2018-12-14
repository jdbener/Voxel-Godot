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
*/
#include "ChunkMap.hpp"

/*
FUNCTION:          _init()
DESCRIPTION:       Used to sync properties from the engine to the code
*/
void ChunkMap::_init() {
	radius = 4 * 2;
	origin = Vector3(0, 0, 0);
}

/*
FUNCTION:          _register_methods()
DESCRIPTION:       Used to tell the engine what functions this script is providing
*/
void ChunkMap::_register_methods(){
	// Export <radius> and <origin>
    register_property<ChunkMap, int>("radius", &ChunkMap::radius, 4 * 2);
    register_property<ChunkMap, Vector3>("origin", &ChunkMap::origin, Vector3(0, 0, 0));

	// Con/destructor
    register_method("_enter_tree", &ChunkMap::_enter_tree);
    register_method("_exit_tree", &ChunkMap::_exit_tree);

	// Reinit
	register_method("reinitSphere", &ChunkMap::reinitSphere);

	// Index/Point Getter Functions
	register_method("getIndex", &ChunkMap::getIndex);
	register_method("getPoint", &ChunkMap::getPoint);

	// Subcube/Subsphere
	//register_method("getCube", &ChunkMap::getCubeInt);
	register_method("getCube", &ChunkMap::getCube);
	//register_method("getSphere", &ChunkMap::getSphereInt);
	register_method("getSphere", &ChunkMap::getSphere);
}

/*
FUNCTION:          CONSTRUCTOR()
DESCRIPTION:       Creates the sphere (centered at <origin> of radius <radius>)
					and indexes the elements
*/
void ChunkMap::_enter_tree() {
	initSphere();
	initIndex();
}

/*
FUNCTION:          DESTRUCTOR()
DESCRIPTION:       Cleans up the array and index
*/
void ChunkMap::_exit_tree() {
	sphere.clear();
	for(int i = -radius; i <= radius; i++)
		index[i].clear();
	index.clear();
}

/*
FUNCTION:          reinitSphere(Vector3 translation)
DESCRIPTION:       Regenerates the array, changing the <origin> to reflect the
 					provided <translation>, reusing as many points as possible
*/
void ChunkMap::reinitSphere(Vector3 translation){
	// Create (and reserve) a new temporary array to hold the new sphere
	std::vector<Vector3> tempSphere;
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
				// Ignore points not inside sphere
				if ((radius * radius) >= search.distance_squared_to(origin + translation)){
					// See if the point is already in the sphere...
					Vector3 temp = getPoint(search);
					if(temp != NULL_VECTOR)
						// If it is in the sphere add the point from the sphere
						tempSphere.push_back(temp);
					else
						// Otherwise load the sphere
						tempSphere.push_back(search);
					/*
						This block isn't very important yet, but it will be as soon as we are holding chunks
						instead of just vector3's. At that point we will have to read in/generate a chunk in this part
						which will make the memory re-use very important
					*/
				}
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
FUNCTION:          getPoint(const Vector3& search)
DESCRIPTION:       Gets a element stored at the provided search point
RETURNS: 		   The element
*/
Vector3 ChunkMap::getPoint(Vector3 search){
	/*
		This algorithm narrows its results to a single z-axis column (from the index)
		and then remaps the z-axis point to the [0, axis.size()] range
	*/
	int max = getIndex(-(search.x - origin.x), -(search.y - origin.y)), // Element index of the end of search range
		min = getIndex(-(search.x - origin.x), -(search.y - origin.y)-1), // Element index of the beginning of search range
		range = (max - min - 1) / 2 + 1;

	// Return NULL if the search is out of array bounds
	if(min < 0) return NULL_VECTOR;
	// Return NULL if the search is out of slice bounds
	if(int(-(search.z - origin.z)) < -range || int(search.z - origin.z) > range) return NULL_VECTOR;
	// Since we know the value is in the array, get it
	return sphere[max - (int(search.z - origin.z) + range)];
}


/*
FUNCTION:          getCube(Vector3 origin, int radius)
DESCRIPTION:       Gets a cube of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the cube
*/
Array ChunkMap::getCube(int radius, Vector3 origin){
	Array out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x > -radius; x--)
		for(int y = radius; y > -radius; y--)
			for(int z = radius; z > -radius; z--){
				// Get the point from the main array (compensating for origin)
				Vector3 point = getPoint(Vector3(x, y, z) + origin);
				// If the point exists add it to the output array
				if(point != NULL_VECTOR)
					out.push_back(point);
			}
	return out;
}

/*
FUNCTION:          getSphere(Vector3 origin, int radius)
DESCRIPTION:       Gets a sphere of elements centered at <origin> of radius <radius>
RETURNS: 		   An array containing the sphere
*/
Array ChunkMap::getSphere(int radius, Vector3 origin){
	Array out; // Variable storing the output array
	// Generate a box of radius <radius>
	for(int x = radius; x > -radius; x--)
		for(int y = radius; y > -radius; y--)
			for(int z = radius; z > -radius; z--){
				// Get the point from the main array (compensating for origin)
				Vector3 point = getPoint(Vector3(x, y, z) + origin);
				// If the point exists...
				if(point != NULL_VECTOR)
					// And is inside the sphere, add it to the array
					if ((radius * radius) >= point.distance_squared_to(origin))
						out.push_back(point);
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
	{ std::vector<Vector3>().swap(sphere); }
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
					sphere.push_back(temp);
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
