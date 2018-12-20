/*
FILE:               Voxel.hpp
DESCRIPTION:        The base type which blocks, chunks, and sub-chunks inherit from

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-14		  0.0 - Implemented Voxel type (glorified vector3)
Joshua Dahl		   2018-12-14		  1.0 - Godotized file
*/

#ifndef VOXEL_H
#define VOXEL_H

#include <Vector3.hpp>

using namespace godot;

/*
    TODO: Make Voxel's inherit from Godot's Vector3?
*/

class Voxel {
public:
    int x = 0, y = 0, z = 0;

    Voxel(Vector3 center) : x(center.x), y(center.y), z(center.z) { }
    Voxel() { }

    template <class Archive>
    void serialize( Archive & ar ) {
        ar ( CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z) );
    }

    template <class Archive>
    void save(Archive & ar) const {
        ar ( CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z) );
    }

    template <class Archive>
    void load(Archive & ar) {
        ar ( CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z) );
    }
};

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES( Voxel, cereal::specialization::member_serialize )
#endif
