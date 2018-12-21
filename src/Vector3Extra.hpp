/*
FILE:              Vector3Extra.hpp
DESCRIPTION:       A couple of extensions to Godot's Vector3 class

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  1.0 - Implemented >, single float constructing, and <<
*/

#ifndef VECTOR3_E_H
#define VECTOR3_E_H

#include <ostream>

static bool operator> (Vector3 a, Vector3 b){
    if(a.x <= b.x) return false;
    if(a.y <= b.y) return false;
    if(a.z <= b.z) return false;
    return true;
}

static Vector3 expand(float what){
    return Vector3(what, what, what);
}

static std::ostream& operator<<(std::ostream & stream, Vector3 vec){
    stream << "{" << vec.x << ", " << vec.y << ", " << vec.z << "} ";
    return stream;
}

#endif
