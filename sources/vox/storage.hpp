#pragma once

#include "camera.hpp"
#include "voxelmap.hpp"

class Storage {
public:
	VoxelMap vox;
	Camera cam;
};
