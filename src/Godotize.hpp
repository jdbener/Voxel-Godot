/*
FILE:              Godotize.hpp
DESCRIPTION:	   Provides a small helper library which converts STD types to Godot types

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-19         1.0 - vector->Array and string->String
Joshua Dahl        2018-12-19         1.1 - Tested/fixed string implementation
*/
#ifndef GODOTIZE_H
#define GODOTIZE_H

#include <Array.hpp> // Godot Array
#include <vector> // STD vector
#include <String.hpp> // Godot string
#include <string> // STD string
#include <iostream>

/*
NAME:           godotize(std::vector<T> array)
DESCRIPTION:    Converts a vector into a Godot Array
*/
template <typename T>
static godot::Array godotize(std::vector<T> array){
    godot::Array out;
	for(T cur: array){
        out.push_back(cur);
	}
    return out;
}

/*
NAME:           godotize(std::wstring in)
DESCRIPTION:    Converts a wide string into a Godot String
*/
static inline godot::String godotize(std::wstring in){
    return godot::String(in.c_str());
}

/*
NAME:           godotize(std::string in)
DESCRIPTION:    Converts a string into a Godot String
*/
static inline godot::String godotize(std::string in){
    return godotize(std::wstring(in.begin(), in.end()));
}

/*
NAME:           godotize(const char* in)
DESCRIPTION:    Converts a c-string into a Godot String
*/
static inline godot::String godotize(const char* in){
    return godotize(std::string(in));
}

#endif
