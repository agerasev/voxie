#pragma once

#include <gl/vertexbuffer.hpp>

class Cube {
public:
	gl::VertexBuffer cube_vertex_buffer, cube_normal_buffer;
	gl::VertexBuffer quad_vertex_buffer, quad_normal_buffer;
	Cube();
	~Cube() = default;
};
