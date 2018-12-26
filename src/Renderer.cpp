/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
Joshua Dahl		   2018-12-25		  0.1 - Added support for opaque blocks
*/

#include "Renderer.hpp"
#include "Godotize.hpp"
#include "BlockList.hpp"
#include "Vector3Extra.hpp"
#include "MaterialList.hpp"

#include <CSGBox.hpp>
#include <CSGMesh.hpp>
#include <CSGCombiner.hpp>
#include <PlaneMesh.hpp>
#include <MeshInstance.hpp>
#include <StaticBody.hpp>
#include <CollisionShape.hpp>
#include <BoxShape.hpp>

#include <cstdlib>

#include <string>
#include <iostream>
#include <thread>
#include <fstream>

#include <SimplexNoise/src/SimplexNoise.h>

#include <ResourceLoader.hpp>
#include <Material.hpp>

using namespace std;

// Constant representing a path to the folder where the world is stored
const string worldPath = "worlds/Test/";
// Instance of SimplexNoise library for temp world gen
SimplexNoise noiseGen = SimplexNoise(1, 1, 2, .5);

// Constants representing the translations and rotations to make all 6 faces of a cube out of planes
const Vector3 upTrans = Vector3(0, .5, 0), upRot = Vector3(0, 180, 0),
	downTrans = Vector3(0, -.5, 0), downRot = Vector3(0, 0, -180),
	leftTrans = Vector3(-.5, 0, 0), leftRot = Vector3(90, -90, 0),
	rightTrans = Vector3(.5, 0, 0), rightRot = Vector3(90, 90, 0),
	frontTrans = Vector3(0, 0, 0.5), frontRot = Vector3(90, 0, 0),
	backTrans = Vector3(0, 0, -.5), backRot = Vector3(90, 180, 0);

/*
NAME:          _register_methods()
DESCRIPTION:   Used to tell the engine what functions this script is providing
*/
void ChunkRenderer::_register_methods() {
    register_method("_enter_tree", &ChunkRenderer::_enter_tree);

    //register_method("generateChunk", &ChunkRenderer::generateChunk);
}

/*
NAME:          setSolidityTest (Block& b)
DESCRIPTION:   Prototype chunk generator which uses layered simplex noise to generate a
				max block height within the interval [8, 20]
*/
void setSolidityTest (Block& b){
	const int max = 10,
            min = -2,
			seed = 12345;
    const float scale = 200;

	Vector3 center = b.getCenter();
    if(b.y < noiseGen.fractal(5, center.x / scale + seed, center.z / scale + seed) * max - (max - min) + 10)
        b.setBlockRef(2);

	//if(center.y >= 14 && center.z < 15)
	//	b.setBlockRef(1);
}

/*
NAME:          generateChunk(Vector3 center, bool forceRegenerate, bool worldSpace)
DESCRIPTION:   Either loads the chunk centered at <center> from disc or generates a new one if loading isn't possible
NOTES:			Setting forceRegenerate to true will cause the function to never load from disc
*/
Chunk* ChunkRenderer::generateChunk(Vector3 center, bool forceRegenerate, bool worldSpace){
	// Create the name of the chunk file to load
    string filePath = worldPath + to_string((int) center.x) + "|" + to_string((int) center.y) + "|" + to_string((int) center.z) + ".chunk";
	// If we are taking in chunk space coordinates convert them to world space
    if(worldSpace) center *= 32;

	// Create a pointer to the new chunk and heap allocate it
    Chunk* out = new Chunk(center);
	// If the chunk file exists, load it
    if(ifstream(filePath).good() && !forceRegenerate){
        loadChunk(filePath.c_str(), *out);
		// Debug
	    Godot::print("Loaded chunk from: " + godotize(filePath));
	// Otherwise, generate a new chunk and save it to the file
	} else {
        out->runOnBlocks(setSolidityTest);
        saveChunk(filePath.c_str(), *out);
		// Debug
	    Godot::print("Saved chunk to: " + godotize(filePath));
    }

	/*
		TODO: when I merge in the chunk map, part of its reinit processes has to include freeing chunks
	*/
	// Return the allocated chunk
    return out;
}

