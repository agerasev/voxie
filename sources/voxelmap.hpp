#pragma once

#include <vector>

#include <gl/texture.hpp>

class VoxelMap {
public:
	gl::Texture color;
	gl::Texture shadow;
	int size[3];
	
	static int is_solid(const unsigned char *c, const int s[3], int x, int y, int z) {
		if(x < 0 || x >= s[0] || y < 0 || y >= s[1] || z < 0 || z >= s[2])
			return 0;
		return c[4*((z*s[1] + y)*s[0] + x) + 3] > 0;
	}
	static int align(int n, int a) {
		return ((n - 1)/a + 1)*a;
	}
	
	void init(const int size[3], const unsigned char *data = nullptr) {
		for(int i = 0; i < 3; ++i)
			this->size[i] = size[i];
		color.loadData(3, data, size, gl::Texture::RGBA8, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::LINEAR);
		
		unsigned char *shdata = nullptr;
		const int shsize[3] = {size[0] + 1, size[1] + 1, size[2] + 1};
		std::vector<unsigned char> shdata_vector;
		if(data != nullptr) {
			shdata_vector.resize(4*shsize[0]*shsize[1]*shsize[2]);
			shdata = shdata_vector.data();
			for(int iz = 0; iz < shsize[2]; ++iz)
			for(int iy = 0; iy < shsize[1]; ++iy)
			for(int ix = 0; ix < shsize[0]; ++ix)
			{
				int i = 4*(shsize[0]*(shsize[1]*iz + iy) + ix);
				for(int k = 0; k < 3; ++k) {
					int cs[2] = {0, 0};
					for(int j = 0; j < 8; ++j) {
						int ic[3] = {(j>>0)&1, (j>>1)&1, (j>>2)&1};
						cs[ic[k]] += is_solid(data, size, ix - ic[0], iy - ic[1], iz - ic[2]);
					}
					int c = cs[1] > cs[0] ? cs[0] : cs[1];
					float v[5] = {1.0, 0.5, 0.5, 0.25, 0.0};
					shdata[i + k] = 0xFF*v[c];
				}
			}
		}
		shadow.loadData(3, shdata, shsize, gl::Texture::RGBA8, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::LINEAR);
		
	}
	void write(const unsigned char *data, const int offset[3], const int sub_size[3]) {
		// TODO: loadSubData
	}
};
