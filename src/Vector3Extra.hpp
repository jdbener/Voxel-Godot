/*
FILE:              Vector3Extra.hpp
DESCRIPTION:       A couple of extensions to Godot's Vector3 class

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  1.0 - Implemented >, single float constructing, and <<
Joshua Dahl		   2018-12-24		  1.1 - Implemented integize
Joshua Dahl		   2018-12-25		  1.2 - Implemented to_string
*/

#ifndef VECTOR3_E_H
#define VECTOR3_E_H

#include <ostream>
#include <string>

static bool operator> (Vector3 a, Vector3 b){
    if(a.x <= b.x) return false;
    if(a.y <= b.y) return false;
    if(a.z <= b.z) return false;
    return true;
}

/*
NAME:           expand(float what)
DESCRIPTION:    Expands the provided float into a Vector3
*/
static Vector3 expand(float what){
    return Vector3(what, what, what);
}

/*
NAME:           integize(Vector3 in)
DESCRIPTION:    Drops the decimal information from the provided array
*/
static Vector3 integize(Vector3 in){
    return Vector3(int(in.x), int(in.y), int(in.z));
}

static std::string to_string(Vector3 in){
    return "{" + std::to_string(in.x) + ", " + std::to_string(in.y) + ", " + std::to_string(in.z) + "}";
}

static std::string to_string(Vector3 in, bool integize){
    return "{" + std::to_string((int) in.x) + ", " + std::to_string((int) in.y) + ", " + std::to_string((int) in.z) + "}";
}

static std::ostream& operator<<(std::ostream & stream, Vector3 vec){
    stream << to_string(vec);
    return stream;
}

#endif