/*
NAME:          makeVoxelNode(Vector3 center, BlockRef* ref, int scale = 1)
DESCRIPTION:   Converts the provided blockRef into a cube of the provided scale
*/
Spatial* makeVoxelNodeTranslucent(Vector3 center, BlockRef* ref, int scale = 1) {
	// Create a spatial to hold the cube
    Spatial* box = Spatial::_new();

	// If the top face's material is 0 don't render it
    if(ref->up > 0){
		// Otherwise... Create a new mesh
        MeshInstance* up = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        up->call_deferred("set_mesh", MaterialList::getPlane(ref->up));
		// Position the plane
        up->set_translation(upTrans * scale);
        up->set_rotation_degrees(upRot);
		up->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", up);
    }

	// If the top face's material is 0 don't render it
    if(ref->down > 0){
		// Otherwise... Create a new mesh
        MeshInstance* down = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        down->call_deferred("set_mesh", MaterialList::getPlane(ref->down));
		// Position the plane
        down->set_translation(downTrans * scale);
        down->set_rotation_degrees(downRot);
		down->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", down);
    }

	// If the top face's material is 0 don't render it
    if(ref->left > 0){
		// Otherwise... Create a new mesh
        MeshInstance* left = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        left->call_deferred("set_mesh", MaterialList::getPlane(ref->left));
		// Position the plane
        left->set_translation(leftTrans * scale);
        left->set_rotation_degrees(leftRot);
		left->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", left);
    }

	// If the top face's material is 0 don't render it
    if(ref->right > 0){
		// Otherwise... Create a new mesh
        MeshInstance* right = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        right->call_deferred("set_mesh", MaterialList::getPlane(ref->right));
		// Position the plane
        right->set_translation(rightTrans * scale);
        right->set_rotation_degrees(rightRot);
		right->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", right);
    }

	// If the top face's material is 0 don't render it
    if(ref->front > 0){
		// Otherwise... Create a new mesh
        MeshInstance* front = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        front->call_deferred("set_mesh", MaterialList::getPlane(ref->front));
		// Position the plane
        front->set_translation(frontTrans * scale);
        front->set_rotation_degrees(frontRot);
		front->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", front);
    }

	// If the top face's material is 0 don't render it
    if(ref->back > 0){
		// Otherwise... Create a new mesh
        MeshInstance* back = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        back->call_deferred("set_mesh", MaterialList::getPlane(ref->back));
		// Position the plane
        back->set_translation(backTrans * scale);
        back->set_rotation_degrees(backRot);
		back->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", back);
    }

	// If the block is solid, generate a collision model for it
	if(ref->solid){
		// Create a collision node
		StaticBody* collisionNode = StaticBody::_new();
		// Create a collision shape node
		CollisionShape* collisionShape = CollisionShape::_new();
		// Create a box shape
		Ref<Shape> shape = BoxShape::_new();

		// Apply the box shape to the collision shape
		collisionShape->set_shape(shape);
		// Apply the collision shape to the collision node
		collisionNode->add_child(collisionShape);

		// Set the scale of the of the collision node
		collisionNode->set_scale(expand(scale));
		// Parent the collision node to the block
		box->call_deferred("add_child", collisionNode);
	}

	// Position the cube where it should be
    box->set_translation(center);
	// Return a pointer to the cube
    return box;
}

