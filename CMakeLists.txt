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
# add_executable(scratch ${SOURCES})
add_library(equaio ${SOURCES})

# =========================== TEST 

include(CTest)
enable_testing()

file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(equaio_test ${TEST_SOURCES})
target_link_libraries(equaio_test equaio)
add_test(NAME Libequaio_test COMMAND equaio_test)
