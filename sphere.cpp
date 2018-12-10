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
	int radius;
	vector<Vector3> sphere;
	map<int, map<int, int>> count;
	ChunkMap(Vector3 origin, int _radius) : radius(_radius) {
		initSphere(origin);
		time(initCount());
	}
	~ChunkMap(){
		sphere.clear();
		for(int i = -radius; i <= radius; i++)
			count[i].clear();
		count.clear();
	}

	Vector3 getPoint(const Vector3& search){
		int max = count[-int(search.x)][-int(search.y)], min = count[-int(search.x)][-int(search.y)-1];

		for(int i = min; i < max; i++){
			auto equals = [](Vector3 a, Vector3 b){
				if(int(a.x) != int(b.x)) return false;
				if(int(a.y) != int(b.y)) return false;
				if(int(a.z) != int(b.z)) return false;
				return true;
			};

			if(equals(sphere[i], search))
				return sphere[i];
		}
		return Vector3(-99, -99, -99);
	}
	/*
	FUNCTION:          getSphere(Vector3 origin)
	DESCRIPTION:       Generates an array representing a sphere of integer points with radius <radius
	NOTES:			   All points are integer offsets from the origin
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
						We are comparing radius sqaured to the distance with no sqrt because
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

	void initCount(){
		cout << "Caching";
		// Loop through x axis
		for(int x = -radius, total = 0; x <= radius; x++){
			// Loop through y axis
			for(int y = -radius; y <= radius; y++)
				// If the point is within the sphere
				if(dist(Vector3(x, y, 0), Vector3()) <= (radius * radius)){
					// Store a count of the total elements so fars
					for(Vector3 cur: sphere)
						if(cur.y == y && cur.x == x)
							total++;
					count[x][y] = total;
				}
			cout << "..." << x; if(!(x % 10)) cout << flush;
		}
		cout << endl;
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
	const int r = 4 * 5;
	ChunkMap sphere(origin, r);

	visualizeSphere(sphere);

	time(cout << sphere.getPoint(Vector3(0, 0, 0)))



	/*int i = 0;
	for(Vector3 cur: sphere){
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

void visualizeSphere(ChunkMap sphere, ostream& out){
	for(int x = -sphere.radius; x <= sphere.radius; x++)
		if (sphere.count.find(x) != sphere.count.end()) {
			bool first = true;
			for(int y = -sphere.radius; y <= sphere.radius; y++)
				if (sphere.count[x].find(y) != sphere.count[x].end()){
					if(first)
						out << (sphere.count[x][y] - sphere.count[x-1][(sphere.count[x-1].size()-1)/2]) << " ";
					else
						out << (sphere.count[x][y] - sphere.count[x][y-1]) << " ";
					first = false;
				} else
					out << "  ";
			out << endl;
		}
}

/*
FUNCTION:          centerText(input, width)
DESCRIPTION:       Takes an input string and centers it based on the input width
RETURNS:           The string with added spacing
NOTES:
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
