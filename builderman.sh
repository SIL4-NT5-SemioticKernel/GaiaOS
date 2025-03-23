#!/bin/bash

cd src || exit

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit

# Generate build files and build the project
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Move back to parent directory
cd ..

# Create bin directory if it doesn't exist
mkdir -p ../bin

# Copy based on OS (no .exe needed on Linux/macOS)
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "win32" ]]; then
    cp build/debug/NT4.exe ../bin/NT4.exe
else
    cp build/NT4 ../bin/NT4
fi

echo "Build complete."