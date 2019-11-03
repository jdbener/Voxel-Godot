#ifndef __ORIENTATION_FEATURE_H__
#define __ORIENTATION_FEATURE_H__
#include "../BlockFeatureDatabase.h"

// constant representing the name of the feature
namespace Features { const char ORIENTATION[] = "Orientation"; }

// Class representing the possible orientations of a block
class OrientationFeature: public Feature {
public:
    enum Orientaion {NORTH = 1, EAST = 2, UP = 4};
    Orientaion orientation = NORTH;

    OrientationFeature() : Feature(Features::ORIENTATION) {}
    virtual ~OrientationFeature(){}

    virtual Feature* _new() const { return new OrientationFeature(); }

    virtual void load(iarchive& archive){ archive(cereal::make_nvp("Orientation", orientation)); }
    virtual void save(oarchive& archive) const { archive(cereal::make_nvp("Orientation", orientation)); }
};

#endif // __ORIENTATION_FEATURE_H__
