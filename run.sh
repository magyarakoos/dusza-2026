#!/bin/bash
set -e
mkdir -p build

for src in src/*.cpp; do
    obj="build/$(basename "${src%.cpp}").o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "Compiling $src"
        g++ -std=c++17 -Iinclude -c "$src" -o "$obj"
    fi
done

g++ build/*.o -o build/app -pthread -DUNICODE -DNOMINMAX
./build/app
