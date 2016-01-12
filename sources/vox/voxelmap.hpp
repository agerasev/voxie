#pragma once

#include <cstdio>
#include <vector>

#include <gl/texture.hpp>

#include <la/vec.hpp>

class VoxelMap {
public:
	gl::Texture texture;
	gl::Texture light;
	
	std::vector<unsigned char> data;
	ivec3 size, offset;
	ivec3 real_size;
	
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
	void init(ivec3 real_size, const unsigned char *data = nullptr, bool host = false) {
		this->real_size = real_size;
		size = real_size - ivec3(2,2,2);
		offset = ivec3(1,1,1);
		texture.loadData(3, data, real_size.data(), gl::Texture::RGBA8, gl::Texture::RGBA, gl::Texture::UBYTE, gl::Texture::NEAREST);
		if(host && data != nullptr) {
			this->data.resize(dataSize());
			memcpy(this->data.data(), data, this->data.size());
		}
	}
	
	void write(const unsigned char *data, ivec3 offset, ivec3 sub_size) {
		texture.loadSubData(data, offset.data(), sub_size.data(), gl::Texture::RGBA, gl::Texture::UBYTE);
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
				real_size = ivec3(int(d[0]) + 1, int(d[1]) + 1, int(d[2]) + 1);
				if(s >= 4 + dataSize()) {
					data.resize(dataSize());
					fread(data.data(), data.size(), 1, file);
					init(real_size, data.data());
				} else {
					real_size = ivec3(0,0,0);
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
		if(4*real_size[0]*real_size[1]*real_size[2] <= int(data.size())) {
			FILE *file = fopen(fn.c_str(), "wb");
			if(file != nullptr) {
				unsigned char hdr[4] = {(unsigned char)(real_size[0] - 1), (unsigned char)(real_size[1] - 1), (unsigned char)(real_size[2] - 1), 0};
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
