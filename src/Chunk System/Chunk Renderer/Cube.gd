extends Object

enum faces{none = 0, top = 1, bottom = 2, north = 4, south = 8, east = 16, west = 32, all = 63}
var planeUVs = PoolVector2Array([Vector2(0, 1), Vector2(1, 1), Vector2(1, 0), Vector2(0, 0)])
var planeIndicies = PoolIntArray([0, 1, 2, 2, 3, 0])

func reverse(old):
	var new = old
	new.resize(old.size())
	for x in range(0, old.size()):
		new[x] = old[old.size() - 1 - x]
	return new

func offset(array, amount):
	var new = []
	for x in array:
		new.push_back(x + amount)
	return new
	
func repeat(what, times):
	var new = []
# warning-ignore:unused_variable
	for x in range(0, times):
		new.push_back(what)
	return new
	
func scaleUVs(scale):
	var new = []
	for x in planeUVs:
		new.push_back(x * scale)
	return new
	
func positionVerts(what, scale, center):
	var new = []
	for x in what:
		new.push_back(x * scale + center)
	return new

func makeFaceVerts(face):
	match face:
		faces.top:
			return reverse(PoolVector3Array([Vector3(-1, 1, 1), Vector3(1, 1, 1), Vector3(1, 1, -1), Vector3(-1, 1, -1)]))
		faces.bottom:
			return PoolVector3Array([Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(1, -1, -1), Vector3(-1, -1, -1)])
		faces.north:
			return PoolVector3Array([Vector3(1, -1, 1), Vector3(1, 1, 1), Vector3(1, 1, -1), Vector3(1, -1, -1)])
		faces.south:
			return reverse(PoolVector3Array([Vector3(-1, -1, 1), Vector3(-1, 1, 1), Vector3(-1, 1, -1), Vector3(-1, -1, -1)]))
		faces.east:
			return PoolVector3Array([Vector3(-1, 1, 1), Vector3(1, 1, 1), Vector3(1, -1, 1), Vector3(-1, -1, 1)])
		faces.west:
			return reverse(PoolVector3Array([Vector3(-1, 1, -1), Vector3(1, 1, -1), Vector3(1, -1, -1), Vector3(-1, -1, -1)]))
		

func makeFaces(mask, scale = 1, center = Vector3()):
	var out = PoolVector3Array()
	if(mask & faces.top):
		out.append_array(makeFaceVerts(faces.top))
	if(mask & faces.bottom):
		out.append_array(makeFaceVerts(faces.bottom))
	if(mask & faces.north):
		out.append_array(makeFaceVerts(faces.north))
	if(mask & faces.south):
		out.append_array(makeFaceVerts(faces.south))
	if(mask & faces.east):
		out.append_array(makeFaceVerts(faces.east))
	if(mask & faces.west):
		out.append_array(makeFaceVerts(faces.west))
	return positionVerts(out, scale, center)

func makeNormals(mask):
	var out = PoolVector3Array()
	if(mask & faces.top):
		out.append_array(repeat(Vector3(0, 1, 0), 4))
	if(mask & faces.bottom):
		out.append_array(repeat(Vector3(0, -1, 0), 4))
	if(mask & faces.north):
		out.append_array(repeat(Vector3(0, 0, -1), 4))
	if(mask & faces.south):
		out.append_array(repeat(Vector3(1, 0, 1), 4))
	if(mask & faces.east):
		out.append_array(repeat(Vector3(-1, 0, 0), 4))
	if(mask & faces.west):
		out.append_array(repeat(Vector3(1, 0, 0), 4))
	return out

func makeIndicies(faces):
	var out = PoolIntArray()
	for x in range(0, faces):
		out.append_array(offset(planeIndicies, x * 4))
	return out

func makeUVs(faces, scale = 1):
	var out = PoolVector2Array()
# warning-ignore:unused_variable
	for x in range(0, faces):
		out.append_array(scaleUVs(scale))
	return out
	
func makeCube(mask, scale = 1, center = Vector3(0, 0, 0), uvScale = 1):
	var out = []
	out.resize(ArrayMesh.ARRAY_MAX)
	out[ArrayMesh.ARRAY_VERTEX] = makeFaces(mask, scale, center)
	out[ArrayMesh.ARRAY_NORMAL] = makeNormals(mask)
	var faces = out[ArrayMesh.ARRAY_VERTEX].size() / 4
	out[ArrayMesh.ARRAY_TEX_UV] = makeUVs(faces, uvScale)
	out[ArrayMesh.ARRAY_INDEX] = makeIndicies(faces)
	return out