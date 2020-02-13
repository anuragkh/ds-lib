CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(succinct-ds CXX)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
FILE(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})

SET(INCLUDE include/)
INCLUDE_DIRECTORIES(${INCLUDE})

find_package(Threads REQUIRED)

ADD_SUBDIRECTORY(bench)

# Testing framework
# Build google testing framework
add_subdirectory(external/gtest-1.7.0)

# Build and run all tests for succinct
add_subdirectory(test)

enable_testing()
add_test(DSTest ${TESTS_PATH}/ds_test)
