CC64=g++ -fPIC -g -O3 -std=c++14 -pthread -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib

LIBRARIES =

OBJ = src/gdlink.o src/SurfaceOptimization.o src/SurfFaceEdges.o

%.o: %.cpp
	$(CC64) -c -o $@ $< -std=c++14 -pthread

build:  $(OBJ)
	g++ -fPIC -g -pthread -o bin/GameCode.so -std=c++14 -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.64.a

clean:
	rm $(OBJ)

run:
	echo "Built sucessfully"
