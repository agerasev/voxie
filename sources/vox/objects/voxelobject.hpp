#pragma once

#include "object.hpp"
#include <vox/voxelmap.hpp>

class VoxelObject : public Object {
public:
	VoxelMap map;
	
	VoxelObject() = default;
	virtual ~VoxelObject() = default;
};
