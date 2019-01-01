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
Joshua Dahl		   2018-12-31		  1.0 - Switched from instancing to using a mesh generator
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
#include <SurfaceTool.hpp>
#include <ArrayMesh.hpp>
#include <Vector2.hpp>

#include <Panel.hpp>
#include <Label.hpp>

#include <RayCast.hpp>

#include <cstdlib>

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>

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
Returns:   Returns true if the block in <direction> in relation to this block (<in>) is opaque
*/
bool directionOpaque(Block* in, Vector3 direction, int scale = 1){
	if(scale == 1){
		/*
			TODO: load block/subChunk from chunkmap instead of chunk
			this should theoretically stop internal chunk faces from rendering
		*/
		Block* block = in->chunk->getBlock(in->getCenter() + direction * 2);
		// If the block exists...
		if(block)
			// And is opaque...
			if(block->blockRef->opaque)
				// Return true
				return true;
	} else if (scale == SubChunk2::SCALE) {
		SubChunk2* chunk = in->chunk->getSubChunk2(in->getCenter() + direction * 2 * scale);
		// If the subChunk exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	} else if (scale == SubChunk4::SCALE) {
		SubChunk4* chunk = in->chunk->getSubChunk4(in->getCenter() + direction * 2 * scale);
		// If the subChunk exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	} else if (scale == SubChunk8::SCALE) {
		SubChunk8* chunk = in->chunk->getSubChunk8(in->getCenter() + direction * 2 * scale);
		// If the subChunk exists...
		if(chunk)
			// And is opaque...
			if(chunk->opaque)
				// Return true
				return true;
	}
	return false;
}

