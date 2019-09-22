#ifndef _SURFACE_OPTIMIZATION_H_
#define _SURFACE_OPTIMIZATION_H_
#include "SurfFaceEdge.h"
#include "MeshInstance.hpp"

class SurfaceOptimization: public MeshInstance{
    GODOT_CLASS(SurfaceOptimization, MeshInstance)

public:
    static void _register_methods(){
        register_method("_ready", &SurfaceOptimization::_ready);
    }
    void _init(){}

    void _ready();
    void visualizeEdges(Surface surface, Vector3 normal = Vector3(0, 0, 0));
};

#endif
