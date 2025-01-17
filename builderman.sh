#!/bin/bash
cd src

# Create build directory if not exists
mkdir -p build

# Move into build directory
cd build || exit

# Run cmake to generate build files
cmake ..

# Build the project
cmake --build .

# Move back to parent directory
cd ..

# Copy the built executable to bin directory
cp build/debug/NT4.exe bin/NT4.exe

# Pause the script
read -rsp $'Press any key to continue...\n' -n1 key