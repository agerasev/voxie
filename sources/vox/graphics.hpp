#pragma once

#include <cstdio>

#include <string>
#include <vector>
#include <map>

#include <GL/glew.h>

#include <gl/program.hpp>
#include <gl/framebuffer.hpp>

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
		glClearColor(0.2f,0.2f,0.2f,1.0f);
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
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Camera *cam = dynamic_cast<Camera*>(storage->getObject(cam_id));
		if(cam != nullptr) {
			for(Object *obj : *storage) {
				VoxelObject *vobj = dynamic_cast<VoxelObject*>(obj);
				if(vobj != nullptr) {
					program.setUniform("u_size", vobj->map.size.data(), 3);
					program.setUniform("u_texture", &vobj->map.color);
					program.setUniform("u_shadow", &vobj->map.shadow);
					program.setUniform("u_proj", proj.proj.data(), 16);
					cube.draw(&proj, cam, vobj->model, &program);
				}
			}
		}
		
		glFlush();
	}
};
