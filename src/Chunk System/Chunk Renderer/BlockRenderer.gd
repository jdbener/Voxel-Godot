extends MeshInstance

enum faces{none = 0, top = 1, bottom = 2, north = 4, south = 8, east = 16, west = 32, all = 63}

var cube = preload("res://Cube.gd").new()
var marchingCube = preload("res://MarchingCube.gd").new()

"""
	data starts at +x, +y, +z corner,
	+y = up
	+x = north
	+z = east
"""

func append(first, second):
	var out = []
	for x in first:
		out.push_back(x)
	for x in second:
		out.push_back(x)
	return out
	
func arrayMax(array):
	var _max = 0
	for a in array:
		if a > _max:
			_max = a

	if _max > 0:
		return _max + 1
	return _max
	

func _ready():
	#fill out blockMap for testing purposes
	var blockMap = {}
	for x in range(0, 3):
		for y in range(0, 3):
			for z in range(0, 3): 
				blockMap[Vector3(x, y, z)] = Block.new(Vector3(x, y, z))
	for x in blockMap:
		blockMap[x] = calculateVisibility(blockMap[x], blockMap)
	
	var out = []
	out.resize(ArrayMesh.ARRAY_MAX)
	out[ArrayMesh.ARRAY_VERTEX] = PoolVector3Array()
	out[ArrayMesh.ARRAY_NORMAL] = PoolVector3Array()
	out[ArrayMesh.ARRAY_TEX_UV] = PoolVector2Array()
	out[ArrayMesh.ARRAY_INDEX] = PoolIntArray()
	for x in blockMap:
		var face = marchingCube.makeMarchingCube(blockMap[x].mask, .5, blockMap[x].center)
		out[ArrayMesh.ARRAY_VERTEX] = PoolVector3Array(append(out[ArrayMesh.ARRAY_VERTEX], face[ArrayMesh.ARRAY_VERTEX]))
		out[ArrayMesh.ARRAY_NORMAL] = PoolVector3Array(append(out[ArrayMesh.ARRAY_NORMAL], face[ArrayMesh.ARRAY_NORMAL]))
		out[ArrayMesh.ARRAY_TEX_UV] = PoolVector2Array(append(out[ArrayMesh.ARRAY_TEX_UV], face[ArrayMesh.ARRAY_TEX_UV]))
	out[ArrayMesh.ARRAY_INDEX] = marchingCube.calculateIndex(out[ArrayMesh.ARRAY_VERTEX].size())
	"""for x in blockMap:
		var face = cube.makeCube(blockMap[x].mask, .5, blockMap[x].center)
		out[ArrayMesh.ARRAY_VERTEX] = PoolVector3Array(append(out[ArrayMesh.ARRAY_VERTEX], face[ArrayMesh.ARRAY_VERTEX]))
		out[ArrayMesh.ARRAY_NORMAL] = PoolVector3Array(append(out[ArrayMesh.ARRAY_NORMAL], face[ArrayMesh.ARRAY_NORMAL]))
		out[ArrayMesh.ARRAY_TEX_UV] = PoolVector2Array(append(out[ArrayMesh.ARRAY_TEX_UV], face[ArrayMesh.ARRAY_TEX_UV]))
		out[ArrayMesh.ARRAY_INDEX] = PoolIntArray(append(out[ArrayMesh.ARRAY_INDEX], cube.offset(face[ArrayMesh.ARRAY_INDEX], arrayMax(out[ArrayMesh.ARRAY_INDEX]))))"""
	
	var arr_mesh = ArrayMesh.new()
	arr_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, out)
	mesh = arr_mesh
	
	var points = [Vector3(0.5, 0, 1.5), Vector3(0.5, 0, 0.5), Vector3(0, 0.5, 0.5)]
	#visualizePoints(points)
	#visualizePoints(marchingCube.calculateUVs(points, marchingCube.calculateNormals(points)), .025)

func visualizePoints(points, scale = .05):
	for pos in points:
		var mi = MeshInstance.new()
		mi.mesh = SphereMesh.new()
		mi.translate(pos)
		mi.scale_object_local(Vector3(scale, scale, scale))
		add_child(mi)

func visualizeIndex(index, corners):
	var verts = []
	if index & 1:
		verts.push_back(corners[0])
	if index & 2:
		verts.push_back(corners[1])
	if index & 4:
		verts.push_back(corners[2])
	if index & 8:
		verts.push_back(corners[3])
	if index & 16:
		verts.push_back(corners[4])
	if index & 32:
		verts.push_back(corners[5])
	if index & 64:
		verts.push_back(corners[6])
	if index & 128:
		verts.push_back(corners[7])
	visualizePoints(verts)
	
func calculateVisibility(block : Block, blockMap):
	block.mask = 0
	if !(blockMap.has(block.center + Vector3(0, 1, 0))):
		block.mask |= faces.top
	if !(blockMap.has(block.center + Vector3(0, -1, 0))):
		block.mask |= faces.bottom
	if !(blockMap.has(block.center + Vector3(1, 0, 0))):
		block.mask |= faces.north
	if !(blockMap.has(block.center + Vector3(-1, 0, 0))):
		block.mask |= faces.south
	if !(blockMap.has(block.center + Vector3(0, 0, 1))):
		block.mask |= faces.east
	if !(blockMap.has(block.center + Vector3(0, 0, -1))):
		block.mask |= faces.west
	return block

class Block:
	var mask : int
	var center: Vector3
	
	func _init(cen):
		center = cen