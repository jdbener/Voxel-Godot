/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
*/

#include <Godot.hpp>
#include <Spatial.hpp>
#include <Array.hpp>

#include "world/Chunk.hpp"

using namespace godot;

class ChunkRenderer: public Spatial {
    GODOT_CLASS(ChunkRenderer, Spatial);
public:
    void _init() {}
	static void _register_methods();

    static Chunk* generateChunk(Vector3 center, bool forceRegenerate = false, bool worldSpace = true);
    static void bakeChunk(Chunk* chunk, int LoD = 0);

    void _enter_tree();
};
