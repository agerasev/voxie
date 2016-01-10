#pragma once

#include <la/vec.hpp>
#include <la/mat.hpp>

#include "id.hpp"

class Object {
public:
	ID id = 0;
	fmat4 model = unifmat4;
	
	Object() = default;
	virtual ~Object() = default;
};
