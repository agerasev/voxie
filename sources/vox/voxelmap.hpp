#pragma once

#include <cstdio>
#include <vector>

#include <gl/texture.hpp>

#include <la/vec.hpp>

class VoxelMap {
public:
	gl::Texture texture;
	gl::Texture light;
	
	std::vector<std::vector<unsigned char>> data_vector;
	std::vector<unsigned char> &data;
	ivec3 size = nullivec3;
	
	bool host = false;
	
	VoxelMap() 
	  : data_vector(1), data(data_vector[0])
	{
		
	}
	
	~VoxelMap() = default;
	
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
		texture.init(3, size.data(), gl::Texture::RGBA8);
		texture.write(data, nullivec3.data(), size.data(), gl::Texture::RGBA, gl::Texture::UBYTE);
		texture.setInterpolation(gl::Texture::NEAREST_MIPMAP_NEAREST, gl::Texture::NEAREST);
		this->host = host;
		if(host && data != nullptr) {
			this->data.resize(dataSize());
			memcpy(this->data.data(), data, this->data.size());
		}
	}
	
	void write(const unsigned char *data, ivec3 offset, ivec3 size) {
		texture.write(data, nullivec3.data(), size.data(), gl::Texture::RGBA, gl::Texture::UBYTE);
		// TODO: update host data respectively
	}
	
	static long get_index(ivec3 p, ivec3 s) {
		return s.x()*(s.y()*p.z() + p.y()) + p.x();
	}
	
	void genMipMap(int mlvl) {
		if(host) {
			data_vector.resize(mlvl + 1);
			ivec3 prev_mmsize = size;
			for(int i = 1; i <= mlvl; ++i) {
				ivec3 mmsize = (size - ivec3(1,1,1))/(1<<i) + ivec3(1,1,1);
				const std::vector<unsigned char> &prev_mmdata = data_vector[i - 1];
				std::vector<unsigned char> &mmdata = data_vector[i];
				mmdata.resize(4*mmsize[0]*mmsize[1]*mmsize[2]);
				for(int iz = 0; iz < mmsize.z(); ++iz)
				for(int iy = 0; iy < mmsize.y(); ++iy)
				for(int ix = 0; ix < mmsize.x(); ++ix) 
				{
					unsigned int new_val[4] = {0,0,0,0};
					for(int j = 0; j < 8; ++j) {
						int idx = 4*get_index(ivec3(2*ix + ((j>>0)&1), 2*iy + ((j>>1)&1), 2*iz + ((j>>2)&1)), prev_mmsize);
						unsigned char val[4];
						for(int k = 0; k < 4; ++k)
							val[k] = prev_mmdata[idx + k];
						for(int k = 0; k < 3; ++k)
							new_val[k] += val[k];
						if(val[3] > new_val[3])
							new_val[3] = val[3];
					}
					for(int k = 0; k < 3; ++k)
						new_val[k] /= 8;
					int idx = 4*get_index(ivec3(ix,iy,iz), mmsize);
					for(int k = 0; k < 4; ++k)
						mmdata[idx + k] = new_val[k];
				}
				texture.init(3,mmsize.data(),gl::Texture::RGBA8,i);
				texture.write(mmdata.data(),nullivec3.data(),mmsize.data(),gl::Texture::RGBA,gl::Texture::UBYTE,i);
				prev_mmsize = mmsize;
			}
		} else {
			fprintf(stderr, "cannot generate mipmap for texture %d without host data\n", texture.id());
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
					this->host = true;
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
