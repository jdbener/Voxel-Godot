/*
FILE:              ChunkRenderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
Joshua Dahl		   2018-12-25		  0.1 - Added support for opaque blocks
Joshua Dahl		   2018-12-25		  0.2 - Optimized interior faces to never be rendered
Joshua Dahl		   2018-12-26		  0.3 - Integrated ChunkMap
*/

#include <Godot.hpp>
#include <Spatial.hpp>
#include <Array.hpp>

#include "world/Chunk.hpp"
#include "ChunkMap.hpp"

using namespace godot;

class ChunkRenderer: public Spatial {
    GODOT_CLASS(ChunkRenderer, Spatial);
public:
    ChunkMap chunkMap;


    void _init() {}
	static void _register_methods();

    static void bakeChunk(Chunk* chunk, int LoD = 0);

    void _enter_tree();
    void _process(float delta);
};
