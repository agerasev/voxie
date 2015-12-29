#pragma once

#include <gl/texture.hpp>

class VoxelMap {
public:
	gl::Texture texture;
	int size[3];
	
	void set_size(const int size[3]) {
		for(int i = 0; i < 3; ++i)
			this->size[i] = size[i];
	}
	const int *get_size() {
		return size;
	}
	
	void write(const char *data) {
		texture.loadData(3, data, size, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::NEAREST);
	}
	void write_part(const char *data, const int offset[3], const int sub_size[3]) {
		// TODO: loadSubData
	}
};
