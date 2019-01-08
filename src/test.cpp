#include <iostream>
#include <cmath>
#include <cstdlib>
#include "Vector3Temp.hpp"
#include "Vector3Extra.hpp"

#include "QuickSort.hpp"

using namespace std;

Vector3 operator/ (Vector3 vec, float scale){
    return Vector3(vec.x / scale, vec.y / scale, vec.z / scale);
}

Vector3 operator- (Vector3 a, Vector3 b){
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 operator- (Vector3 in){
    return Vector3(-in.x, -in.y, -in.z);
}

inline Vector3 block2chunk(Vector3 in){
    in = in - 1;
    return integize( Vector3((in.x - 15) / 30 + (in.x > 0 ? 1 : 0),
                (in.y - 15) / 30 + (in.y > 0 ? 1 : 0),
                (in.z - 15) / 30 + (in.z > 0 ? 1 : 0)) );
}

inline Vector3 pos2dir(Vector3 a, Vector3 b){
    Vector3 delta = b - a;
    float thetaX = acos(delta.x / sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z));
    float thetaY = acos(delta.y / sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z));
    float thetaZ = acos(delta.z / sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z));

    return Vector3(thetaX, thetaY, thetaZ);
}

inline Vector3 pos2dir(Vector3 vec){
    return pos2dir(Vector3(), vec);
}

inline Vector3 rad2deg(Vector3 in){
    const float ratio = 180 / M_PI;
    return Vector3(in.x * ratio, in.y * ratio, in.z * ratio);
}

inline Vector3 maxAngle(Vector3 in){
    Vector3 test = Vector3((in.x > M_PI ? M_PI - in.x  : 2 * M_PI - in.x),
            (in.y > M_PI ? M_PI - in.y  : 2 * M_PI - in.y),
            (in.z > M_PI ? M_PI - in.z  : 2 * M_PI - in.z));
    cout << test << endl;
    if(test.x > test.y && test.x > test.z) return Vector3((in.x > M_PI ? -1 : 1), 0, 0);
    if(test.y > test.x && test.y > test.z) return Vector3(0, (in.y > M_PI ? -1 : 1), 0);
    if(test.z > test.y && test.z > test.x) return Vector3(0, 0, (in.z > M_PI ? -1 : 1));
}

inline Vector3 dir2origin(Vector3 in){
    Vector3 test = abs(in);

    if(test.x > test.y && test.x > test.z) return Vector3((in.x > 0 ? -1 : 1), 0, 0);
    if(test.y > test.x && test.y > test.z) return Vector3(0, (in.y > 0 ? -1 : 1), 0);
    if(test.z > test.y && test.z > test.x) return Vector3(0, 0, (in.z > 0 ? -1 : 1));
    return Vector3();
}

int main(){
    Vector3 a = Vector3(-11, -43, -59), b = Vector3(-19, -17, 13);

    /*
        (-35, 49, -115) = (-1, 2, -4)
        (-11, -43, -59) = (0, -1, -2)
    */

    //[-15, 15] = 0
    //[15, 45] = 1
    //[45, 75] = 2
    //[75, 105] = 3
    //[105, 135] = 4


    Vector3 aChunk = block2chunk(a);
    Vector3 aDir = dir2origin(a);

    cout << a << " = " << aChunk << " rotation = " << -aDir << endl;

    vector<int> test = { 6, 20, 5, 9, 99, 23, 1, 99, 99, 100 };

    for(int cur: test){
        cout << cur << " ";
    }
    cout << endl << "Sorting..." << endl;

    quickSort(test);

    for(int cur: test){
        cout << cur << " ";
    }
    cout << endl;

}
