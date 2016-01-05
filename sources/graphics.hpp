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
public:
	int width = 0, height = 0;
	
	gl::Shader place, trace;
	gl::Program program;
	
	Projector proj;
	Camera cam;
	Cube cube;

	VoxelMap vox_map;
	std::vector<unsigned char> vox_map_data;
	int vox_map_size[3] = {16,16,16};
	fmat4 model = unifmat4;
	
	Graphics(const int size[3], unsigned char *in_data = nullptr) : place(gl::Shader::VERTEX), trace(gl::Shader::FRAGMENT) {
		place.loadSourceFromFile("shaders/voxelmap_place.vert");
		trace.loadSourceFromFile("shaders/voxelmap_trace.frag");
		place.compile();
		trace.compile();
		
		program.setName("voxelmap");
		program.attach(&place);
		program.attach(&trace);
		program.link();
		
		vox_map_size[0] = size[0];
		vox_map_size[1] = size[1];
		vox_map_size[2] = size[2];
		
		const int *bs = vox_map_size;
		vox_map_data.resize(4*bs[0]*bs[1]*bs[2]);
		unsigned char *data = vox_map_data.data();
		if(in_data == nullptr) {
			for(int iz = 0; iz < bs[2]; ++iz)
			for(int iy = 0; iy < bs[1]; ++iy)
			for(int ix = 0; ix < bs[0]; ++ix) {
				double 
				  x = double(ix)/bs[0], 
				  y = double(iy)/bs[1],
				  z = double(iz)/bs[2];
				int i = 4*((iz*bs[1] + iy)*bs[0] + ix);
				data[i + 0] = 0xa0;//0xff*x;
				data[i + 1] = 0xa0;//0xff*y;
				data[i + 2] = 0xa0;//0xff*z;
				data[i + 3] = 0xff;//*((0.9 - (x*x + y*y + z*z)) > 0.0);
			}
		} else {
			for(int iz = 0; iz < bs[2]; ++iz)
			for(int iy = 0; iy < bs[1]; ++iy)
			for(int ix = 0; ix < bs[0]; ++ix) {
				int i = 4*((iz*bs[1] + iy)*bs[0] + ix);
				data[i + 0] = in_data[i + 0];
				data[i + 1] = in_data[i + 1];
				data[i + 2] = in_data[i + 2];
				data[i + 3] = in_data[i + 3];
			}
		}
		vox_map.init(bs, data);
		
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
		
		cube.draw(&proj, &cam, model, &program);
		
		glFlush();
	}
	
	Camera *get_camera() {
		return &cam;
	}
	
	void add(int mx, int my) {
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			const int *bs = vox_map_size;
			if(over.x() >= 0 && over.x() < bs[0] && over.y() >= 0 && over.y() < bs[1] && over.z() >= 0 && over.z() < bs[2]) {
				vox_map_data[4*((over.z()*bs[1] + over.y())*bs[0] + over.x()) + 3] = 0xFF;
				vox_map.init(bs, vox_map_data.data());
			}
		}
	}
	
	void remove(int mx, int my) {
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			const int *bs = vox_map_size;
			if(under.x() >= 0 && under.x() < bs[0] && under.y() >= 0 && under.y() < bs[1] && under.z() >= 0 && under.z() < bs[2]) {
				vox_map_data[4*((under.z()*bs[1] + under.y())*bs[0] + under.x()) + 3] = 0x00;
				vox_map.init(bs, vox_map_data.data());
			}
		}
	}
	
	ivec3 prev = fvec3(-1,-1,-1);
	void highlight(int mx, int my) {
		ivec3 next = fvec3(-1,-1,-1);
		bool upd = false;
		ivec3 under, over;
		if(intersect(mx, my, under, over)) {
			next = under;
		}
		if(next.x() != prev.x() || next.y() != prev.y() || next.z() != prev.z()) {
			const int *bs = vox_map_size;
			if(prev.x() >= 0 && prev.x() < bs[0] && prev.y() >= 0 && prev.y() < bs[1] && prev.z() >= 0 && prev.z() < bs[2]) {
				int i = 4*((prev.z()*bs[1] + prev.y())*bs[0] + prev.x());
				vox_map_data[i + 0] = 0xa0;
				vox_map_data[i + 1] = 0xa0;
				vox_map_data[i + 2] = 0xa0;
				upd = true;
			}
			if(next.x() >= 0 && next.x() < bs[0] && next.y() >= 0 && next.y() < bs[1] && next.z() >= 0 && next.z() < bs[2]) {
				int i = 4*((next.z()*bs[1] + next.y())*bs[0] + next.x());
				vox_map_data[i + 0] = 0xff;
				vox_map_data[i + 1] = 0xff;
				vox_map_data[i + 2] = 0x00;
				upd = true;
			}
			prev = next;
		}
		if(upd) {
			vox_map.init(vox_map_size, vox_map_data.data());
		}
	}
	
	bool intersect(int mx, int my, ivec3 &under, ivec3 &over) {
		fvec2 mv(2*(float)mx/width - 1, 1 - 2*(float)my/height);
		fvec4 view_pos(mv[0]*proj.w, mv[1]*proj.h, -proj.n, 1.0);
		fvec4 view_dir(view_pos.sub<3>(), 0.0);
		fmat4 imv = invert(model*cam.view);
		fvec3 pos = (imv*view_pos).sub<3>() + vec3(0.5,0.5,0.5);
		fvec3 dir = (imv*view_dir).sub<3>();
		
		fvec3 size = fvec3(vox_map_size[0], vox_map_size[1], vox_map_size[2]);
		fvec3 d = dir*size;
		fvec3 p = pos*size;
		ivec3 id((d.x() > 0) - (d.x() < 0), (d.y() > 0) - (d.y() < 0), (d.z() > 0) - (d.z() < 0));
		ivec3 ip(
		  int(ceil(p.x()))*(id.x() > 0) + int(floor(p.x()))*(id.x() < 0),
		  int(ceil(p.y()))*(id.y() > 0) + int(floor(p.y()))*(id.y() < 0),
		  int(ceil(p.z()))*(id.z() > 0) + int(floor(p.z()))*(id.z() < 0)
		);
		ivec3 dip(0,0,0);
		
		fvec3 sp = p, cp = p;
		float color = 0;
		
		bool found = false;
		for(int i = 0; i < 0x100; ++i) {
			// break if point is outside
			/*
			if(is_outside(sp,d,size)) {
				break;
			}
			*/
			
			// get color and break if opaque enough
			ivec3 icp(floor(cp.x()), floor(cp.y()), floor(cp.z()));
			const int *bs = vox_map_size;
			if(icp.x() >= 0 && icp.x() < bs[0] && icp.y() >= 0 && icp.y() < bs[1] && icp.z() >= 0 && icp.z() < bs[2])
				color = vox_map_data[4*((icp.z()*bs[1] + icp.y())*bs[0] + icp.x()) + 3];
				if(color > 0.9) {
					under = icp;
					over = icp - dip;
					found = true;
					break;
				}
			
			// choose next intersection plane
			fvec3 ts;
			float t;
			ts = (fvec3(ip) - p)/d;
			if(ts.x() < ts.y()) {
				if(ts.x() < ts.z()) {
					dip = ivec3(id.x(),0,0);
					t = ts.x();
				} else {
					dip = ivec3(0,0,id.z());
					t = ts.z();
				}
			} else {
				if(ts.y() < ts.z()) {
					dip = ivec3(0,id.y(),0);
					t = ts.y();
				} else {
					dip = ivec3(0,0,id.z());
					t = ts.z();
				}
			}
			
			// compute next intersection parameters
			sp = p + d*t;
			cp = sp + 0.5f*fvec3(dip);
			
			// increment intersection iterator
			ip += dip;
		}
		
		if(found) {
			//printf("%d %d %d\n", under(0), under(1), under(2));
			//fflush(stdout);
			return true;
		}
		
		return false;
	}
};