/*
NAME:          makeVoxelNode(Vector3 center, BlockRef* ref, int scale = 1)
DESCRIPTION:   Converts the provided blockRef into a constructive solid cube of the provided scale
*/
CSGCombiner* makeVoxelNodeOpaque(Vector3 center, BlockRef* ref, int scale = 1){
	// Create a combiner to merge the cube together
    CSGCombiner* box = CSGCombiner::_new();

	// If the top face's material is 0 don't render it
    if(ref->up > 0){
		// Otherwise... Create a new mesh
        CSGMesh* up = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        up->call_deferred("set_mesh", MaterialList::getPlane(ref->up));
		// Position the plane
        up->set_translation(upTrans * scale);
        up->set_rotation_degrees(upRot);
		up->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(up);
    }

	// If the top face's material is 0 don't render it
    if(ref->down > 0){
		// Otherwise... Create a new mesh
        CSGMesh* down = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        down->call_deferred("set_mesh", MaterialList::getPlane(ref->down));
		// Position the plane
        down->set_translation(downTrans * scale);
        down->set_rotation_degrees(downRot);
		down->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(down);
    }

	// If the top face's material is 0 don't render it
    if(ref->left > 0){
		// Otherwise... Create a new mesh
        CSGMesh* left = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        left->call_deferred("set_mesh", MaterialList::getPlane(ref->left));
		// Position the plane
        left->set_translation(leftTrans * scale);
        left->set_rotation_degrees(leftRot);
		left->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(left);
    }

	// If the top face's material is 0 don't render it
    if(ref->right > 0){
		// Otherwise... Create a new mesh
        CSGMesh* right = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        right->call_deferred("set_mesh", MaterialList::getPlane(ref->right));
		// Position the plane
        right->set_translation(rightTrans * scale);
        right->set_rotation_degrees(rightRot);
		right->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(right);
    }

	// If the top face's material is 0 don't render it
    if(ref->front > 0){
		// Otherwise... Create a new mesh
        CSGMesh* front = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        front->call_deferred("set_mesh", MaterialList::getPlane(ref->front));
		// Position the plane
        front->set_translation(frontTrans * scale);
        front->set_rotation_degrees(frontRot);
		front->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(front);
    }

	// If the top face's material is 0 don't render it
    if(ref->back > 0){
		// Otherwise... Create a new mesh
        CSGMesh* back = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        back->call_deferred("set_mesh", MaterialList::getPlane(ref->back));
		// Position the plane
        back->set_translation(backTrans * scale);
        back->set_rotation_degrees(backRot);
		back->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->add_child(back);
    }

	// Position the cube where it should be
    box->set_translation(center);
	// Set whether or not the block is solid
	box->set_use_collision(ref->solid);
	// Return a pointer to the cube
    return box;
}

/*
NAME:          makeVoxelNode(Vector3 center, BlockRef* ref, int scale = 1)
DESCRIPTION:   Converts the provided blockRef into a cube of the provided scale
NOTES:			Switches between opaque and translucent code depending on blockRef
*/
inline Spatial* makeVoxelNode(Vector3 center, BlockRef* ref, int scale = 1){
	if(ref->opaque)
		return makeVoxelNodeOpaque(center, ref, scale);
	return makeVoxelNodeTranslucent(center, ref, scale);
}

/*
NAME:          makeVoxelNode(Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, int scale = 1)
DESCRIPTION:   Converts the provided list of materials into a cube of the provided scale
*/
Spatial* makeVoxelNode(Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, int scale = 1){
	// Create a temporary blockRef to pass to the first function
	BlockRef temp = BlockRef(0, up, down, left, right, front, back);
	// If this subChunk isn't solid, don't generate collisions
	if(up == 0 && down == 0 && left == 0 && right == 0 && front == 0 && back == 0)
		temp.solid = false;
	// If this subChunk isn't opaque, don't generate csg blocks
	if(!BlockList::getReference(up)->opaque || !BlockList::getReference(down)->opaque ||
			!BlockList::getReference(left)->opaque || !BlockList::getReference(right)->opaque ||
			!BlockList::getReference(front)->opaque || !BlockList::getReference(back)->opaque)
		temp.opaque = false;

	// Return the value from the other version of maxeVoxelNode
    return makeVoxelNode(center, &temp, scale);
}

