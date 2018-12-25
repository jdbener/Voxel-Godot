extends Spatial

# Declare member variables here. Examples:
# var a = 2
# var b = "text"

# Called when the node enters the scene tree for the first time.
func _ready():
	#var transform = Transform(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0)).translated(Vector3(0, .5, 0)).rotated(Vector3(0, 1, 0), 180)
	var up = Transform(Basis(Vector3(0, 1, 0), deg2rad(180)), Vector3(0, .5, 0))
	var down = Transform(Basis(Vector3(0, 0, -1), deg2rad(180)), Vector3(0, 0, -.5))
	#var left = Transform(Basis(Vector3(1, -1, 0), deg2rad(90)), Vector3(-.5, 0, 0))
	var left = Transform(Basis(Vector3(1, 0, 0), deg2rad(90))).rotated(Vector3(0, -1, 0), deg2rad(90)).translated(Vector3(-.5, 0, 0))
	#var right = Transform(Basis(Vector3(1, 1, 0), deg2rad(90)), Vector3(.5, 0, 0))
	var front = Transform(Basis(Vector3(1, 0, 0), deg2rad(90)), Vector3(0, 0, .5))
	#var back = Transform(Basis(Vector3(1, 2, 0), deg2rad(90)), Vector3(0, 0, -.5))
	
	var upTrans = Vector3(0, .5, 0)
	var upRot = Vector3(0, 180, 0)
	var downTrans = Vector3(0, -.5, 0)
	var downRot = Vector3(0, 0, -180)
	var leftTrans = Vector3(-.5, 0, 0)
	var leftRot = Vector3(90, -90, 0)
	var rightTrans = Vector3(.5, 0, 0)
	var rightRot = Vector3(90, 90, 0)
	var frontTrans = Vector3(0, 0, 0.5)
	var frontRot = Vector3(90, 0, 0)
	var backTrans = Vector3(0, 0, -.5)
	var backRot = Vector3(90, 180, 0)
	#var transform = get_node("Chunk/Up").transform
	print(get_node("Chunk/Left").transform, "=", left)
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
