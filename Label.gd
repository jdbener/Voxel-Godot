extends Label

func _process(delta):
	var fps = Engine.get_frames_per_second()
	if(fps > 1000):
		add_color_override("font_color", Color("#567d46"))
	elif(fps > 100):
		add_color_override("font_color", Color("#ffffed"))
	else:
		add_color_override("font_color", Color("#8e1600"))
	text = str(fps)