extends MeshInstance

func _ready():
	var plane = SurfaceTool.new()
	plane.begin(Mesh.PRIMITIVE_TRIANGLES)
	plane.set_material(load("res://materials/stone.tres"))
	
	#1
	plane.add_uv(Vector2(1, 1));
	plane.add_vertex(Vector3(.5, .5, .5));

	#2
	plane.add_uv(Vector2(1, 0));
	plane.add_vertex(Vector3(.5, -.5, .5));

	#3
	plane.add_uv(Vector2(0, 0));
	plane.add_vertex(Vector3(.5, -.5, -.5));
	
	#3
	plane.add_uv(Vector2(0, 0));
	plane.add_vertex(Vector3(.5, -.5, -.5));

	#4
	plane.add_uv(Vector2(0, 1));
	plane.add_vertex(Vector3(.5, .5, -.5));
	
	#1
	plane.add_uv(Vector2(1, 1));
	plane.add_vertex(Vector3(.5, .5, .5));
	
	
	
	
	
	plane.index()
	if(self.mesh):
		plane.commit(self.mesh)
	else:
		self.mesh = plane.commit()