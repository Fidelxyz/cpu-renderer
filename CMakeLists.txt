cmake_minimum_required(VERSION 3.16)

project(cpu-renderer VERSION 1.0.0 LANGUAGES C CXX)

# set(CMAKE_C_COMPILER "/usr/bin/gcc")
# set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fsanitize=address,undefined -pthread")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -pthread")

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS})

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${OpenMP_CXX_INCLUDE_DIRS})

include_directories(include)
include_directories(3rdparty/include)

link_directories(3rdparty/lib)

FILE(GLOB_RECURSE CURRENT_INCLUDE include/*.hpp)
FILE(GLOB_RECURSE CURRENT_SOURCES src/*.cpp)
FILE(GLOB_RECURSE THIRDPARTY_INCLUDE 3rdparty/include/*.h)
# aux_source_directory(src CURRENT_SOURCES)

add_executable(cpu-renderer ${CURRENT_SOURCES})

target_link_libraries(cpu-renderer
    ${OpenCV_LIBS}
    ${EIGEN3_LIBS}
    ${OpenMP_CXX_LIBRARIES}
    yaml-cpp
)