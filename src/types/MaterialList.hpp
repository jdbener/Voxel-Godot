/*
FILE:              MaterialList.hpp
DESCRIPTION:       Provides a list of the surface materials blocks use

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  0.0 - Initial testing version
Joshua Dahl		   2019-01-04		  0.1 - Removed the plane component
*/

#ifndef MATERIAL_LIST_H
#define MATERIAL_LIST_H

#include <ResourceLoader.hpp>
#include <Material.hpp>
#include <Reference.hpp>

#include <vector>

using namespace godot;

namespace MaterialList {
    static std::vector<Ref<Material>> materials;

    /*
    NAME:           addMaterial(Ref<Material> mat)
    DESCRIPTION:    Adds a plane with the provided material applied to it the the list of planes
    */
    static void addMaterial(Ref<Material> mat){
        // Add it to the array
        materials.push_back(mat);
    }

    /* ----------------------------------------------------------------------------
        This is where the MaterialList is defined
    ---------------------------------------------------------------------------- */
    static void initMaterialList(){
        static bool first = true;
        if(first){
            MaterialList::addMaterial(ResourceLoader::get_singleton()->load("res://materials/null.tres")); //0, null
            MaterialList::addMaterial(ResourceLoader::get_singleton()->load("res://materials/TansparentTest.tres")); //1, debug
            MaterialList::addMaterial(ResourceLoader::get_singleton()->load("res://materials/stone.tres")); //2, stone
            MaterialList::addMaterial(ResourceLoader::get_singleton()->load("res://materials/dirt.tres")); //3, dirt
            MaterialList::addMaterial(ResourceLoader::get_singleton()->load("res://materials/grass.tres")); //4, grass

            first = false;
        }
    }

    /*
    NAME:           getMaterial(matID ID)
    DESCRIPTION:    Gets the material stored on the plane stored at index ID
    */
    static Ref<Material> getMaterial(matID ID){
        if(materials.size() < ID) return materials[0];
        return materials[ID];
    }
}

#endif
