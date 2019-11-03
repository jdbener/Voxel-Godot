#include "SurfaceOptimization.h"

#include "Geometry.hpp"
#include "SurfaceTool.hpp"

#include <iomanip>
#include <vector>
#include <fstream>

#include "timer.h"
#include "godot/Gstream.hpp"
#include "world/ChunkMap.h"
#include "block/BlockDatabase.h"

void SurfaceOptimization::_ready(){
    gout << BlockDatabase::getSingleton()->getBlock(0)->checkFlag(BlockData::TRANSPARENT) << endl;
    gout << BlockDatabase::getSingleton()->getBlock(1)->checkFlag(BlockData::TRANSPARENT) << endl;
    ChunkMap map;
    map.init();
    {
        std::ofstream os("test.chunk.json", std::ios::binary);
        oarchive save(os);
        save(*map.chunk);
    }

    Chunk* c = Chunk::_new();
    c->initalize(&map);
    {
        std::ifstream is("test.chunk.json", std::ios::binary);
        iarchive load(is);
        load(*c);
    }
    c->rebuildMesh();
    c->buildWireframe();

    gout << String("res://icon.png").get_file() << endl;

    //visualizeEdges(surf, surf.norms[0]);
    //this->set_mesh(surf.getMesh());
    add_child(c, true);
}
