#include "SurfaceOptimization.h"

#include "Geometry.hpp"
#include "SurfaceTool.hpp"

#include <iomanip>
#include <vector>
#include <fstream>

#include "timer.h"
#include "godot/Gstream.hpp"
#include "world/Chunk.h"
#include "block/BlockDatabase.h"

void SurfaceOptimization::_ready(){
    gout << BlockDatabase::getSingleton()->getBlock(0)->checkFlag(BlockData::TRANSPARENT) << endl;
    gout << BlockDatabase::getSingleton()->getBlock(1)->checkFlag(BlockData::TRANSPARENT) << endl;
    {
        Chunk c;
        c.iteraterate(BLOCK_LEVEL, [](VoxelInstance* v, int) {
            //gout << v->center << endl;
            if(v->center.y > 0)
                v->blockData = BlockDatabase::getSingleton()->getBlock(1);
            else
                v->blockData = BlockDatabase::getSingleton()->getBlock(0);
        });
        c.prune();
        c.recalculate();

        std::ofstream os("test.chunk.json", std::ios::binary);
        //cereal::PortableBinaryOutputArchive test(os);
        cereal::JSONOutputArchive save(os);
        save(c);
    }

    Surface surf;
    Chunk c(Chunk::DONT_INTIALIZE);
    {
        std::ifstream is("test.chunk.json", std::ios::binary);
        //cereal::PortableBinaryInputArchive test(is);
        cereal::JSONInputArchive load(is);
        load(c);

        std::vector<Face> facesArr;
        c.iteraterate(1, [&facesArr](VoxelInstance* me, int) {
            me->getFaces(facesArr);
        });

        if(facesArr.size())
            for (Face& f: facesArr)
                surf.append(f.getSurface());

        gout << facesArr.size() << " faces" << endl;
    }

    gout << c.checkFlag(Chunk::TOP_VISIBLE) << endl;
    gout << c.blockData->checkFlag(BlockData::INVISIBLE) << " - " << c.blockData->flags << endl;

    //visualizeEdges(surf, surf.norms[0]);
    this->set_mesh(surf.getMesh());
}

void SurfaceOptimization::visualizeEdges(Surface surface, Vector3 normal){
	for(int i = 0; i < surface.indecies.size(); i += 3){
		SurfaceTool* line = SurfaceTool::_new();
		normal = normal.normalized() / 10000;
		line->begin(Mesh::PRIMITIVE_LINES);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i]] + normal);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i + 1]] + normal);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i + 1]] + normal);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i + 2]] + normal);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i + 2]] + normal);
		line->add_vertex(surface.verts.read()[surface.indecies.read()[i]] + normal);
        MeshInstance* instance = MeshInstance::_new();
		instance->set_mesh(line->commit());
		this->add_child(instance);
    }
}
