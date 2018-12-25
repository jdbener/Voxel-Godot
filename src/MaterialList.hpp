/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  0.0 - Initial testing version
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
    std::vector<PlaneMesh*> planes;
};

namespace MaterialList {
    static _MaterialList materials;

    /*
    NAME:           addMaterial(Ref<Material> mat)
    DESCRIPTION:    Adds a plane with the provided material applied to it the the list of planes
    */
    static void addMaterial(Ref<Material> mat){
        // Create a new plane
        PlaneMesh* plane = PlaneMesh::_new();
        // Set it's size to 1x1
        plane->set_size(Vector2(1, 1));
        // Apply the material
        plane->set_material(mat);
        // Add it to the array
        materials.planes.push_back(plane);
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
        if(materials.planes.size() < ID) return materials.planes[0]->get_material();
        return materials.planes[ID]->get_material();
    }

    /*
    NAME:           getPlane(matID ID)
    DESCRIPTION:    Gets the plane stored at index ID
    */
    static Ref<Mesh> getPlane(matID ID){
        Ref<Mesh> ref;

        if(materials.planes.size() < ID) ref = Ref<Mesh>(materials.planes[0]);
        ref = Ref<Mesh>(materials.planes[ID]);

        return ref;
    }

}

#endif
