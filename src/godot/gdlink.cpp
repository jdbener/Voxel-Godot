#include "../SurfaceOptimization.h"
#include "../world/Chunk.h"
#include "../world/ChunkMap.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {

	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	// Initialize the link with godot
	godot::Godot::nativescript_init(handle);

	godot::register_class<SurfaceOptimization>();
	godot::register_class<ChunkMap>();
	godot::register_class<Chunk>();
}
