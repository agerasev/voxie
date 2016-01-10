#pragma once

#include <gl/vertexbuffer.hpp>
#include <gl/program.hpp>

#include "projector.hpp"
#include "voxelmap.hpp"

#include <vox/objects/camera.hpp>

#define sizeof_array(x) (sizeof(x)/sizeof((x)[0]))

class Cube {
public:
	gl::VertexBuffer cube_vertex_buffer, cube_normal_buffer;
	gl::VertexBuffer quad_vertex_buffer, quad_normal_buffer;
	
	Cube() {
		const float cube_vertex_data[6*2*3*4] = {
		-0.5, -0.5,  0.5, 1,
		 0.5, -0.5,  0.5, 1,
		-0.5,  0.5,  0.5, 1,
		-0.5,  0.5,  0.5, 1,
		 0.5, -0.5,  0.5, 1,
		 0.5,  0.5,  0.5, 1,
		
		-0.5,  0.5, -0.5, 1,
		 0.5,  0.5, -0.5, 1,
		-0.5, -0.5, -0.5, 1,
		-0.5, -0.5, -0.5, 1,
		 0.5,  0.5, -0.5, 1,
		 0.5, -0.5, -0.5, 1,
		
		 0.5, -0.5, -0.5, 1,
		 0.5, -0.5,  0.5, 1,
		 0.5,  0.5, -0.5, 1,
		 0.5,  0.5, -0.5, 1,
		 0.5, -0.5,  0.5, 1,
		 0.5,  0.5,  0.5, 1,
		
		-0.5,  0.5, -0.5, 1,
		-0.5,  0.5,  0.5, 1,
		-0.5, -0.5, -0.5, 1,
		-0.5, -0.5, -0.5, 1,
		-0.5,  0.5,  0.5, 1,
		-0.5, -0.5,  0.5, 1,
		
		-0.5,  0.5, -0.5, 1,
		 0.5,  0.5, -0.5, 1,
		-0.5,  0.5,  0.5, 1,
		-0.5,  0.5,  0.5, 1,
		 0.5,  0.5, -0.5, 1,
		 0.5,  0.5,  0.5, 1,
		
		 0.5, -0.5, -0.5, 1,
		-0.5, -0.5, -0.5, 1,
		 0.5, -0.5,  0.5, 1,
		 0.5, -0.5,  0.5, 1,
		-0.5, -0.5, -0.5, 1,
	    -0.5, -0.5,  0.5, 1,
		};
		const float cube_normal_data[6*2*3*4] = {
		 0, 0, 1, 0,
		 0, 0, 1, 0,
		 0, 0, 1, 0,
		 0, 0, 1, 0,
		 0, 0, 1, 0,
		 0, 0, 1, 0,
		
		 0, 0,-1, 0,
		 0, 0,-1, 0,
		 0, 0,-1, 0,
		 0, 0,-1, 0,
		 0, 0,-1, 0,
		 0, 0,-1, 0,
		
		 1, 0, 0, 0,
		 1, 0, 0, 0,
		 1, 0, 0, 0,
		 1, 0, 0, 0,
		 1, 0, 0, 0,
		 1, 0, 0, 0,
		
		-1, 0, 0, 0,
		-1, 0, 0, 0,
		-1, 0, 0, 0,
		-1, 0, 0, 0,
		-1, 0, 0, 0,
		-1, 0, 0, 0,
		
		 0, 1, 0, 0,
		 0, 1, 0, 0,
		 0, 1, 0, 0,
		 0, 1, 0, 0,
		 0, 1, 0, 0,
		 0, 1, 0, 0,
		
		 0,-1, 0, 0,
		 0,-1, 0, 0,
		 0,-1, 0, 0,
		 0,-1, 0, 0,
		 0,-1, 0, 0,
		 0,-1, 0, 0,
		};
		cube_vertex_buffer.loadData(cube_vertex_data, sizeof_array(cube_vertex_data));
		cube_normal_buffer.loadData(cube_normal_data, sizeof_array(cube_normal_data));
		
		const float quad_vertex_data[2*3*4] = {
		-0.5, -0.5, 0, 1,
		 0.5, -0.5, 0, 1,
		 0.5,  0.5, 0, 1,
		-0.5, -0.5, 0, 1,
		 0.5,  0.5, 0, 1,
		-0.5,  0.5, 0, 1,
		};
		const float quad_normal_data[2*3*4] = {
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		};
		quad_vertex_buffer.loadData(quad_vertex_data, sizeof_array(quad_vertex_data));
		quad_normal_buffer.loadData(quad_normal_data, sizeof_array(quad_normal_data));
	}
	
	void draw(Projector *proj, Camera *cam, const fmat4 &model, gl::Program *prog) {
		static const fmat4 vert_to_tex(
		1, 0, 0, 0.5, 
		0, 1, 0, 0.5, 
		0, 0, 1, 0.5, 
		0, 0, 0, 1
		);
		prog->setUniform("u_view", cam->view.data(), 16);
		prog->setUniform("u_inv_view", invert(cam->view).data(), 16);
		prog->setUniform("u_lod", ivec2(0,0).data(), 2);
		
		// draw cube
		prog->setAttribute("a_vertex", &cube_vertex_buffer);
		prog->setAttribute("a_normal", &cube_normal_buffer);
		prog->setUniform("u_model", model.data(), 16);
		prog->setUniform("u_inv_model", invert(model).data(), 16);
		prog->setUniform("u_tex", vert_to_tex.data(), 16);
		prog->setUniform("u_inv_tex", invert(vert_to_tex).data(), 16);
		prog->evaluate();
		
		//draw clipping quad
		fmat4 imv = invert(cam->view*model);
		if(sqrt(abs2(imv*fvec4(0,0,0,1)) - 1) <= sqrt(3) + proj->n) {
			float epsm = (1.0 + 1e-3);
			fmat4 move = fmat4(
			2*proj->w*epsm, 0, 0, 0,
			0, 2*proj->h*epsm, 0, 0,
			0, 0, 1, -proj->n*epsm,
			0, 0, 0, 1
			);
			prog->setAttribute("a_vertex", &quad_vertex_buffer);
			prog->setAttribute("a_normal", &quad_normal_buffer);
			fmat4 mmodel = model*imv*move;
			prog->setUniform("u_model", mmodel.data(), 16);
			prog->setUniform("u_inv_model", invert(mmodel).data(), 16);
			fmat4 mtex = vert_to_tex*imv*move;
			prog->setUniform("u_tex", mtex.data(), 16);
			prog->setUniform("u_inv_tex", invert(mtex).data(), 16);
			prog->evaluate();
		}
	}
};
