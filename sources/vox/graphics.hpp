#pragma once

#include <cstdio>

#include <algorithm>
#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>

#include <gl/program.hpp>

#include <la/mat.hpp>

#include "projector.hpp"
#include "cube.hpp"

#include "storage.hpp"

#include <vox/objects/camera.hpp>
#include <vox/objects/voxelobject.hpp>

class Graphics {
public:
	int width = 0, height = 0;
	
	gl::Shader place, trace;
	gl::Program program;
	
	Projector proj;
	Cube cube;
	
	Storage *storage;
	ID cam_id = 0;
	
	Graphics(Storage *s) 
	  : place(gl::Shader::VERTEX), trace(gl::Shader::FRAGMENT),
	    storage(s)
	{
		place.loadSourceFromFile("shaders/voxelmap_place.vert");
		trace.loadSourceFromFile("shaders/voxelmap_trace.frag");
		place.compile();
		trace.compile();
		
		program.setName("voxelmap");
		program.attach(&place);
		program.attach(&trace);
		program.link();
		
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClearColor(0.2f,0.6f,1.0f,1.0f);
	}
	
	void setCamera(ID id) {
		cam_id = id;
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		glViewport(0, 0, w, h);
		proj.update(w, h);
	}
	
	template <typename S, typename T>
	static bool cmp_pairs(const std::pair<S,T> &first, const std::pair<S,T> &second) {
		return first.first < second.first;
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Camera *cam = dynamic_cast<Camera*>(storage->getObject(cam_id));
		std::vector<std::pair<double,VoxelObject*>> vector;
		if(cam != nullptr) {
			for(Object *obj : *storage) {
				VoxelObject *vobj = dynamic_cast<VoxelObject*>(obj);
				if(vobj != nullptr) {
					double dist = abs2(vobj->model.col(3) - cam->model.col(3));
					vector.push_back(std::pair<double,VoxelObject*>(dist, vobj));
				}
			}
			std::sort(vector.data(), vector.data() + vector.size(), cmp_pairs<double,VoxelObject*>);
			for(auto p : vector) {
				VoxelObject *vobj = p.second;
				program.setUniform("u_size", vobj->map.size.data(), 3);
				program.setUniform("u_texture", &vobj->map.texture);
				// program.setUniform("u_light_texture", &vobj->map.light);
				
				program.setUniform("u_ambient", fvec4(0.1,0.3,0.5,1).data(), 4);
				program.setUniform("u_light_pos", fvec4(0,0.5,0.5*sqrt(3),0).data(), 4);
				program.setUniform("u_light_color", fvec4(0.9,0.7,0.5,1).data(), 4);
				
				program.setUniform("u_proj", proj.proj.data(), 16);
				
				fmat4 model = vobj->model;
				static const fmat4 vert_to_tex(
				1, 0, 0, 0.5, 
				0, 1, 0, 0.5, 
				0, 0, 1, 0.5, 
				0, 0, 0, 1
				);
				program.setUniform("u_view", cam->view.data(), 16);
				program.setUniform("u_inv_view", invert(cam->view).data(), 16);
				program.setUniform("u_lod", ivec2(0,4).data(), 2);
				
				// draw cube
				program.setAttribute("a_vertex", &cube.cube_vertex_buffer);
				program.setAttribute("a_normal", &cube.cube_normal_buffer);
				program.setUniform("u_model", model.data(), 16);
				program.setUniform("u_inv_model", invert(model).data(), 16);
				program.setUniform("u_tex", vert_to_tex.data(), 16);
				program.setUniform("u_inv_tex", invert(vert_to_tex).data(), 16);
				program.evaluate();
				
				//draw clipping quad
				fmat4 imv = invert(cam->view*model);
				if(sqrt(abs2(imv*fvec4(0,0,0,1)) - 1) <= 0.5*sqrt(3) + proj.n) {
					float epsm = (1.0 + 1e-3);
					fmat4 move = fmat4(
					2*proj.w*epsm, 0, 0, 0,
					0, 2*proj.h*epsm, 0, 0,
					0, 0, 1, -proj.n*epsm,
					0, 0, 0, 1
					);
					program.setAttribute("a_vertex", &cube.quad_vertex_buffer);
					program.setAttribute("a_normal", &cube.quad_normal_buffer);
					fmat4 mmodel = model*imv*move;
					program.setUniform("u_model", mmodel.data(), 16);
					program.setUniform("u_inv_model", invert(mmodel).data(), 16);
					fmat4 mtex = vert_to_tex*imv*move;
					program.setUniform("u_tex", mtex.data(), 16);
					program.setUniform("u_inv_tex", invert(mtex).data(), 16);
					program.evaluate();
				}
			}
		}
		
		glFlush();
	}
};
