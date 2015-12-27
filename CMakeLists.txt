cmake_minimum_required(VERSION 2.6)
project(voxie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

set(SOURCES
	sources/main.cpp
)

include_directories(submodules/libla)
include_directories(submodules/libgl4u)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} SDL2 GL GLEW)
