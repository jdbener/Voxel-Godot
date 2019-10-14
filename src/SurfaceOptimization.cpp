#include "SurfaceOptimization.h"

#include "Geometry.hpp"
#include "SurfaceTool.hpp"

#include "gstream/Gstream.hpp"
#include <iomanip>

#include <vector>

#include "timer.h"

void SurfaceOptimization::_ready(){
    Face faces[] = { Face(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1)),
        Face(Vector3(3, 0, -1), Vector3(3, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, -1)),
        Face(Vector3(3, 0, 1), Vector3(3, 0, 3), Vector3(1, 0, 3), Vector3(1, 0, 1)),
        Face(Vector3(1, 0, 1), Vector3(1, 0, 3), Vector3(-1, 0, 3), Vector3(-1, 0, 1)),
        Face(Vector3(1, 0, 3), Vector3(1, 0, 5), Vector3(-1, 0, 5), Vector3(-1, 0, 3)),
        Face(Vector3(7, 0, 3), Vector3(7, 0, 5), Vector3(5, 0, 5), Vector3(5, 0, 3), 2),
        Face(Vector3(5, 0, 3), Vector3(5, 0, 5), Vector3(3, 0, 5), Vector3(3, 0, 3), 2),
        Face(Vector3(3, 0, 3), Vector3(3, 0, 5), Vector3(1, 0, 5), Vector3(1, 0, 3), 2),

    };
    // Swap Triangle face for Quad face
    faces[0] = Face(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1), Vector3(-1, 0, -1));

    // Nievely construct surface from faces
    Surface surf;
    {
        Timer t;
        surf.append(faces[0].getSurface());
        surf.append(faces[1].getSurface());
        surf.append(faces[2].getSurface());
        surf.append(faces[3].getSurface());
        surf.append(faces[4].getSurface());
        surf.append(faces[5].getSurface());
        surf.append(faces[6].getSurface());
        surf.append(faces[7].getSurface());
    }

    gout << (faces[0].checkContiguiousCoplanar(faces[1]) ? "true" : "false") << std::endl
        << (faces[4].checkContiguiousCoplanar(faces[1]) ? "true" : "false") << std::endl;

    //surf = Surface::fromContiguousCoplanarFaces(std::vector<Face>(faces, faces + sizeof(faces)/sizeof(faces[0])));
    {
        Timer t;
        surf = Surface::fromFaces(std::vector<Face>(faces, faces + sizeof(faces)/sizeof(faces[0])));
    }

    visualizeEdges(surf, surf.norms[0]);
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
