#ifndef __BLOCK_FEATURE_DATABASE__
#define __BLOCK_FEATURE_DATABASE__
#include <map>

#include "../godot/CerealGodot.h"

// Base class representing an abstract block feature
class Feature {
public:
	// Variable representing the name of the feature
    const godot::String name;
	// Variable storing if a block with this feature can be pruned
	bool pruneable = true;
	// Constructer/destructer
    Feature(const char* _name) : name(_name){}
    virtual ~Feature() {}

	// Return a dynamically allocated copy of this feature, the constructor should only set a feature's name
	// This function should act as a class's constructor
    virtual Feature* _new() const { return nullptr; }

	// Functions which dictate what hapens when this feature is loaded/saved from disc
	virtual void save(oarchive& archive) const {}
	virtual void load(iarchive& archive) {}

    // Utility function which checks if a feature is the specified feature
    bool is(const char name[]) const { return this->name == godot::String(name); }
};

// NOTE: all features should be loaded before the first reference to the BlockDatabase
class BlockFeatureDatabase {
public:
	// Map storing all of the loaded features
	std::map<const godot::String, Feature*> features;
	// Function which gets a reference to the singleton for the database
	static BlockFeatureDatabase* getSingleton();

	// Function which adds the built in features
    BlockFeatureDatabase();
	// Function which cleans up
	~BlockFeatureDatabase();
    // Function which adds a feature to the database
    void addFeature(const char name[], Feature* d) { features[name] = d; }
    // Function which gets an instance of one of the features in the database
    Feature* getFeature(const char name[]) const;
	// Function which gets a list of feature instances
	std::map<godot::String, Feature*> getFeatures(std::initializer_list<godot::String> list) const;
	// Function which checks if a feature is in the database
	bool hasFeature(const char name[]) const { return features.find(name) != features.end(); }

};

#endif // __BLOCK_FEATURE_DATABASE__
