#pragma once

#include <cstdio>

#include <string>
#include <vector>
#include <map>
#include <functional>

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
	std::map<std::string, gl::Shader*> shaders;
	std::map<std::string, gl::Program*> programs;
	
	struct ShaderInfo {
		std::string name;
		std::string path;
		gl::Shader::Type type;
		ShaderInfo(const std::string &n, const std::string &p, gl::Shader::Type t)
		  : name(n), path(p), type(t) {}
	};
	
	struct ProgramInfo {
		std::string name;
		std::string vert, frag;
		ProgramInfo(const std::string &n, const std::string &v, const std::string &f)
		  : name(n), vert(v), frag(f) {}
	};
	
	Projector proj;
	Camera cam;
	Cube cube;
	VoxelMap vox_map;
	
public:
	Graphics()
	{
		std::vector<ShaderInfo> shader_info({
		  ShaderInfo("place", "shaders/voxelmap_place.vert", gl::Shader::VERTEX),
		  ShaderInfo("trace", "shaders/voxelmap_trace.frag", gl::Shader::FRAGMENT)
		});
		
		for(const ShaderInfo &info : shader_info) {
			gl::Shader *shader = new gl::Shader(info.type);
			shader->setName(info.name);
			shader->loadSourceFromFile(info.path);
			shader->compile();
			shaders.insert(std::pair<std::string, gl::Shader*>(info.name, shader));
		}
		
		std::vector<ProgramInfo> program_info({
		  ProgramInfo("voxelmap", "place", "trace")
		});
		
		for(const ProgramInfo &info : program_info) {
			gl::Program *prog = new gl::Program();
			prog->setName(info.name);
			prog->attach(shaders[info.vert]);
			prog->attach(shaders[info.frag]);
			prog->link();
			programs.insert(std::pair<std::string, gl::Program*>(info.name, prog));
		}
		
		int bs[3] = {16,16,16};
		vox_map.set_size(bs);
		
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
		vox_map.write((const char*)data);
		delete[] data;
		
		proj.update(1, 1);
		cam.move(0,0);
		
		programs["voxelmap"]->setUniform("u_tex_size", bs, 3);
		programs["voxelmap"]->setUniform("u_texture", &vox_map.texture);
		
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClearColor(0.2f,0.2f,0.2f,1.0f);
	}
	
	~Graphics() {
		for(const auto &p : programs) {
			delete p.second;
		}
		for(const auto &p : shaders) {
			delete p.second;
		}
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		glViewport(0, 0, w, h);
		proj.update(w, h);
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		programs["voxelmap"]->setUniform("u_proj", proj.proj.data(), 16);
		
		fmat4 model = unifmat4;
		cube.draw(&proj, &cam, model, programs["voxelmap"]);
		
		model(0, 0) = 0.4;
		model(1, 1) = 0.4;
		model(2, 2) = 0.4;
		model(3, 0) = 0.1;
		model(3, 1) = 0.2;
		model(3, 2) = 0.1;
		cube.draw(&proj, &cam, model, programs["voxelmap"]);
		
		glFlush();
	}
	
	Camera *get_camera() {
		return &cam;
	}
};
