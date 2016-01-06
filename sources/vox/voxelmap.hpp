#pragma once

#include <cstdio>
#include <vector>

#include <gl/texture.hpp>

#include <la/vec.hpp>

class VoxelMap {
public:
	gl::Texture color;
	gl::Texture shadow;
	
	std::vector<unsigned char> data;
	ivec3 size;
	
private:
	static int is_solid(const unsigned char *c, ivec3 s, int x, int y, int z) {
		if(x < 0 || x >= s[0] || y < 0 || y >= s[1] || z < 0 || z >= s[2])
			return 0;
		return c[4*((z*s[1] + y)*s[0] + x) + 3] > 0;
	}
	static int align(int n, int a) {
		return ((n - 1)/a + 1)*a;
	}
	int dataSize() {
		return 4*size[0]*size[1]*size[2];
	}
	
public:
	void init(ivec3 size, const unsigned char *data = nullptr, bool host = false) {
		this->size = size;
		color.loadData(3, data, size.data(), gl::Texture::RGBA8, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::NEAREST);
		if(host && data != nullptr) {
			this->data.resize(dataSize());
			memcpy(this->data.data(), data, this->data.size());
		}
		
		unsigned char *shdata = nullptr;
		ivec3 shsize = size + ivec3(1,1,1);
		std::vector<unsigned char> shdata_vector;
		if(data != nullptr) {
			shdata_vector.resize(4*shsize[0]*shsize[1]*shsize[2]);
			shdata = shdata_vector.data();
			for(int iz = 0; iz < shsize.z(); ++iz)
			for(int iy = 0; iy < shsize.y(); ++iy)
			for(int ix = 0; ix < shsize.x(); ++ix)
			{
				int i = 4*(shsize.x()*(shsize.y()*iz + iy) + ix);
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
		shadow.loadData(3, shdata, shsize.data(), gl::Texture::RGBA8, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::LINEAR);	
	}
	
	void write(const unsigned char *data, ivec3 offset, ivec3 sub_size) {
		color.loadSubData(data, offset.data(), sub_size.data(), gl::Texture::RGBA, gl::Texture::UBYTE);
		
		if(int(this->data.size()) >= dataSize()) {
			unsigned char *shdata = nullptr;
			ivec3 shsize = sub_size + ivec3(1,1,1);
			std::vector<unsigned char> shdata_vector;
			if(data != nullptr) {
				shdata_vector.resize(4*shsize[0]*shsize[1]*shsize[2]);
				shdata = shdata_vector.data();
				for(int iz = 0; iz < shsize.z(); ++iz)
				for(int iy = 0; iy < shsize.y(); ++iy)
				for(int ix = 0; ix < shsize.x(); ++ix)
				{
					int i = 4*(shsize.x()*(shsize.y()*iz + iy) + ix);
					for(int k = 0; k < 3; ++k) {
						int cs[2] = {0, 0};
						for(int j = 0; j < 8; ++j) {
							int ic[3] = {(j>>0)&1, (j>>1)&1, (j>>2)&1};
							cs[ic[k]] += is_solid(this->data.data(), size, offset.x() + ix - ic[0], offset.y() + iy - ic[1], offset.z() + iz - ic[2]);
						}
						int c = cs[1] > cs[0] ? cs[0] : cs[1];
						float v[5] = {1.0, 0.5, 0.5, 0.25, 0.0};
						shdata[i + k] = 0xFF*v[c];
					}
				}
			}
			shadow.loadSubData(shdata, offset.data(), shsize.data(), gl::Texture::RGBA, gl::Texture::UBYTE);
		}
	}
	
	int fileLoad(const std::string &fn, bool host = false) {
		int status = 0;
		FILE *file = fopen(fn.c_str(), "rb");
		if(file != nullptr) {
			fseek(file, 0, SEEK_END);
			long s = ftell(file);
			fseek(file, 0, SEEK_SET);
			
			if(s >= 3) {
				unsigned char d[4] = {0,0,0,0};
				fread(d, 4, 1, file);
				size = ivec3(int(d[0]) + 1, int(d[1]) + 1, int(d[2]) + 1);
				if(s >= 4 + dataSize()) {
					data.resize(dataSize());
					fread(data.data(), data.size(), 1, file);
					init(size, data.data());
				} else {
					size = ivec3(0,0,0);
					status = 3;
				}
			} else {
				status = 2;
			}
			fclose(file);
		} else {
			status = 1;
		}
		return status;
	}
	
	int fileSave(const std::string &fn) {
		int status = 0;
		if(4*size[0]*size[1]*size[2] <= int(data.size())) {
			FILE *file = fopen(fn.c_str(), "wb");
			if(file != nullptr) {
				unsigned char hdr[4] = {(unsigned char)(size[0] - 1), (unsigned char)(size[1] - 1), (unsigned char)(size[2] - 1), 0};
				fwrite(hdr, 4, 1, file);
				fwrite(data.data(), data.size(), 1, file);
				fclose(file);
			} else {
				status = 2;
			}
		} else {
			status = 1;
		}
		return status;
	}
};
