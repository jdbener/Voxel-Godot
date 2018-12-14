extends Spatial

# Variable storing a reference to the root node
onready var root = get_tree().get_root().get_child(0)
# Variable storing a reference to the test cube scene we are instancing
var testCube = preload("Test Cube.tscn")

func _ready():
	# Variable storing a reference to the ChunkMap
	var ChunkMap = root.get_node("ChunkMap");

	# Clalculate the thickest point of the sphere (Based on the C++ returns)
	print("Thickest Point = ", ChunkMap.getIndex(0, 0) - ChunkMap.getIndex(0, -1))

	# For every position in a subset sphere with radius 3
	for pos in ChunkMap.getSphere(3, ChunkMap.origin):
		# Create a new instacnce of the test cube
		var cube = testCube.instance()
		# Position that instance
		cube.translate(pos)
		# Add that instance as a child of the ChunkMap
		ChunkMap.add_child(cube)

	for i in ClassDB.get_class_list():
		print(i)
