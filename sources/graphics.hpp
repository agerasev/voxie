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
#include "camera.hpp"
#include "cube.hpp"

class Graphics {
private:
	int width = 0, height = 0;
	
	gl::Shader place, trace;
	gl::Program program;
	
	Projector proj;
	Camera cam;
	Cube cube;
	VoxelMap vox_map;
	
public:
	Graphics() : place(gl::Shader::VERTEX), trace(gl::Shader::FRAGMENT) {
		place.loadSourceFromFile("shaders/voxelmap_place.vert");
		trace.loadSourceFromFile("shaders/voxelmap_trace.frag");
		place.compile();
		trace.compile();
		
		program.setName("voxelmap");
		program.attach(&place);
		program.attach(&trace);
		program.link();
		
		const int bs[3] = {16,16,16};
		unsigned char *data = new unsigned char[4*bs[0]*bs[1]*bs[2]];
		for(int iz = 0; iz < bs[2]; ++iz)
		for(int iy = 0; iy < bs[1]; ++iy)
		for(int ix = 0; ix < bs[0]; ++ix) {
			double 
			  x = double(ix)/bs[0], 
			  y = double(iy)/bs[1],
			  z = double(iz)/bs[2];
			int i = 4*((iz*bs[1] + iy)*bs[0] + ix);
			data[i + 0] = 0xff*x;
			data[i + 1] = 0xff*y;
			data[i + 2] = 0xff*z;
			data[i + 3] = 0xff*((0.9 - (x*x + y*y + z*z)) > 0.0);
		}
		vox_map.init(bs, data);
		delete[] data;
		
		proj.update(1, 1);
		cam.move(0,0);
		
		program.setUniform("u_size", bs, 3);
		program.setUniform("u_texture", &vox_map.color);
		program.setUniform("u_shadow", &vox_map.shadow);
		
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f,0.2f,0.2f,1.0f);
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		glViewport(0, 0, w, h);
		proj.update(w, h);
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		program.setUniform("u_proj", proj.proj.data(), 16);
		
		fmat4 model = unifmat4;
		cube.draw(&proj, &cam, model, &program);
		
		glFlush();
	}
	
	Camera *get_camera() {
		return &cam;
	}
};
