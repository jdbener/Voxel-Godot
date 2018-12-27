/*
FILE:              ChunkRenderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-24		  0.0 - Initial testing version
Joshua Dahl		   2018-12-25		  0.1 - Added support for opaque blocks
Joshua Dahl		   2018-12-25		  0.2 - Optimized interior faces to never be rendered
Joshua Dahl		   2018-12-26		  0.3 - Integrated ChunkMap
*/

#include "ChunkRenderer.hpp"
#include "Godotize.hpp"
#include "BlockList.hpp"
#include "Vector3Extra.hpp"
#include "MaterialList.hpp"
#include "ChunkGenerator.hpp"

#include <CSGBox.hpp>
#include <CSGMesh.hpp>
#include <CSGCombiner.hpp>
#include <PlaneMesh.hpp>
#include <MeshInstance.hpp>
#include <StaticBody.hpp>
#include <CollisionShape.hpp>
#include <BoxShape.hpp>

#include <Panel.hpp>
#include <Label.hpp>

#include <cstdlib>

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

#include <ResourceLoader.hpp>
#include <Material.hpp>

using namespace std;

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
	register_method("_process", &ChunkRenderer::_process);

    //register_method("generateChunk", &ChunkRenderer::generateChunk);
}

/*
NAME:          directionOpaque(Block* in, Vector3 direction, int scale = 1)
DESCRIPTION:   Returns true if the block in <direction> in relation to this <in> is opaque
*/
bool directionOpaque(Block* in, Vector3 direction, int scale = 1){
	if(scale == 1){
		Block* block = in->chunk->getBlock(integize(in->getCenter()) + direction * 2);
		// If the block exists...
		if(block)
			// And is opaque...
			if(block->blockRef->opaque)
				// Return true
				return true;
	} else if (scale == 2) {
		SubChunk2* chunk = in->chunk->getSubChunk2(integize(in->getCenter()) + direction * 2 * scale);
		// If the block exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	} else if (scale == 4) {
		SubChunk4* chunk = in->chunk->getSubChunk4(integize(in->getCenter()) + direction * 2 * scale);
		// If the block exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	} else if (scale == 8) {
		SubChunk8* chunk = in->chunk->getSubChunk8(integize(in->getCenter()) + direction * 2 * scale);
		// If the block exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	}
	return false;
}

/*
NAME:          makeVoxelNodeTranslucent(Block* block, int scale = 1)
DESCRIPTION:   Converts the provided blockRef into a cube of the provided scale
*/
Spatial* makeVoxelNodeTranslucent(Block* block, int scale = 1) {
	// Create a spatial to hold the cube
    Spatial* box = Spatial::_new();
	// Variable storing weather or not any faces were baked
	bool render = false;

	// If the top face's material is 0 don't render it
    if(block->blockRef->up > 0 && !directionOpaque(block, Vector3(0, 1, 0), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* up = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        up->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->up));
		// Position the plane
        up->set_translation(upTrans * scale);
        up->set_rotation_degrees(upRot);
		up->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", up);
		// We baked a face, set render to true
		render = true;
    }

	// If the right face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->down > 0 && !directionOpaque(block, Vector3(0, -1, 0), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* down = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        down->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->down));
		// Position the plane
        down->set_translation(downTrans * scale);
        down->set_rotation_degrees(downRot);
		down->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", down);
		// We baked a face, set render to true
		render = true;
    }

	// If the left face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->left > 0 && !directionOpaque(block, Vector3(-1, 0, 0), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* left = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        left->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->left));
		// Position the plane
        left->set_translation(leftTrans * scale);
        left->set_rotation_degrees(leftRot);
		left->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", left);
		// We baked a face, set render to true
		render = true;
    }

	// If the right face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->right > 0 && !directionOpaque(block, Vector3(1, 0, 0), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* right = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        right->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->right));
		// Position the plane
        right->set_translation(rightTrans * scale);
        right->set_rotation_degrees(rightRot);
		right->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", right);
		// We baked a face, set render to true
		render = true;
    }

	// If the front face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->front > 0 && !directionOpaque(block, Vector3(0, 0, 1), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* front = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        front->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->front));
		// Position the plane
        front->set_translation(frontTrans * scale);
        front->set_rotation_degrees(frontRot);
		front->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", front);
		// We baked a face, set render to true
		render = true;
    }

	// If the back face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->back > 0 && !directionOpaque(block, Vector3(0, 0, -1), scale)){
		// Otherwise... Create a new mesh
        MeshInstance* back = MeshInstance::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        back->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->back));
		// Position the plane
        back->set_translation(backTrans * scale);
        back->set_rotation_degrees(backRot);
		back->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", back);
		// We baked a face, set render to true
		render = true;
    }

	// If the block is solid and at least one of the faces was rendered,
	// generate a collision model for it
	if(block->blockRef->solid && render){
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
    box->set_translation(block->getCenter() / 2);
	// Return a pointer to the cube
    return box;
}

