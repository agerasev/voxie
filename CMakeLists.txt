cmake_minimum_required(VERSION 2.6)
project(voxie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -fpermissive")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

set(SOURCES
	#sources/main.cpp
)

include_directories(submodules/libla)
include_directories(submodules/libgl4u)

add_executable(editor sources/editor_main.cpp ${SOURCES})
target_link_libraries(editor SDL2 GL GLEW)