/*
NAME:			makeVoxelNode(Block* block, bool last = false, int scale)
DESCRIPTION:	Converts the provided <block> into a cube of the provided <scale>
RETURNS:		Nullptr if this is not the <last> call, otherwise it returns a pointer
				to a MeshInstance containing the generated Mesh
*/
Spatial* makeVoxelNode(Block* block, bool last = false, int scale = 1){
	/*
	MACRO:			initSide(side)
	DESCRIPTION:	A macro which adds a <matID, SurfaceTool> pair to chunkSurfaces map.
					Also defines a variable <side> which is a pointer to the matID's SurfaceTool
	*/
	#define initSide(side)	\
		/* If a surface for the matID doesn't exist in the map... */ \
		if(!chunkSurfaces[block->blockRef->side]){ 	\
			/* Create a new surface */	\
			chunkSurfaces[block->blockRef->side] = SurfaceTool::_new();	\
			/* Declare that the surface is made of triangles */	\
			chunkSurfaces[block->blockRef->side]->begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);	\
			/* Set the surface's material */	\
			chunkSurfaces[block->blockRef->side]->set_material(MaterialList::getMaterial(block->blockRef->side));	\
		}	\
		/* Create a pointer to the surface for the matID */	\
		SurfaceTool* side = chunkSurfaces[block->blockRef->side]

	/*------------------------------------------------------------------------*/
	// Variable storing <matID, SurfaceTool> pairs
	static map<matID, SurfaceTool*> chunkSurfaces;

	// If the top of the block is not null, and is not facing an opaque block
	if(block->blockRef->up && !directionOpaque(block, Vector3(0, 1, 0), scale)){
		// Setup the SurfaceTool
		initSide(up);

		// Add the vertexes
		// 1
		up->add_uv(Vector2(0, 0));
		up->add_vertex(Vector3(-.5, .5, -.5) * scale + block->getCenter() / 2);

		// 2
		up->add_uv(Vector2(1, 0));
		up->add_vertex(Vector3(.5, .5, -.5) * scale + block->getCenter() / 2);

		//3
		up->add_uv(Vector2(1, 1));
		up->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);

		//3
		up->add_uv(Vector2(1, 1));
		up->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);

		// 4
		up->add_uv(Vector2(0, 1));
		up->add_vertex(Vector3(-.5, .5, .5) * scale + block->getCenter() / 2);

		// 1
		up->add_uv(Vector2(0, 0));
		up->add_vertex(Vector3(-.5, .5, -.5) * scale + block->getCenter() / 2);
	}

	// If the bottom of the block is not null, and is not facing an opaque block
	if(block->blockRef->down && !directionOpaque(block, Vector3(0, -1, 0), scale)){
		// Setup the SurfaceTool
		initSide(down);


		//3
		down->add_uv(Vector2(1, 1));
		down->add_vertex(Vector3(.5, -.5, .5) * scale + block->getCenter() / 2);

		// 2
		down->add_uv(Vector2(1, 0));
		down->add_vertex(Vector3(.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 1
		down->add_uv(Vector2(0, 0));
		down->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 1
		down->add_uv(Vector2(0, 0));
		down->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 4
		down->add_uv(Vector2(0, 1));
		down->add_vertex(Vector3(-.5, -.5, .5) * scale + block->getCenter() / 2);

		//3
		down->add_uv(Vector2(1, 1));
		down->add_vertex(Vector3(.5, -.5, .5) * scale + block->getCenter() / 2);
	}

	// If the left side of the block is not null, and is not facing an opaque block
	if(block->blockRef->left && !directionOpaque(block, Vector3(1, 0, 0), scale)){
		// Setup the SurfaceTool
		initSide(left);

		// Add the vertexes
		// 1
		left->add_uv(Vector2(0, 0));
		left->add_vertex(Vector3(.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 2
		left->add_uv(Vector2(1, 0));
		left->add_vertex(Vector3(.5, -.5, .5) * scale + block->getCenter() / 2);

		// 3
		left->add_uv(Vector2(1, 1));
		left->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);

		// 3
		left->add_uv(Vector2(1, 1));
		left->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);

		// 4
		left->add_uv(Vector2(0, 1));
		left->add_vertex(Vector3(.5, .5, -.5) * scale + block->getCenter() / 2);

		// 1
		left->add_uv(Vector2(0, 0));
		left->add_vertex(Vector3(.5, -.5, -.5) * scale + block->getCenter() / 2);

	}

	// If the right side of the block is not null, and is not facing an opaque block
	if(block->blockRef->right && !directionOpaque(block, Vector3(-1, 0, 0), scale)){
		// Setup the SurfaceTool
		initSide(right);

		// Add the vertexes (inverse of left)
		// 3
		right->add_uv(Vector2(1, 1));
		right->add_vertex(Vector3(-.5, .5, .5) * scale + block->getCenter() / 2);

		// 2
		right->add_uv(Vector2(1, 0));
		right->add_vertex(Vector3(-.5, -.5, .5) * scale + block->getCenter() / 2);

		// 1
		right->add_uv(Vector2(0, 0));
		right->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 1
		right->add_uv(Vector2(0, 0));
		right->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 4
		right->add_uv(Vector2(0, 1));
		right->add_vertex(Vector3(-.5, .5, -.5) * scale + block->getCenter() / 2);

		// 3
		right->add_uv(Vector2(1, 1));
		right->add_vertex(Vector3(-.5, .5, .5) * scale + block->getCenter() / 2);
	}

	// If the front side of the block is not null, and is not facing an opaque block
	if(block->blockRef->front && !directionOpaque(block, Vector3(0, 0, -1), scale)){
		// Setup the SurfaceTool
		initSide(front);

		// Add the vertexes
		// 1
		front->add_uv(Vector2(0, 0));
		front->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 2
		front->add_uv(Vector2(1, 0));
		front->add_vertex(Vector3(.5, -.5, -.5) * scale + block->getCenter() / 2);

		// 3
		front->add_uv(Vector2(1, 1));
		front->add_vertex(Vector3(.5, .5, -.5) * scale + block->getCenter() / 2);

		// 3
		front->add_uv(Vector2(1, 1));
		front->add_vertex(Vector3(.5, .5, -.5) * scale + block->getCenter() / 2);

		// 4
		front->add_uv(Vector2(0, 1));
		front->add_vertex(Vector3(-.5, .5, -.5) * scale + block->getCenter() / 2);

		// 1
		front->add_uv(Vector2(0, 0));
		front->add_vertex(Vector3(-.5, -.5, -.5) * scale + block->getCenter() / 2);
	}

	// If the back side of the block is not null, and is not facing an opaque block
	if(block->blockRef->back && !directionOpaque(block, Vector3(0, 0, 1), scale)){
		// Setup the SurfaceTool
		initSide(back);

		// Add the vertexes (inverse of front)
		// 3
		back->add_uv(Vector2(1, 1));
		back->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);

		// 2
		back->add_uv(Vector2(1, 0));
		back->add_vertex(Vector3(.5, -.5, .5) * scale + block->getCenter() / 2);

		// 1
		back->add_uv(Vector2(0, 0));
		back->add_vertex(Vector3(-.5, -.5, .5) * scale + block->getCenter() / 2);

		// 1
		back->add_uv(Vector2(0, 0));
		back->add_vertex(Vector3(-.5, -.5, .5) * scale + block->getCenter() / 2);

		// 4
		back->add_uv(Vector2(0, 1));
		back->add_vertex(Vector3(-.5, .5, .5) * scale + block->getCenter() / 2);

		// 3
		back->add_uv(Vector2(1, 1));
		back->add_vertex(Vector3(.5, .5, .5) * scale + block->getCenter() / 2);
	}

	// If this is the last block in chunk...
	if(last){
		// Create a mesh instance to be returned
		MeshInstance* out = MeshInstance::_new();
		// Variable which switches between overwriting the blank mesh and appending to the already existing mesh
		bool first = true;
		// For every surface in the chunk
		for(auto cur: chunkSurfaces){
			// Remove duplicate vertexes in the chunk
			cur.second->index();
			// Generate normals and tangents
			cur.second->generate_normals();
			cur.second->generate_tangents();
			// If this is the first surface, set the generated mesh as the instance's mesh
			if(first){
				out->set_mesh(cur.second->commit());
				first = false;
			// Otherwise, append the generated mesh to the instance's mesh
			} else
				cur.second->commit(out->get_mesh());
			// Delete the SurfaceTool
			cur.second->free();
		}
		// Clear out the map of IDs and Tools
		map<matID, SurfaceTool*>().swap(chunkSurfaces);

		// Return the mesh instance with the surfaces added
		return out;
	}
	// If this isn't the last block in the chunk, return nullptr
	return nullptr;
}

