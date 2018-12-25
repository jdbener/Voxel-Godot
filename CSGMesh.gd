extends CSGMesh

func _ready():
	mesh.surface_set_material(0, load("res://materials/stone.tres"))
	mesh.surface_set_material(1, load("res://materials/stone.tres"))
	mesh.surface_set_material(2, load("res://materials/stone.tres"))
	mesh.surface_set_material(3, load("res://materials/stone.tres"))
	mesh.surface_set_material(4, load("res://materials/stone.tres"))
	mesh.surface_set_material(5, load("res://materials/stone.tres"))
	print(self.mesh.surface_get_material(0))
	pass

