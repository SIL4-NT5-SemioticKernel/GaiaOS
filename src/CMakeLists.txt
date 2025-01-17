# CMakeList.txt : CMake project for NT4, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("NT4")

# Add source to this project's executable.
add_executable (NT4 "NT4.cpp" "NT4.h")

# TODO: Add tests and install targets if needed.
target_include_directories(NT4 PUBLIC "include")

# Set compiler options for profiling
target_compile_options(NT4 PUBLIC "-pg")

# TODO: Add tests and install targets if needed.
target_include_directories(NT4 PUBLIC "include")