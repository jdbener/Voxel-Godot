/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

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
#include <PlaneMesh.hpp>
#include <Reference.hpp>

#include <vector>

using namespace godot;

struct _MaterialList {
    // Variable storing a list of planes
    std::vector<Ref<Material>> materials;
};

namespace MaterialList {
    static _MaterialList materials;

    /*
    NAME:           addMaterial(Ref<Material> mat)
    DESCRIPTION:    Adds a plane with the provided material applied to it the the list of planes
    */
    static void addMaterial(Ref<Material> mat){
        // Add it to the array
        materials.materials.push_back(mat);
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

            first = false;
        }
    }

    /*
    NAME:           getMaterial(matID ID)
    DESCRIPTION:    Gets the material stored on the plane stored at index ID
    */
    static Ref<Material> getMaterial(matID ID){
        if(materials.materials.size() < ID) return materials.materials[0];
        return materials.materials[ID];
    }

}

#endif
