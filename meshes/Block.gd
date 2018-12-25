extends MeshInstance

export (Material) var material
func _enter_tree():
	set_surface_material(0, material)
