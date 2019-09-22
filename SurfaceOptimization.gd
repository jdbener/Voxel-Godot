extends MeshInstance

enum FaceType {TRIANGLE, QUAD}
class Face:
	var type
	var a : Vector3
	var b : Vector3
	var c : Vector3
	var normal : Vector3
	
	func _init(_a, _b, _c):
		type = FaceType.TRIANGLE
		a = _a
		b = _b
		c = _c
		# Calculate the surface normal
		normal = (c - a).cross(b - a).normalized()
	
class QuadFace extends Face:
	var d : Vector3
	
	func _init(_a, _b, _c, _d).(_a, _b, _c):
		type = FaceType.QUAD
		d = _d
		
	func reverse():
		var temp = d
		d = a
		a = temp
		temp = b
		b = c
		c = temp
		normal = (b - a).cross(c - a).normalized()

class Edge:
	var tail : Vector3
	var tip : Vector3
	
	func _init(_a, _b):
		tail = _a
		tip = _b
		
	func direction():
		return tip - tail
		
	func onEdge(p):
		if tail.x <= max(p.x, tip.x) and tail.x >= min(p.x, tip.x) and tail.y <= max(p.y, tip.y) and tail.y >= min(p.y, tip.y):
			return true
		return false
		
	func overlap(other):
		# If the edges are parralel
		print(direction().normalized(), " - ", other.direction().normalized())
		if direction().normalized() == other.direction().normalized() or direction().normalized() == -other.direction().normalized():
			var connection = Edge.new(other.tail, tail)
			# If the edges are colinear
			if direction().normalized() == connection.direction().normalized() or direction().normalized() == -connection.direction().normalized():
				# If the edges overlap
				if onEdge(other.tip) or onEdge(other.tail) or other.onEdge(tip) or other.onEdge(tail):
					return true
		return false
	
	#TODO: make edges equal if either is a subset of the other
	func equal(other):
		if (tail == other.tail and tip == other.tip) or (tail == other.tip and tip == other.tail):
			return true
		return false
	
	func oneEqual(other):
		if tail == other.tail or tail == other.tip or tip == other.tail or tip == other.tip:
			return true
		return false
		
	func visualize():
		var line = ImmediateGeometry.new()
		line.begin(Mesh.PRIMITIVE_LINES)
		line.add_vertex(tail)
		line.add_vertex(tip)
		line.end()
		return line
		
class Surface:
	# Positions
	var verts = PoolVector3Array()
	# Normals
	var norms = PoolVector3Array()
	# Indecies
	var indecies = PoolIntArray()
	
	func _init():
		verts.resize(0)
		norms.resize(0)
		indecies.resize(0)
		
	func append(other):
		var maxIndex = verts.size()
		
		for vert in other.verts:
			verts.push_back(vert)
		for normal in other.norms:
			norms.push_back(normal)
		for index in other.indecies:
			indecies.push_back(index + maxIndex)
		
	func verts_push_back(what):
		verts.push_back(what)
		
	func norms_push_back(what):
		norms.push_back(what)
	
	func indecies_push_back(what):
		indecies.push_back(what)
	
	func getArrayMesh(mesh = ArrayMesh.new()):
		var arrays = []
		arrays.resize(ArrayMesh.ARRAY_MAX)
		
		arrays[ArrayMesh.ARRAY_VERTEX] = verts
		if norms.size() > 0:
			arrays[ArrayMesh.ARRAY_NORMAL] = norms
		if indecies.size() > 0:
			arrays[ArrayMesh.ARRAY_INDEX] = indecies
		
		mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
		return mesh
		
func outlineEdgesFromFace(face):
	var out = []
	if face.type == FaceType.TRIANGLE:
		out.push_back(Edge.new(face.a, face.b))
		out.push_back(Edge.new(face.b, face.c))
		out.push_back(Edge.new(face.c, face.a))
	elif face.type == FaceType.QUAD:
		out.push_back(Edge.new(face.a, face.b))
		out.push_back(Edge.new(face.b, face.c))
		out.push_back(Edge.new(face.c, face.d))
		out.push_back(Edge.new(face.d, face.a))
	return out
		
func vertsFromFace(face):
	var verts = PoolVector3Array()
	if face.type == FaceType.TRIANGLE:
		verts.push_back(face.a)
		verts.push_back(face.b)
		verts.push_back(face.c)
	elif face.type == FaceType.QUAD:
		verts.push_back(face.a)
		verts.push_back(face.b)
		verts.push_back(face.c)
		verts.push_back(face.d)
	

func surfaceFromFace(face):
	var surf = Surface.new()
	if face.type == FaceType.TRIANGLE:
		surf.verts_push_back(face.a)
		surf.norms_push_back(face.normal)
		surf.indecies_push_back(0)
		surf.verts_push_back(face.b)
		surf.norms_push_back(face.normal)
		surf.indecies_push_back(1)
		surf.verts_push_back(face.c)
		surf.norms_push_back(face.normal)
		surf.indecies_push_back(2)
	elif face.type == FaceType.QUAD:
		surf.verts_push_back(face.a)
		surf.norms_push_back(face.normal)
		surf.verts_push_back(face.b)
		surf.norms_push_back(face.normal)
		surf.verts_push_back(face.c)
		surf.norms_push_back(face.normal)
		surf.verts_push_back(face.d)
		surf.norms_push_back(face.normal)
		
		surf.indecies_push_back(0)
		surf.indecies_push_back(1)
		surf.indecies_push_back(2)
		surf.indecies_push_back(2)
		surf.indecies_push_back(3)
		surf.indecies_push_back(0)
	return surf

