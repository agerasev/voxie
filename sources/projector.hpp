#pragma once

#include <la/mat.hpp>

class Projector {
public:
	float f = 1e2, n = 1e-2;
	float w = n, h = n;
	fmat4 proj;
	void update(int sw, int sh) {
		w = (float)sw/sh*h;
		proj(0, 0) = n/w;
		proj(1, 1) = n/h;
		proj(2, 2) = -(f + n)/(f - n);
		proj(3, 2) = -2*f*n/(f - n);
		proj(2, 3) = -1;
		proj(3, 3) = 0;
	}
};
