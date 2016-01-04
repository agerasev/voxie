#pragma once

#include <cmath>
#ifndef M_PI
#define M_PI 3.1415
#endif // M_PI

#include <la/mat.hpp>

class Camera {
public:
	fmat4 view = unifmat4;
	float phi = 0.0, theta = 0.0, radius = 1.6;
	
	fmat4 look_from(fvec3 p) {
		fvec3 z = normalize(p);
		fvec3 x = normalize(fvec3(0, 0, 1) ^ z);
		fvec3 y = z ^ x;
		return invert(fmat4(
		  x.x(), y.x(), z.x(), p.x(),
		  x.y(), y.y(), z.y(), p.y(),
		  x.z(), y.z(), z.z(), p.z(),
		  0, 0, 0, 1
		));
	}
	
	void move(int dx, int dy) {
		const float d = 1e-4;
		phi -= 0.01*dx;
		theta += 0.01*dy;
		if(theta > 0.5*M_PI - d)
			theta = 0.5*M_PI - d;
		if(theta < -0.5*M_PI + d)
			theta = -0.5*M_PI + d;
		update_view();
	}
	
	void zoom(int z) {
		radius *= pow(1.12, z);
		update_view();
	}
	
	void update_view() {
		view = look_from(vec3(
		  radius*cos(phi)*cos(theta),
		  radius*sin(phi)*cos(theta),
		  radius*sin(theta)
		));
	}
};
