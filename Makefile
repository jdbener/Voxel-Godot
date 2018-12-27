CC32=g++ -m32 -g -O3 -std=c++14 -pthread -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib
CC64=g++ -fPIC -g -O3 -std=c++14 -pthread -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib

STRICT=-z defs

LIBRARIES = lib/SimplexNoise/build/SimplexNoise

OBJ32 = src/init.32 src/ChunkMap.32 src/BlockList.32 src/world/Block.32 src/world/Chunk.32 src/ChunkRenderer.32
OBJ64 = src/init.64 src/ChunkMap.64 src/BlockList.64 src/world/Block.64 src/world/Chunk.64 src/ChunkRenderer.64

%.32: %.cpp
	$(CC32) -c -o $@ $< -std=c++14 -pthread

%.64: %.cpp
	$(CC64) -c -o $@ $< -std=c++14 -pthread

build64:  $(OBJ64)
	g++ -fPIC -pthread -o bin/GameCode.so -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.64.a $(LIBRARIES) $(STRICT)

build32: $(OBJ32)
	g++ -fPIC -m32 -pthread -o bin/GameCode.32.so -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.32.a $(LIBRARIES) $(STRICT)

build: build64 build32
	echo "Done!"

clean:
	rm $(OBJ32) $(OBJ64) bin/GameCode.so bin/GameCode.32.so

clean-chunks:
	rm worlds/Test/*

run:
	echo "Built Successfully"