/*
NAME: 			bakeChunk(Chunk* chunk, int LoD)
DESCRIPTION: 	Creates the node tree for the provided <chunk> at the request level of detail <LoD>
*/
void ChunkRenderer::bakeChunk(Chunk* chunk, int LoD){
	// If the chunk isn't locked
	if(!chunk->locked)
		// Only rebake the chunk if it's level of detail has changed
		// initial bakes will always be preformed since curLoD is set to -1 by default
		if(chunk->curLoD != LoD){
			Godot::print("Baking " + godotize(to_string(chunk->getCenter() / 32, true)) + " at LoD: " + String::num(LoD));
			// Lock the chunk
			chunk->locked = true;

			// If this is a rebake... remove the old bake from memory
			if(chunk->node != nullptr)
				chunk->node->queue_free();
			// Update the chunk's level of detail counter to the new one
			chunk->curLoD = LoD;

			// Create a new spatial for the chunk base to originate from
			chunk->node = CSGCombiner::_new();
			//chunk->node->set_translation(chunk->getCenter());

			if(LoD < 4) { // lod != 4
				for(SubChunk8& c8: chunk->subChunks) if(LoD < 3){ // lod != 3
					    for(SubChunk4& c4: c8.subChunks) if(LoD < 2){ // lod != 2
						        for(SubChunk2& c2: c4.subChunks) if(LoD < 1){ // lod != 1
										for(Block& b: c2.blocks) // lod == 0
											chunk->node->call_deferred("add_child", makeVoxelNode(b.getCenter() / 2, b.blockRef) );
									} else { // lod == 1
										chunk->node->call_deferred("add_child", makeVoxelNode(c2.getCenter() / 2, c2.up, c2.down, c2.left, c2.right, c2.front, c2.back, SubChunk2::SCALE) );
									}
							} else { // lod == 2
								chunk->node->call_deferred("add_child", makeVoxelNode(c4.getCenter() / 2, c4.up, c4.down, c4.left, c4.right, c4.front, c4.back, SubChunk4::SCALE) );
							}
					} else { // lod == 3
						chunk->node->call_deferred("add_child", makeVoxelNode(c8.getCenter() / 2, c8.up, c8.down, c8.left, c8.right, c8.front, c8.back, SubChunk8::SCALE) );
					}
			} else { // lod == 4
				chunk->node->call_deferred("add_child", makeVoxelNode(chunk->getCenter() / 2, chunk->up, chunk->down, chunk->left, chunk->right, chunk->front, chunk->back, Chunk::SCALE) );
			}

			// Unlock the chunk
			chunk->locked = false;
		}
}

/*
NAME:          _enter_tree()
DESCRIPTION:   Runs when the node is created... just some code testing chunk generation
*/
void ChunkRenderer::_enter_tree(){
	/*
		TODO: move this call somewhere else?
	*/
    MaterialList::initMaterialList();

    Chunk* chunk = generateChunk(Vector3(0, 0, 0), true);
	Chunk* chunk2 = generateChunk(Vector3(0, 0, 1), true);
	Chunk* chunk3 = generateChunk(Vector3(0, 0, 2), true);
	Chunk* chunk4 = generateChunk(Vector3(0, 0, 3), true);
	Chunk* chunk5 = generateChunk(Vector3(0, 0, 4), true);
	Chunk* chunk6 = generateChunk(Vector3(0, -1, 4), true);

    thread bc0(bakeChunk, chunk, 0);
	thread(bakeChunk, chunk2, 1).detach();
	thread(bakeChunk, chunk3, 2).detach();
	thread(bakeChunk, chunk4, 3).detach();
	thread(bakeChunk, chunk5, 4).detach();
	thread(bakeChunk, chunk6, 4).detach();

	bc0.join();
	add_child(chunk->node);
	add_child(chunk2->node);
	add_child(chunk3->node);
	add_child(chunk4->node);
	add_child(chunk5->node);
	add_child(chunk6->node);
}
