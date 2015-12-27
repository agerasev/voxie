#pragma once

#include <cstdio>

#include <string>
#include <vector>
#include <map>
#include <functional>

#include <GL/glew.h>

#include "gl/program.hpp"
#include "gl/framebuffer.hpp"

#include "la/mat.hpp"

class Graphics {
private:
	int width = 0, height = 0;
	std::map<std::string, gl::Shader*> shaders;
	std::map<std::string, gl::Program*> programs;
	gl::VertexBuffer cube_buffer;
	gl::Texture texture;
	
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
	
	fmat4 
	  model = unifmat4, 
	  view  = unifmat4,
	  proj  = unifmat4;
	float phi = 0.0, theta = 0.0, radius = 1.6;

public:
	Graphics()
	{
		std::vector<ShaderInfo> shader_info({
		  ShaderInfo("position",  "shaders/position.vert",  gl::Shader::VERTEX),
		  ShaderInfo("draw",      "shaders/draw.frag",      gl::Shader::FRAGMENT)
		});
		
		for(const ShaderInfo &info : shader_info) {
			gl::Shader *shader = new gl::Shader(info.type);
			shader->setName(info.name);
			shader->loadSourceFromFile(info.path);
			shader->compile();
			shaders.insert(std::pair<std::string, gl::Shader*>(info.name, shader));
		}
		
		std::vector<ProgramInfo> program_info({
		  ProgramInfo("draw", "position", "draw")
		});
		
		for(const ProgramInfo &info : program_info) {
			gl::Program *prog = new gl::Program();
			prog->setName(info.name);
			prog->attach(shaders[info.vert]);
			prog->attach(shaders[info.frag]);
			prog->link();
			programs.insert(std::pair<std::string, gl::Program*>(info.name, prog));
		}
		
		float cube_data[] = {
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
		
		int bs[3] = {4,4,4};
		float *data = new float[3*bs[0]*bs[1]*bs[2]];
		for(int iz = 0; iz < bs[2]; ++iz)
		for(int iy = 0; iy < bs[1]; ++iy)
		for(int ix = 0; ix < bs[0]; ++ix) {
			double 
			  x = double(ix)/bs[0], 
			  y = double(iy)/bs[1],
			  z = double(iz)/bs[2];
			int i = 3*((iz*bs[1] + iy)*bs[0] + ix);
			data[i + 0] = x;
			data[i + 1] = y;
			data[i + 2] = z;
		}
		texture.loadData(3, data, bs, gl::Texture::RGB, gl::Texture::FLOAT, gl::Texture::NEAREST);
		delete[] data;
		
		move(0,0);
		
		programs["draw"]->setAttribute("a_vertex", &cube_buffer);
		programs["draw"]->setUniform("u_texture", &texture);
		
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.5f,0.5f,0.5f,1.0f);
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
		glViewport(0, 0, width, height);
		update_proj();
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		programs["draw"]->setUniform("u_model", model.data(), 16);
		programs["draw"]->setUniform("u_view",  view.data(),  16);
		programs["draw"]->setUniform("u_proj",  proj.data(),  16);
		programs["draw"]->evaluate();
		glFlush();
	}
	
	void move(int dx, int dy) {
		const float d = 1e-4;
		phi -= 0.01*dx;
		theta += 0.01*dy;
		if(theta > M_PI_2 - d)
			theta = M_PI_2 - d;
		if(theta < -M_PI_2 + d)
			theta = -M_PI_2 + d;
		view = look_from(vec3(
		  radius*cos(phi)*cos(theta),
		  radius*sin(phi)*cos(theta),
		  radius*sin(theta)
		));
	}
	
	void update_proj() {
		float w = 0.2, h = 0.2, f = 100, n = 0.2;
		w *= (float)width/height;
		proj(0, 0) = n/w;
		proj(1, 1) = n/h;
		proj(2, 2) = -(f + n)/(f - n);
		proj(3, 2) = -2*f*n/(f - n);
		proj(2, 3) = -1;
		proj(3, 3) = 0;
	}
	
	fmat4 look_from(fvec3 p) {
		fvec3 z = normalize(p);
		fvec3 x = normalize(fvec3(0, 0, 1) ^ z);
		fvec3 y = z ^ x;
		return invert(transpose(fmat4(
		  x.x(), x.y(), x.z(), 0,
		  y.x(), y.y(), y.z(), 0,
		  z.x(), z.y(), z.z(), 0,
		  p.x(), p.y(), p.z(), 1
		)));
	}
};
