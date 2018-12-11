/*

FILE:              sphere.cpp
DESCRIPTION:
NOTES:

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-09         0.0 -
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

struct ChunkMap{
	int radius;	// Variable storing the radius of the sphere
	vector<Vector3> sphere; // Array storing the elements which make up the sphere
	map<int, map<int, int>> index; // Index of element count of each point on an x-y slice through the sphere

	/*
	FUNCTION:          CONSTRUCTOR(Vector3 origin, int radius)
	DESCRIPTION:       Creates the sphere (centered at <origin> of radius <radius>)
						and indexes the elements
	*/
	ChunkMap(Vector3 origin, int _radius) : radius(_radius) {
		initSphere(origin);
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
	FUNCTION:          initSphere(Vector3 origin)
	DESCRIPTION:       Generates an array representing a sphere of integer points with radius <radius
	NOTES:			   All points are integer offsets from the origin
						Array is sorted by x's, then y's, then z's
	*/
	void initSphere(Vector3 origin){
		// Clear sphere (reallocating)
		{ vector<Vector3>().swap(sphere); }
		/*
			Considered pre-reserving the required space but... Any benefit is well
			within the bounds of experimentation error
		*/
		// Variable holding the current point to be added (begins at the top-left corner)
		Vector3 temp = Vector3(origin.x + radius, origin.y + radius, origin.z + radius);

		// Generate cube
		for(int x = 0; x <= radius * 2; x++){
			for(int y = 0; y <= radius * 2; y++){
				for(int z = 0; z <= radius * 2; z++){
					/*
						We are comparing radius squared to the distance with no sqrt because
						square rooting is more expensive than multiplying
					*/
					// Ignore points not inside sphere
					if ((radius * radius) >= dist(temp, origin))
						// Add the current point to the output array
						sphere.push_back(temp);
					// We are decrementing from the current position because
					// we are working from the top-left to the bottom-right
					temp.z--;
				}
				// (For every line of y's) Reset z-axis and decrement y
				temp.z = origin.z + radius;
				temp.y--;
			}
			// (For every line of x's) Reset y and z axis and decrement x
			temp.y = origin.y + radius;
			temp.z = origin.z + radius;
			temp.x--;
		}
	}

	/*
	FUNCTION:          initIndex()
	DESCRIPTION:       Indexes the array
	*/
	void initIndex(){
		cout << "Caching"; // Print loading message
		// Loop through x and y axises...
		for(int x = -radius, total = 0; x <= radius; x++){
			for(int y = -radius; y <= radius; y++)
				// If the point is within the sphere...
				if(dist(Vector3(x, y, 0), Vector3()) <= (radius * radius)){
					// Store a count of the total elements so far
					for(Vector3 cur: sphere)
						if(cur.y == y && cur.x == x)
							total++;
					index[x][y] = total;
				}
			// Print loading message
			cout << "..." << x; if(!(x % 10)) cout << flush;
		}
	}

	/*
	FUNCTION:          getPoint(const Vector3& search)
	DESCRIPTION:       Gets a element stored at the provided search point
	RETURNS: 		   The element
	*/
	inline Vector3 getPoint(const Vector3 search){
		/*
			This algorithm narrows its results to a single z-axis column (from the index)
			and then looks for the point.
			At worst it will search through 2r + 1 elements
		*/
		int max = index[-int(search.x)][-int(search.y)], // Element index of the end of search range
			min = index[-int(search.x)][-int(search.y)-1]; // Element index of the beginning of search range

		/*
			TODO: binary search
		*/
		// Linearly search through array
		for(int i = min; i < max; i++)
			if(sphere[i] == search)
				return sphere[i];
		// Return NULL if we didn't find the search
		return NULL_VECTOR;
	}

	/*
	FUNCTION:          getCube(int radius)
	DESCRIPTION:       Gets a cube of elements centered at (0, 0, 0) of radius <radius>
	RETURNS: 		   An array containing the cube
	*/
	inline vector<Vector3> getCube(int radius){
		return getCube(Vector3(), radius);
	}

	/*
	FUNCTION:          getCube(Vector3 origin, int radius)
	DESCRIPTION:       Gets a cube of elements centered at <origin> of radius <radius>
	RETURNS: 		   An array containing the cube
	*/
	vector<Vector3> getCube(Vector3 origin, int radius){
		vector<Vector3> out; // Variable storing the output array
		// Generate a box of radius <radius>
		for(int x = -radius; x <= radius; x++)
			for(int y = -radius; y <= radius; y++)
				for(int z = -radius; z <= radius; z++){
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
	DESCRIPTION:       Gets a sphere of elements centered at (0, 0, 0) of radius <radius>
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
	Vector3 origin;
	const int r = 4 * 1;
	ChunkMap sphere(origin, r);

	visualizeSphere(sphere);

	time(cout << sphere.getPoint(Vector3(1, 3, 6)))


	/*vector<Vector3> alwaysLoad;
	time(alwaysLoad = sphere.getSphere(1));
	int i = 0;
	for(Vector3 cur: alwaysLoad){
		cout << cur << '\t';
		if (++i > 2){
			cout << endl;
			i = 0;
		}
	}*/

	/*
	int i = 0;
	for(Vector3 cur: sphere.sphere){
		cout << cur << "\t";
		if(i++ > 5){
			cout << endl;
			i = 0;
		}
	}*/
	const int pt = 121;
	cout << "Dist to " << sphere.sphere[pt] << " is " << sqrt(sphere.dist(sphere.sphere[pt], origin)) << endl;
	cout << "Total chunks: " << sphere.sphere.size() << " Total blocks: " << sphere.sphere.size() * 16 * 16 << endl;
	cout << "Vec3: " << origin << " Vec3.xy: " << origin.xy() << endl;

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
	for(int x = -sphere.radius; x <= sphere.radius; x++)
		if (sphere.index.find(x) != sphere.index.end()) {
			bool first = true; // Variable storing if this is the first number of a line
			for(int y = -sphere.radius; y <= sphere.radius; y++)
				if (sphere.index[x].find(y) != sphere.index[x].end()){
					// If this is the first number of a row
					if(first)
						// Calculate the delta elements based on the end of the previous row
						out << (sphere.index[x][y] - sphere.index[x-1][(sphere.index[x-1].size()-1)/2]) << " ";
					else
						// Otherwise calculate the delta elements from the previous y-axis value
						out << (sphere.index[x][y] - sphere.index[x][y-1]) << " ";
					// Just display the total row count
					//out << (sphere.index[x][y]) << " ";
					first = false;
				// If the value doesn't exist in the array, just print some horizontal space
				} else
					out << "  ";
			// For every x-row print a newline
			out << endl;
		}
}

/*
FUNCTION:          centerText(input, width)
DESCRIPTION:       Takes an input string and centers it based on the input width
RETURNS:           The string with added spacing
*/
string centerText(string input, int width) {
	// Calculates the middle of the desired width
	// and subtracts half of the length of the input string from it
	int middle = width / 2;
	middle -= input.length() / 2;

	string out = ""; // Initialize a blank output
	// Add the desired numbers of spaces to the output
	for (int i = 0; i < middle; i++)
		out += ' ';
	// Add the input string to the output
	out += input;

	return out; // Return the string with spaces added
}
