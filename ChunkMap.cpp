/*
FILE:              ChunkMap.cpp
DESCRIPTION:
NOTES:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-09         0.0 - Implemented sphere generation, indexing, and access
Joshua Dahl		   2018-12-10		  0.1 - Implemented subcube/sphere access
Joshua Dahl		   2018-12-11		  0.2 - Rewritten initialization code to support origins other than (0, 0, 0)
										Additionally the sphere can now be reinitialized with a new origin,
										all of the points shared between the old and new spheres are transfered
										as opposed to being recreated.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <chrono>

#include "Vector3.hpp"

using namespace std;

// Macro which determines the execution time of a function call
#define time(x) {\
	auto start = std::chrono::high_resolution_clock::now();\
	x;\
	auto stop = std::chrono::high_resolution_clock::now();\
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);\
	cout << " Time: " << duration.count() << " microseconds" << endl;\
}

// Constant representing a number getIndex returns when the searched for index doesn't exist
const int NULL_INDEX = -999;

struct ChunkMap{
	int radius;	// Variable storing the radius of the sphere
	Vector3 origin; // Variable storing the origin of the sphere (defaults to (0, 0, 0))
	vector<Vector3> sphere; // Array storing the elements which make up the sphere
	map<int, map<int, int>> index; // Stores the number of elements at the end of each x/y location

	/*
	FUNCTION:          CONSTRUCTOR(Vector3 origin, int radius)
	DESCRIPTION:       Creates the sphere (centered at <origin> of radius <radius>)
						and indexes the elements
	*/
	ChunkMap(Vector3 _origin, int _radius) : radius(_radius), origin(_origin) {
		initSphere();
		time(initIndex());
		cout << endl;
	}

	ChunkMap(int _radius) : radius(_radius), origin(Vector3()) {
		initSphere();
		time(initIndex());
		cout << endl;
	}

	/*
	FUNCTION:          DESTRUCTOR()
	DESCRIPTION:       Cleans up the array and index
	*/
	~ChunkMap(){
		sphere.clear();
		for(int i = -radius; i <= radius; i++)
			index[i].clear();
		index.clear();
	}

	/*
	FUNCTION:          reinitSphere(Vector3 translation)
	DESCRIPTION:       Regenerates the array, changing the <origin> to reflect the
	 					provided <translation>, resuing as many points as possible
	*/
	void reinitSphere(Vector3 translation){
		// Create (and reserve) a new temporary array to hold the new sphere
		vector<Vector3> tempSphere;
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
					if ((radius * radius) >= dist(search, origin + translation)){
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

	inline int getIndex(int x, int y, int print = false){
		if (print)
			cout << endl << Vector2(x, y);
		if(index.find(x) != index.end())
			if(index[x].find(y) != index[x].end())
				return index[x][y];
		return NULL_INDEX;
	}

	/*
	FUNCTION:          getPoint(const Vector3& search)
	DESCRIPTION:       Gets a element stored at the provided search point
	RETURNS: 		   The element
	*/
	inline Vector3 getPoint(Vector3 search){
		/*
			This algorithm narrows its results to a single z-axis column (from the index)
			and then looks for the point.
			At worst it will search through 2r + 1 elements
		*/
		int max = getIndex(-(search.x - origin.x), -(search.y - origin.y)), // Element index of the end of search range
			min = getIndex(-(search.x - origin.x), -(search.y - origin.y)-1); // Element index of the beginning of search range

		/*
			TODO: binary search
		*/
		// Linearly search through array
		if(min >= 0)
			for(int i = min; i < max; i++)
				if(sphere[i] == search)
					return sphere[i];
		// Return NULL if we didn't find the search
		return NULL_VECTOR;
	}

	/*
	FUNCTION:          getCube(int radius)
	DESCRIPTION:       Gets a cube of elements centered at the stored <origin> of radius <radius>
	RETURNS: 		   An array containing the cube
	*/
	inline vector<Vector3> getCube(int radius){
		return getCube(origin, radius);
	}

	/*
	FUNCTION:          getCube(Vector3 origin, int radius)
	DESCRIPTION:       Gets a cube of elements centered at <origin> of radius <radius>
	RETURNS: 		   An array containing the cube
	*/
	vector<Vector3> getCube(Vector3 origin, int radius){
		vector<Vector3> out; // Variable storing the output array
		// Generate a box of radius <radius>
		for(int x = radius; x >= -radius; x--)
			for(int y = radius; y >= -radius; y--)
				for(int z = radius; z >= -radius; z--){
					// Get the point from the main array (compensating for origin)
					Vector3 point = getPoint(Vector3(x, y, z) + origin);
					// If the point exists add it to the output array
					if(point != NULL_VECTOR)
						out.push_back(point);
				}
		return out;
	}

	/*
	FUNCTION:          getSphere(int radius)
	DESCRIPTION:       Gets a sphere of elements centered at the stored <origin> of radius <radius>
	RETURNS: 		   An array containing the sphere
	*/
	vector<Vector3> getSphere(int radius){
		return getSphere(Vector3(), radius);
	}

	/*
	FUNCTION:          getSphere(Vector3 origin, int radius)
	DESCRIPTION:       Gets a sphere of elements centered at <origin> of radius <radius>
	RETURNS: 		   An array containing the sphere
	*/
	vector<Vector3> getSphere(Vector3 origin, int radius){
		vector<Vector3> out; // Variable storing the output array
		// Generate a box of radius <radius>
		for(int x = -radius; x <= radius; x++)
			for(int y = -radius; y <= radius; y++)
				for(int z = -radius; z <= radius; z++){
					// Get the point from the main array (compensating for origin)
					Vector3 point = getPoint(Vector3(x, y, z) + origin);
					// If the point exists...
					if(point != NULL_VECTOR)
						// And is inside the sphere, add it to the array
						if ((radius * radius) >= dist(point, origin))
							out.push_back(point);
				}
		return out;
	}

	/*
	FUNCTION:          dist(Vector3 a, Vector3 b)
	DESCRIPTION:       Calculates the distance between two points
	RETURNS:           The distance
	NOTES:			   Calculation ignores square root step (comparison needs to be squared)
	 */
	inline int dist(Vector3 a, Vector3 b){
		return abs((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
	}

private:
	/*
	FUNCTION:          initSphere()
	DESCRIPTION:       Generates an array representing a sphere of integer points with radius <radius
	NOTES:			   All points are integer offsets from the origin
						Array is sorted by x's, then y's, then z's
						Sphere begins at top-left and goes to bottom-right
	*/
	void initSphere(){
		// Clear sphere (reallocating)
		{ vector<Vector3>().swap(sphere); }
		/*
			Considered pre-reserving the required space but... Any benefit is well
			within the bounds of experimentation error
		*/
		// Generate cube
		for(int x = radius; x >= -radius; x--){
			for(int y = radius; y >= -radius; y--){
				for(int z = radius; z >= -radius; z--){
					/*
						We are comparing radius squared to the distance with no sqrt because
						square rooting is more expensive than multiplying
					*/
					// Variable storing the current position of the point (compensating for the origin)
					Vector3 temp = Vector3(x, y, z) + origin;
					// Ignore points not inside sphere
					if ((radius * radius) >= dist(temp, origin))
						// Add the current point to the output array
						sphere.push_back(temp);
					// We are decrementing from the current position because
					// we are working from the top-left to the bottom-right
				}
			}
		}
	}

	/*
	FUNCTION:          initIndex()
	DESCRIPTION:       Indexes the array
	*/
	void initIndex(){
		cout << "Indexing x="; // Print loading message
		// Loop through x and y axises...
		for(int x = -radius, total = 0; x <= radius; x++){
			int x1 = x + origin.x;
			for(int y = -radius; y <= radius; y++){
				int y1 = y + origin.y;
				// If the point is within the sphere...
				if(dist(Vector3(x, y, 0), Vector3()) <= (radius * radius)){
					// Store a count of the total elements so far
					for(Vector3 cur: sphere)
						if(cur.y == y1 && cur.x == x1)
							total++;
					index[x][y] = total;
				}
			}
			// Print loading message
			cout << "..." << x; if(!(x % 10)) cout << flush;
		}
	}
};

// Helper Prototypes
string centerText(string input, int width);
void visualizeSphere(ChunkMap sphere, ostream& out = cout);
int binarySearch(vector<Vector3> arr, int l, int r, Vector3 x);

/*
FUNCTION:          main()
DESCRIPTION:       .
RETURNS:           0
NOTES:
 */
int main()
{
	const int r = 4 * 1;
	ChunkMap sphere(Vector3(1, 1, 1), r);
	time(sphere.reinitSphere(Vector3(1, -1, 1)));

	visualizeSphere(sphere);

	time(cout << sphere.getPoint(Vector3(0, 0, 0)))

	cout << "Thickest point: " << sphere.index[0][0] - sphere.index[0][-1] << endl;


	/*vector<Vector3> alwaysLoad;
	time(alwaysLoad = sphere.getCube(Vector3(-2, -1, 0), 1));
	int i = 0;
	for(Vector3 cur: alwaysLoad){
		cout << cur << '\t';
		if (++i > 2){
			cout << endl;
			i = 0;
		}
	}*/
	/* int i = 0;
	for(Vector3 cur: sphere.sphere){
		cout << cur << "\t";
		if(i++ > 5){
			cout << endl;
			i = 0;
		}
	}
	*/
	const int pt = 121;
	cout << "Dist to " << sphere.sphere[pt] << " is " << sqrt(sphere.dist(sphere.sphere[pt], Vector3())) << endl;
	cout << "Total chunks: " << sphere.sphere.size() << " Total blocks: " << sphere.sphere.size() * 16 * 16 << endl;
	cout << "Vec3: " << Vector3() << " Vec3.xy: " << Vector3().xy() << endl;

	system("pause");

	return 0;
}

/*
FUNCTION:          visualizeSphere(ChunkMap sphere, ostream& out)
DESCRIPTION:       Prints out a 2D representation of the sphere
NOTES:			   The numbers represent the z-axis
*/
void visualizeSphere(ChunkMap sphere, ostream& out){
	// Loop through x and y of index, checking if the values exist
	for(int x = -sphere.radius; x <= sphere.radius; x++){
		bool first = true;
		for(int y = -sphere.radius; y <= sphere.radius; y++){
			int index = sphere.getIndex(x, y);
			if(index != NULL_INDEX){
				if(first){
					if(x != -sphere.radius)
						cout << index - sphere.getIndex(x - 1, (sphere.index[x - 1].size() - 1) / 2) << " ";
					else
						cout << index << " ";
				} else
					cout << index - sphere.getIndex(x, y - 1) << " ";
				first = false;
				//cout << index << " ";
			} else
				cout << "  ";
		}
		cout << endl;
	}
}
