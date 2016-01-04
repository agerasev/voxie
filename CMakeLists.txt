cmake_minimum_required(VERSION 2.6)
project(voxie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -fpermissive")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

set(SOURCES
	sources/main.cpp
)

include_directories(submodules/libla)
include_directories(submodules/libgl4u)

if(${WIN32})
	include_directories(${SDL2_INCLUDE_DIR})
	include_directories(${GLEW_INCLUDE_DIR})
	link_directories(${SDL2_LINK_DIR})
	link_directories(${GLEW_LINK_DIR})
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

if(${WIN32})
	target_link_libraries(${PROJECT_NAME} mingw32 SDL2main SDL2 opengl32 glew32)
else()
	target_link_libraries(${PROJECT_NAME} SDL2 GL GLEW)
endif()
