#include "BlockFeatureDatabase.h"

// Block Features
#include "features/Orientation.h"

// Function which gets a reference to the singleton for the database
BlockFeatureDatabase* BlockFeatureDatabase::getSingleton(){
    static BlockFeatureDatabase db;
    return &db;
}

// Function which adds the built in features
BlockFeatureDatabase::BlockFeatureDatabase(){
    addFeature(Features::ORIENTATION, new OrientationFeature());
}

// Function which cleans up
BlockFeatureDatabase::~BlockFeatureDatabase(){
    for(auto f: features) delete f.second;
}

// Function which gets a copy of one of the features in the database
Feature* BlockFeatureDatabase::getFeature(const char name[]) const {
    // If the feature isn't in the database
    if(!hasFeature(name)) return nullptr;

    return features.at(name)->_new();
}

std::map<godot::String, Feature*> BlockFeatureDatabase::getFeatures(const std::initializer_list<godot::String> list) const {
    std::map<godot::String, Feature*> out;
    for(const godot::String& name: list)
        if(hasFeature(name.utf8().get_data()))
            out[name] = getFeature(name.utf8().get_data());
    return out;
}
