/*
FILE:              Renderer.hpp
DESCRIPTION:       Class which provides a means for interfacing Chunks with Godot

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl		   2018-12-21		  0.0 - Initial testing version
*/

#include "Renderer.hpp"
#include "Godotize.hpp"
#include "BlockList.hpp"
#include "Vector3Extra.hpp"

#include <CSGBox.hpp>
#include <ResourceLoader.hpp>
#include <Material.hpp>

#include <cstdlib>

#include <string>
#include <iostream>
#include <map>

using namespace godot;
using namespace std;

const string fileName = "worlds/Test/test.chunk";
short heightmap [16 * 16];

void ChunkRenderer::_register_methods() {
    register_method("_ready", &ChunkRenderer::_ready);
    register_method("_enter_tree", &ChunkRenderer::_enter_tree);

}

void setSolidityTest (Block& b){
    if(b.y < heightmap[abs(b.z * 16 + b.x)])
        b.setBlockRef(1);
}

void ChunkRenderer::_enter_tree(){
    Ref<Material> TransparentTest = ResourceLoader::get_singleton()->load(godotize("res://materials/TansparentTest.tres"));

    /*srand(time(0));
    for(int i = 0; i < (16 * 16); i++)
        heightmap[i] = rand() % (10 - (-2)) - 2;

    chunk.runOnBlocks(setSolidityTest);

    saveChunk(fileName.c_str(), chunk);
    Godot::print("Wrote chunk to: " + godotize(fileName));*/

    loadChunk(fileName.c_str(), chunk);
    Godot::print("Loaded chunk from: " + godotize(fileName));

    RUN_ON_BLOCKS(chunk.)
        if(block.blockRef->visible == true){
            CSGBox* cube = CSGBox::_new();
            cube->set_width(1); cube->set_height(1); cube->set_depth(1);
            cube->set_translation(block.getCenter() / 2);
            cube->set_material(TransparentTest);
            add_child(cube);
        }
}

void ChunkRenderer::_ready(){
    Block* block = chunk.getBlock(Vector3(1, 1, -1));
    Godot::print(Vector3(1, 1, -1));
    Godot::print("solidity is " + godotize(block->blockRef->solid ? "true" : "false"));
}
