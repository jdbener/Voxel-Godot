#ifndef _SURFACE_OPTIMIZATION_H_
#define _SURFACE_OPTIMIZATION_H_
#include "SurfFaceEdge.h"
#include "MeshInstance.hpp"

/*template<class T>
std::wostream& operator<<(std::wostream& s, std::vector<T>& vect){
    s << "{ ";
    for(T& elem: vect)
        s << elem << ", ";
    s.seekp(-2L, std::ios_base::end);
    s << " }";
    return s;
}*/

class SurfaceOptimization: public MeshInstance{
    GODOT_CLASS(SurfaceOptimization, MeshInstance)

public:
    static void _register_methods(){
        register_method("_ready", &SurfaceOptimization::_ready);
    }
    void _init(){}

    void _ready();
};

#endif