/*
NAME:          makeVoxelNode(Chunk* chunk, Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, int scale = 1)
DESCRIPTION:   Converts the provided list of materials into a cube of the provided <scale>
*/
Spatial* makeVoxelNode(Chunk* chunk, Vector3 center, matID up, matID down, matID left, matID right, matID front, matID back, bool last = false, int scale = 1){
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
    return makeVoxelNode(&b, last, scale);
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
			chunk->node = Spatial::_new();
			//chunk->node->set_translation(chunk->getCenter());

			int i = 1;
			if(LoD < 4) { // lod != 4
				for(SubChunk8& c8: chunk->subChunks) if(LoD < 3){ // lod != 3
					    for(SubChunk4& c4: c8.subChunks) if(LoD < 2){ // lod != 2
						        for(SubChunk2& c2: c4.subChunks) if(LoD < 1){ // lod != 1
										for(Block& b: c2.blocks){ // lod == 0
											Spatial* node = makeVoxelNode(&b, i++ > BLOCKS_PER_CHUNK - 1);
											if (node) chunk->node = node;
										}
									} else { // lod == 1
										Spatial* node = makeVoxelNode(chunk, c2.getCenter(), c2.up, c2.down, c2.left, c2.right, c2.front, c2.back, i++ > BLOCKS_PER_CHUNK / 8 - 1, SubChunk2::SCALE);
										if (node) chunk->node = node;
									}
							} else { // lod == 2
								Spatial* node = makeVoxelNode(chunk, c4.getCenter(), c4.up, c4.down, c4.left, c4.right, c4.front, c4.back, i++ > BLOCKS_PER_CHUNK / (8 * 8) - 1, SubChunk4::SCALE);
								if (node) chunk->node = node;
							}
					} else { // lod == 3
						Spatial* node = makeVoxelNode(chunk, c8.getCenter(), c8.up, c8.down, c8.left, c8.right, c8.front, c8.back, i++ > BLOCKS_PER_CHUNK / (8 * 8 * 8) - 1, SubChunk8::SCALE);
						if (node) chunk->node = node;
					}
			} else { // lod == 4
				Spatial* node = makeVoxelNode(chunk, chunk->getCenter(), chunk->up, chunk->down, chunk->left, chunk->right, chunk->front, chunk->back, i++ > BLOCKS_PER_CHUNK / (8 * 8 * 8 * 8) - 1, Chunk::SCALE);
				if (node) chunk->node = node;
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
	//chunkMap = ChunkMap(1, Vector3(), true);
}

void ChunkRenderer::_process(float delta){
	static int i = 0;

	Panel* panel = Node::cast_to<Panel>(get_parent()->get_node("CanvasLayer/Loading Panel"));
	Label* label = Node::cast_to<Label>(panel->get_node("Loading"));

	if(i < chunkMap.sphere.size()){
		thread t;

		int LoD = (chunkMap.sphere[i]->getCenter() / 32).distance_squared_to(chunkMap.origin) / chunkMap.radius;
		t = thread(bakeChunk, chunkMap.sphere[i], LoD);
		t.join(); add_child(chunkMap.sphere[i]->node, true);

		if(label) label->set_text("Loading: " + String::num(((float) i / chunkMap.sphere.size()) * 100, 1) + "%");
		i++;
	} else {
		if(panel) panel->set_visible(false);
	}
}
