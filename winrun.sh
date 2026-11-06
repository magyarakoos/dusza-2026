#!/bin/bash

cmake -S . -B build
cmake --build build
./build/Debug/my_app.exe
