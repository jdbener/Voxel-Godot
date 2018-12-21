/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  0.0 - Initial testing version
*/

#include <Godot.hpp>
#include <Spatial.hpp>
#include <Array.hpp>

#include "world/Chunk.hpp"

using namespace godot;

class ChunkRenderer: public Spatial {
    GODOT_CLASS(ChunkRenderer, Spatial);
public:
    Chunk chunk;

    void _init() {}
	static void _register_methods();

    void _enter_tree();
    void _ready();
};
