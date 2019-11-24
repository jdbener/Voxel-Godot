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
//    c->rebuildMesh();
//    c->buildWireframe();

    gout << String("res://icon.png").get_file() << endl;

	int level = 0;
	std::vector<Face> faces = c->getLayerFaces(BOTTOM, level);
	gout << faces.size() << endl;
    Surface surf;
    //for(int level = 0; level <= 16; level++){
        surf += Surface::GreedyMeshLayer(faces, BOTTOM, Vector3(0, 0, 0));
    //}

    this->add_child(surf.getWireframe());
	Surface s2;
	if(faces.size())
		for (Face& f: faces)
			s2.append(f.getSurface());
    this->set_mesh(s2.getMesh());
    add_child(c, true);
}
