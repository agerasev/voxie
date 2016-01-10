#pragma once

#include <cmath>

#include <la/mat.hpp>
#include "object.hpp"

class Camera : public Object {
public:
	fmat4 view = unifmat4;
	float phi = 3.1415, theta = 0.0;
	fvec3 pos = fvec3(0.6,0,0);
	fvec3 dir = fvec3(-1,0,0);
	
	Camera() {
		update();
	}
	virtual ~Camera() = default;
	
	static fmat4 look(fvec3 p, fvec3 d) {
		fvec3 z = -normalize(d);
		fvec3 x = normalize(fvec3(0,0,1)^z);
		fvec3 y = z^x;
		return fmat4(
		  x.x(), y.x(), z.x(), p.x(),
		  x.y(), y.y(), z.y(), p.y(),
		  x.z(), y.z(), z.z(), p.z(),
		  0, 0, 0, 1
		);
	}
	
	void update() {
		dir = vec3(
		  cos(phi)*cos(theta),
	      sin(phi)*cos(theta),
	      sin(theta)
		  );
		model = look(pos,dir);
		view = invert(model);
	}
};
