#!/bin/bash

# Set build directory
BUILD_DIR=build

# Create and navigate to build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Run CMake to configure the project
cmake ..

# Build the project
cmake --build .

# Navigate back
cd ..

echo "Build completed! Executable is located in the build directory."
