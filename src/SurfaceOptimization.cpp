#include "SurfaceOptimization.h"

#include "Geometry.hpp"
#include "SurfaceTool.hpp"

#include <iomanip>
#include <vector>
#include <fstream>

#include <bitset>

#include "timer.h"
#include "godot/Gstream.hpp"
#include "world/ChunkMap.h"
#include "block/BlockDatabase.h"

void SurfaceOptimization::_ready(){
    gout << BlockDatabase::getSingleton()->getBlock(0)->checkFlag(BlockData::TRANSPARENT) << endl;
    gout << BlockDatabase::getSingleton()->getBlock(1)->checkFlag(BlockData::TRANSPARENT) << endl;

    ChunkMap* map = ChunkMap::_new();
    add_child(map);
    {
        std::ofstream os("test.chunk.json", std::ios::binary);
        oarchive save(os);
        save(*map->chunk);
    }

    Chunk* c = Chunk::_new();
    c->initalize(map);
    {
        std::ifstream is("test.chunk.json", std::ios::binary);
        iarchive load(is);
        load(*c);
    }
    c->rebuildMesh();
    c->buildWireframe();

    gout << String("res://icon.png").get_file() << endl;

    Surface surf;
    //for(int level = 3; level <= 7; level++){
    int level = 5;
        std::vector<Face> facesArr = c->getLayer(BOTTOM, level);

        if(facesArr.size())
            for (Face& f: facesArr)
                surf.append(f.getSurface());
    //}

    //visualizeEdges(surf, surf.norms[0]);
    this->set_mesh(surf.getMesh());
    add_child(c, true);
}
