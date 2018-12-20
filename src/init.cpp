/*
FILE:              init.cpp
DESCRIPTION:       File which initializes the NativeScripts for the project

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Godotized project
*/

#include <Godot.hpp>

//#include "world/Chunk.hpp"
#include "ChunkMap.hpp"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<ChunkMap>();

    /*godot::register_class<Voxel>();

    godot::register_class<BlockRef>();
    godot::register_class<_BlocksManager>();

    godot::register_class<Block>();

    godot::register_class<ChunkBase>();
    godot::register_class<Chunk>();
    godot::register_class<SubChunk8>();
    godot::register_class<SubChunk4>();
    godot::register_class<SubChunk2>();*/
}