func findEdges(edges, what):
	var occurences = 0
	for i in range(0, edges.size()):
		if edges[i].equal(what):
			occurences += 1
	return occurences
		
#Assumes provided faces are contiguous
func surfaceFromFaces(faces):
	var surf = Surface.new()
	
	# Make list of edges
	var edges = []
	for face in faces:
		for edge in outlineEdgesFromFace(face):
			edges.append(edge)
	
	# Remove interior Edges
	var borderEdges = []
	for edge in edges:
		# An edge is a border edge only if it occures once (no other faces share the egde)
		if findEdges(edges, edge) == 1:
			borderEdges.push_back(edge)
	
	# Sort edges
	var sortedBorderEdges = []
	sortedBorderEdges.resize(borderEdges.size())
	sortedBorderEdges[0] = borderEdges[0]
	borderEdges.remove(0)
	for i in range (1, sortedBorderEdges.size()):
		for j in range(0, borderEdges.size()):
			if borderEdges[j].oneEqual(sortedBorderEdges[i - 1]) and !borderEdges[j].equal(sortedBorderEdges[i - 1]):
				sortedBorderEdges[i] = borderEdges[j]
				borderEdges.remove(j)
				break
	
	# Extract verticies, removing those breaking straight edges
	for i in range(0, sortedBorderEdges.size()):
		var keep = false
		# Keep the edge if the vertex of the previous edge is not colinear to this edge
		if i > 0:
			if sortedBorderEdges[i - 1].direction().normalized() != sortedBorderEdges[i].direction().normalized():
				keep = true
		# For the first vertex, we assume a closed loop so the previous edge is the last edge
		else:
			if sortedBorderEdges[sortedBorderEdges.size() - 1].direction().normalized() != sortedBorderEdges[i].direction().normalized():
				keep = true
		if keep:
			surf.verts_push_back(sortedBorderEdges[i].tail)
			surf.norms_push_back(faces[0].normal)
		
	# Flatten all of the vertecies in 3D space and drop their normal facing component
	# This is required since the built in triangulation algorithm only works in 2D space
	var transform = Basis(faces[0].b - faces[0].a, faces[0].normal, faces[0].c - faces[0].a)
	var triangulationPoints = PoolVector2Array()
	for p in surf.verts:
		var tmp = transform.xform_inv(p)
		triangulationPoints.push_back(Vector2(tmp.x, tmp.z))
		
	# Triangulate the surface based on the reprojected points
	surf.indecies = Geometry.triangulate_polygon(triangulationPoints)
	return surf
	
func makePlane(center, axis, radius):
	var perpendicular = Vector3(axis.z, axis.x, axis.y) 
	var basis = Basis(perpendicular, axis, axis.cross(perpendicular))
	var a = center + basis.xform(Vector3(radius, 0, -radius))
	var b = center + basis.xform(Vector3(radius, 0, radius))
	var c = center + basis.xform(Vector3(-radius, 0, radius))
	var d = center + basis.xform(Vector3(-radius, 0, -radius))
	return QuadFace.new(a, b, c, d)
	
# Called when the node enters the scene tree for the first time.
enum SurfaceDisplay{WIREFRAME = 1, SOLID = 2, BOTH = 3}
func _ready():
	var face = Face.new(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1))
	face = QuadFace.new(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 1), Vector3(-1, 0, -1))
	var face2 = QuadFace.new(Vector3(3, 0, -1), Vector3(3, 0, 1), Vector3(1, 0, 1), Vector3(1, 0, -1))
	var face3 = QuadFace.new(Vector3(3, 0, 1), Vector3(3, 0, 3), Vector3(1, 0, 3), Vector3(1, 0, 1))
	var face4 = QuadFace.new(Vector3(1, 0, 1), Vector3(1, 0, 3), Vector3(-1, 0, 3), Vector3(-1, 0, 1))
	var face5 = QuadFace.new(Vector3(1, 0, 3), Vector3(1, 0, 5), Vector3(-1, 0, 5), Vector3(-1, 0, 3))
	var surf = surfaceFromFace(face)
	surf.append(surfaceFromFace(face2))
	surf.append(surfaceFromFace(face3))
	surf.append(surfaceFromFace(face4))
	surf.append(surfaceFromFace(face5))
	
	surf = surfaceFromFaces([face, face2, face3, face4, face5])
	
	var wireframe = SurfaceDisplay.BOTH
	if wireframe & 1 > 0:
		visualizeEdges(surf, surf.norms[0])
	if wireframe & 2 > 0:
		mesh = surf.getArrayMesh()
	
func visualizeEdges(surface, normal = Vector3.ZERO):
	for i in range(0, surface.indecies.size(), 3):
		var line = ImmediateGeometry.new()
		normal = normal.normalized() / 1000
		line.begin(Mesh.PRIMITIVE_LINES)
		line.set_color(Color.black)
		line.add_vertex(surface.verts[surface.indecies[i]] + normal)
		line.add_vertex(surface.verts[surface.indecies[i + 1]] + normal)
		line.add_vertex(surface.verts[surface.indecies[i + 1]] + normal)
		line.add_vertex(surface.verts[surface.indecies[i + 2]] + normal)
		line.add_vertex(surface.verts[surface.indecies[i + 2]] + normal)
		line.add_vertex(surface.verts[surface.indecies[i]] + normal)
		line.end()
		add_child(line)

func visualizePoints(points, scale = .025):
	for point in points:
		var sphere = MeshInstance.new()
		sphere.mesh = SphereMesh.new()
		sphere.translate(point)
		sphere.scale_object_local(Vector3(scale, scale, scale))
		add_child(sphere)