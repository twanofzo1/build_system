#!/bin/bash

# Recursively find all .cpp files in src directory
SRC_DIR="src"
CPP_FILES=$(find "$SRC_DIR" -type f -name '*.cpp')

# Output executable name
if [ ! -d "bin" ]; then
    mkdir bin
fi
OUTPUT="bin/tmake"

# Compiler and flags
CXX=g++
CXXFLAGS="-std=c++17 -Wall -Wextra -O2"

# Compile all .cpp files
$CXX $CXXFLAGS $CPP_FILES -o $OUTPUT

if [ $? -eq 0 ]; then
    echo "Build succeeded. Output: $OUTPUT"
else
    echo "Build failed."
    exit 1
fi