/*
NAME:          makeVoxelNodeOpaque(Block* block, int scale = 1)
DESCRIPTION:   Converts the provided blockRef into a constructive solid cube of the provided scale
*/
CSGCombiner* makeVoxelNodeOpaque(Block* block, int scale = 1){
	// Create a combiner to merge the cube together
    CSGCombiner* box = CSGCombiner::_new();
	bool render = false;

	// If the top face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->up > 0 && !directionOpaque(block, Vector3(0, 1, 0), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* up = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        up->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->up));
		// Position the plane
        up->set_translation(upTrans * scale);
        up->set_rotation_degrees(upRot);
		up->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", up);
		// We baked a face, set render to true
		render = true;
    }

	// If the bottom face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->down > 0 && !directionOpaque(block, Vector3(0, -1, 0), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* down = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        down->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->down));
		// Position the plane
        down->set_translation(downTrans * scale);
        down->set_rotation_degrees(downRot);
		down->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", down);
		// We baked a face, set render to true
		render = true;
    }

	// If the left face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->left > 0 && !directionOpaque(block, Vector3(-1, 0, 0), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* left = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        left->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->left));
		// Position the plane
        left->set_translation(leftTrans * scale);
        left->set_rotation_degrees(leftRot);
		left->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", left);
		// We baked a face, set render to true
		render = true;
    }

	// If the right face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->right > 0 && !directionOpaque(block, Vector3(1, 0, 0), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* right = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        right->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->right));
		// Position the plane
        right->set_translation(rightTrans * scale);
        right->set_rotation_degrees(rightRot);
		right->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", right);
		// We baked a face, set render to true
		render = true;
    }

	// If the front face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->front > 0 && !directionOpaque(block, Vector3(0, 0, 1), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* front = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        front->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->front));
		// Position the plane
        front->set_translation(frontTrans * scale);
        front->set_rotation_degrees(frontRot);
		front->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", front);
		// We baked a face, set render to true
		render = true;
    }

	// If the back face's material is 0 or it is facing an opaque block don't render it
    if(block->blockRef->back > 0 && !directionOpaque(block, Vector3(0, 0, -1), scale)){
		// Otherwise... Create a new mesh
        CSGMesh* back = CSGMesh::_new();
		// Apply a copy of the plane storing the requested material from the MaterialList
        back->call_deferred("set_mesh", MaterialList::getPlane(block->blockRef->back));
		// Position the plane
        back->set_translation(backTrans * scale);
        back->set_rotation_degrees(backRot);
		back->set_scale(expand(scale));
		// Parent the plane to the cube merger
        box->call_deferred("add_child", back);
		// We baked a face, set render to true
		render = true;
    }

	// Position the cube where it should be
    box->set_translation(block->getCenter() / 2);
	// Set whether or not the block is solid
	box->set_use_collision(block->blockRef->solid);
	// Return a pointer to the cube
    return box;
}

/*
NAME:          makeVoxelNode(Block* block, int scale = 1)
DESCRIPTION:   Converts the provided <block> into a cube of the provided scale
NOTES:			Switches between opaque and translucent code depending on blockRef
*/
inline Spatial* makeVoxelNode(Block* block, int scale = 1){
	// If this voxel is opaque, generate mesh instances instead of CSG
	if(block->blockRef->opaque)
		return makeVoxelNodeOpaque(block, scale);
	return makeVoxelNodeTranslucent(block, scale);
}

