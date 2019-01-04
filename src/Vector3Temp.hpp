/*
FILE:               Vector3Temp.hpp
DESCRIPTION:        Implements a type which approximates Godot's Vector3 without
					having to link the test programs with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Implementation
*/

#ifndef VECTOR3_H
#define VECTOR3_H

//namespace debug {

struct Vector3 {

	enum Axis {
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
	};

	union {
		struct {
			float x;
			float y;
			float z;
		};

		float coord[3];
	};

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { };

	Vector3() : x(0), y(0), z(0) { }

	Vector3 operator*(Vector3 in){
		return Vector3(x * in.x, y * in.y, z * in.z);
	}
	Vector3 operator+(Vector3 in){
		return Vector3(x + in.x, y + in.y, z + in.z);
	}
};

//} // debug


#endif
