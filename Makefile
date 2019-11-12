CC64=g++ -fPIC -g -std=c++14 -pthread -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib

LIBRARIES =

OBJ = src/godot/gdlink.o src/SurfaceOptimization.o src/SurfFaceEdge.o src/world/Chunk.o src/world/ChunkMap.o src/block/BlockDatabase.o src/block/BlockFeatureDatabase.o

%.o: %.cpp
	$(CC64) -g -c -o $@ $< -std=c++14 -pthread

build:  $(OBJ)
	g++ -fPIC -g -pthread -o bin/lib_GameCode.so -std=c++14 -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.64.a

clean:
	rm $(OBJ)

remake: clean build

run:
	echo "Built sucessfully"
	godot

src/world/Chunk.o : src/world/Chunk.h src/SurfFaceEdge.h
src/godot/gdlink.o: src/world/Chunk.h src/SurfaceOptimization.h
src/SurfaceOptimization.o: src/world/Chunk.h src/SurfFaceEdge.h
src/SurfFaceEdge.o: src/SurfFaceEdge.h