/*
NAME:          makeVoxelNode(Chunk* chunk, Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, int scale = 1)
DESCRIPTION:   Converts the provided list of materials into a cube of the provided scale
*/
Spatial* makeVoxelNode(Chunk* chunk, Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, int scale = 1){
	// Create a temporary blockRef to pass to the first function
	BlockRef temp = BlockRef(0, up, down, left, right, front, back);
	// If this subChunk isn't solid, don't generate collisions
	if(up == 0 && down == 0 && left == 0 && right == 0 && front == 0 && back == 0)
		temp.solid = false;
	// If this subChunk isn't opaque, generate instances instead of CSG
	if(!BlockList::getReference(up)->opaque || !BlockList::getReference(down)->opaque ||
			!BlockList::getReference(left)->opaque || !BlockList::getReference(right)->opaque ||
			!BlockList::getReference(front)->opaque || !BlockList::getReference(back)->opaque)
		temp.opaque = false;

	Block b = Block(chunk, center, &temp);

	// Return the value from the other version of maxeVoxelNode
    return makeVoxelNode(&b, scale);
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
											chunk->node->call_deferred("add_child", makeVoxelNode(&b) );
									} else { // lod == 1
										chunk->node->call_deferred("add_child", makeVoxelNode(chunk, c2.getCenter(), c2.up, c2.down, c2.left, c2.right, c2.front, c2.back, SubChunk2::SCALE) );
									}
							} else { // lod == 2
								chunk->node->call_deferred("add_child", makeVoxelNode(chunk, c4.getCenter(), c4.up, c4.down, c4.left, c4.right, c4.front, c4.back, SubChunk4::SCALE) );
							}
					} else { // lod == 3
						chunk->node->call_deferred("add_child", makeVoxelNode(chunk, c8.getCenter(), c8.up, c8.down, c8.left, c8.right, c8.front, c8.back, SubChunk8::SCALE) );
					}
			} else { // lod == 4
				chunk->node->call_deferred("add_child", makeVoxelNode(chunk, chunk->getCenter(), chunk->up, chunk->down, chunk->left, chunk->right, chunk->front, chunk->back, Chunk::SCALE) );
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

	chunkMap = ChunkMap(4 * 1, Vector3());

	//add_child(nullptr);

	/*for(Chunk* cur: chunkMap.sphere){
		thread(bakeChunk, cur, 0).detach();
	}

	int finished = 0;
	while(finished < chunkMap.sphere.size()){
		finished = 0;
		for(Chunk* cur: chunkMap.sphere)
			if(cur->node){
				if(!cur->added){
					call_deferred("add_child", cur->node);
					cur->added = true;
				}
				finished++;
			}
		Godot::print("Finished! " + String::num(finished / chunkMap.sphere.size()));
		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	Godot::print("Building World Geometry");


			//thread b0/*, b1, b2, b3*/;
			/*if(i < chunkMap.sphere.size())
				b0 = thread(bakeChunk, chunkMap.sphere[i], 0);
			if(i + 1 < chunkMap.sphere.size())
				b1 = thread(bakeChunk, chunkMap.sphere[i + 1], 0);
			if(i + 2 < chunkMap.sphere.size())
				b2 = thread(bakeChunk, chunkMap.sphere[i + 2], 0);
			if(i + 3 < chunkMap.sphere.size())
				b3 = thread(bakeChunk, chunkMap.sphere[i + 3], 0);*/

			/*if(b0.joinable()) { b0.join(); add_child(chunkMap.sphere[i]->node); }
			if(b1.joinable()) { b1.join(); add_child(chunkMap.sphere[i + 1]->node); }
			if(b2.joinable()) { b2.join(); add_child(chunkMap.sphere[i + 2]->node); }
			if(b3.joinable()) { b3.join(); add_child(chunkMap.sphere[i + 3]->node); }*/



    /*Chunk* chunk = generateChunk(Vector3(0, 0, 0));
	Chunk* chunk2 = generateChunk(Vector3(0, 0, 1));
	Chunk* chunk3 = generateChunk(Vector3(0, 0, 2));
	Chunk* chunk4 = generateChunk(Vector3(0, 0, 3));
	Chunk* chunk5 = generateChunk(Vector3(0, 0, 4));
	Chunk* chunk6 = generateChunk(Vector3(0, -1, 4));

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
	add_child(chunk6->node);*/
}

void ChunkRenderer::_process(float delta){
	static int i = 0;

	Panel* panel = Node::cast_to<Panel>(get_parent()->get_node("CanvasLayer/Loading Panel"));
	Label* label = Node::cast_to<Label>(panel->get_node("Loading"));

	if(i < chunkMap.sphere.size()){
		thread t;
		if(i < chunkMap.sphere.size()){
			int LoD = (chunkMap.sphere[i]->getCenter() / 32).distance_squared_to(chunkMap.origin) / chunkMap.radius;
			t = thread(bakeChunk, chunkMap.sphere[i], LoD);

			if(label) label->set_text("Loading: " + String::num(((float) i / chunkMap.sphere.size()) * 100, 2) + "%");
		}

		t.join(); add_child(chunkMap.sphere[i]->node);
		i++;
	} else {
		if(panel) panel->set_visible(false);


	}
}
