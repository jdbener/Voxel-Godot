/*
FILE:              Godotize.hpp
DESCRIPTION:	   Provides a small helper library which converts STD types to Godot types

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-19         1.0 - vector->Array and string->String
*/
#ifndef GODOTIZE_H
#define GODOTIZE_H

#include <Array.hpp> // Godot Array
#include <vector> // STD vector
#include <String.hpp> // Godot string
#include <string> // STD string

template <typename T>
godot::Array godotize(std::vector<T> array){
    godot::Array out;
	for(T cur: array){
        out.push_back(cur);
	}
    return out;
}

godot::String godotize(std::string in){
    return godot::String(in.c_str());
}

#endif
