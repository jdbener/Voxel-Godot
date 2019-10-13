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

    //Godot::print(s(sizeof(Face)));
    //std::vector<int> ints;
    //for(int i = 0; i < 10; i++)
    //    ints.push_back(i);
    //Godot::print(s(ints));
    //std::cout << s(ints).ascii().get_data() << std::endl;

    {
        Timer t;
        gout << "Will this work?\n\n" << 21 << " I like that number" << std::endl;
        gout << 1 << std::endl << 2 << std::endl << 3 << std::endl << 4.5 << std::endl;
        gout << std::setw(50) << std::right << "hello!" << std::endl;

        gout << std::endl << std::endl << "The size of gout is: " << sizeof(gout) << std::endl;
        gout << "The size of cout is: " << sizeof(std::cout) << std::endl;

        String test = "This is a string I am writing! μ";

        gout << test << std::endl;
        gout << surf.verts[0] << std::endl;
        gout << L"μ" << std::endl;
        t.stop(gout);
    }

    surf = Surface::fromContiguousCoplanarFaces(std::vector<Face>(faces, faces + sizeof(faces)/sizeof(faces[0])));

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
