/*
FILE:              init.cpp
DESCRIPTION:       File which initializes the NativeScripts for the project

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  1.0 - Godotized project
*/

#include <Godot.hpp>

#include "ChunkMap.hpp"
#include "Renderer.hpp"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<ChunkMap>();

    godot::register_class<ChunkRenderer>();
}
