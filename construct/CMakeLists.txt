cmake_minimum_required(VERSION 2.8)
project(ds-lib-construct CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -g")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

file(GLOB SOURCE_FILES src/*.cc)
add_executable(construct ${SOURCE_FILES})
target_link_libraries(construct ds)
