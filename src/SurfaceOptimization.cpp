#include "SurfaceOptimization.h"

#include "Geometry.hpp"
#include "SurfaceTool.hpp"

#include "timer.h"

#include <vector>
#include <iostream>

void SurfaceOptimization::_ready(){
    Face faces[] = { Face(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1)),
        Face(Vector3(3, 0, -1), Vector3(3, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, -1)),
        Face(Vector3(3, 0, 1), Vector3(3, 0, 3), Vector3(1, 0, 3), Vector3(1, 0, 1)),
        Face(Vector3(1, 0, 1), Vector3(1, 0, 3), Vector3(-1, 0, 3), Vector3(-1, 0, 1)),
        Face(Vector3(1, 0, 3), Vector3(1, 0, 5), Vector3(-1, 0, 5), Vector3(-1, 0, 3))
    };
    // Swap Triangle face for Quad face
    faces[0] = Face(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1), Vector3(-1, 0, -1));

    // Nievely construct surface from faces
    Surface surf = faces[0].getSurface();
    surf.append(faces[1].getSurface());
    surf.append(faces[2].getSurface());
    surf.append(faces[3].getSurface());
    surf.append(faces[4].getSurface());

    Godot::print(faces[0].checkContiguiousCoplanar(faces[1]) ? "true" : "false");
    Godot::print(faces[4].checkContiguiousCoplanar(faces[1]) ? "true" : "false");

    surf = Surface::fromContiguousCoplanarFaces(std::vector<Face>(faces, faces + sizeof(faces)/sizeof(faces[0])));

    visualizeEdges(surf, surf.norms[0]);
    this->set_mesh(surf.getMesh());
}

void SurfaceOptimization::visualizeEdges(Surface surface, Vector3 normal){
    Timer t;
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
