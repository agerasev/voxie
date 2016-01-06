#pragma once

#include <cmath>
#ifndef M_PI
#define M_PI 3.1415
#endif // M_PI

#include <la/mat.hpp>

class Camera {
public:
	fmat4 view = unifmat4;
	float phi = M_PI, theta = 0.0;
	fvec3 pos = fvec3(0.6,0,0);
	fvec3 dir = fvec3(-1,0,0);
	
	static fmat4 look(fvec3 p, fvec3 d) {
		fvec3 z = -normalize(d);
		fvec3 x = normalize(fvec3(0,0,1)^z);
		fvec3 y = z^x;
		return invert(fmat4(
		  x.x(), y.x(), z.x(), p.x(),
		  x.y(), y.y(), z.y(), p.y(),
		  x.z(), y.z(), z.z(), p.z(),
		  0, 0, 0, 1
		));
	}
	
	Camera() {
		move(0,0);
	}
	
	void move(int dx, int dy) {
		const float d = 1e-4;
		const float s = 4e-3;
		phi -= s*dx;
		theta -= s*dy;
		if(theta > 0.5*M_PI - d)
			theta = 0.5*M_PI - d;
		if(theta < -0.5*M_PI + d)
			theta = -0.5*M_PI + d;
		update_view();
	}
	
	void update_view() {
		dir = vec3(
		cos(phi)*cos(theta),
	    sin(phi)*cos(theta),
	    sin(theta)
		);
		view = look(pos,dir);
	}
};
