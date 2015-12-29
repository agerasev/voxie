#pragma once

#include <gl/vertexbuffer.hpp>
#include <gl/program.hpp>

#include "projector.hpp"
#include "camera.hpp"
#include "voxelmap.hpp"

class Cube {
public:
	gl::VertexBuffer cube_buffer;
	gl::VertexBuffer quad_buffer;
	Cube() {
		static const float cube_data[] = {
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
		cube_buffer.loadData(cube_data, 6*2*3*4);
		
		static const float quad_data[] = {
		-0.5, -0.5, 0, 1,
		 0.5, -0.5, 0, 1,
		 0.5,  0.5, 0, 1,
		-0.5, -0.5, 0, 1,
		 0.5,  0.5, 0, 1,
		-0.5,  0.5, 0, 1,
		};
		quad_buffer.loadData(quad_data, 2*3*4);
	}
	
	fmat3 get_tex_basis(const fmat4 &model, const fmat4 &view) {
		fmat4 tb = unifmat4;
		tb(3,3) = 0.0;
		tb = view*(model*tb);
		return invert(fmat3(tb(0,0), tb(1,0), tb(2,0), tb(0,1), tb(1,1), tb(2,1), tb(0,2), tb(1,2), tb(2,2)));
	}
	
	void draw(Projector *proj, Camera *cam, const fmat4 &model, gl::Program *prog) {
		static const fmat4 vert_to_tex(
		1, 0, 0, 0.5, 
		0, 1, 0, 0.5, 
		0, 0, 1, 0.5, 
		0, 0, 0, 1
		);
		prog->setUniform("u_view", cam->view.data(), 16);
		prog->setUniform("u_tex_basis", get_tex_basis(model, cam->view).data(), 9);
		
		// draw cube
		prog->setAttribute("a_vertex", &cube_buffer);
		prog->setUniform("u_model", model.data(), 16);
		prog->setUniform("u_tex_map", vert_to_tex.data(), 16);
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
			prog->setAttribute("a_vertex", &quad_buffer);
			prog->setUniform("u_model", (model*imv*move).data(), 16);
			prog->setUniform("u_tex_map", (vert_to_tex*imv*move).data(), 16);
			prog->evaluate();
		}
	}
};
