cmake_minimum_required(VERSION 3.0)
project(LibEquaio)

# set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for header files
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Create the executable
add_executable(libequaio ${SOURCES})
