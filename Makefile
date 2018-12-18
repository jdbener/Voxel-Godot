CC32=g++ -m32 -g -O3 -std=c++14 -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib
CC64=g++ -fPIC -g -O3 -std=c++14 -Igodot-cpp/include -Igodot-cpp/include/core -Igodot-cpp/include/gen -Igodot-cpp/godot_headers -Ilib

OBJ32 = src/init.32 src/ChunkMap.32
OBJ64 = src/init.64 src/ChunkMap.64

%.32: %.cpp
	$(CC64) -c -o $@ $<

%.64: %.cpp
	$(CC64) -c -o $@ $<

build64:  $(OBJ64)
	g++ -fPIC -o bin/GameCode.so -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.64.a

build32: $(OBJ32)
	g++ -fPIC -m32 -o bin/GameCode.32.so -shared $^ godot-cpp/bin/libgodot-cpp.linux.debug.32.a

build: build64 build32
	echo "Done!"

run:
	echo "Built Successfully"
