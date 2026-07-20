cmake_minimum_required(VERSION 3.10)
project(file_storage VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(code main.cpp)

# Optimize for performance
target_compile_options(code PRIVATE -O2 -Wall -Wextra)
