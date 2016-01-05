#include <cstdlib>
#include <cstdio>
#include <functional>

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "graphics.hpp"

class SDL {
public:
	SDL() {
		SDL_Init(SDL_INIT_VIDEO);
	}
	~SDL() {
		SDL_Quit();
	}
};

class Window {
public:
	SDL_Window *window;
	Window(const char *n, int x, int y, int w, int h, Uint32 f) {
		window = SDL_CreateWindow(n, x, y, w, h, f);
		if(window == nullptr) {
			fprintf(stderr, "Could not create SDL_Window\n");
			exit(1);
		}
	}
	~Window() {
		SDL_DestroyWindow(window);
	}
};

class Context {
public:
	SDL_Window *window;
	SDL_GLContext context;
	Context(const Window &w) {
		window = w.window;
		context = SDL_GL_CreateContext(window);
		if(context == NULL) {
			fprintf(stderr, "Could not create SDL_GL_Context\n");
			exit(1);
		}
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		/*
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		*/
		SDL_GL_SetSwapInterval(1);
	}
	~Context() {
		SDL_GL_DeleteContext(context);
	}
	void swap() {
		SDL_GL_SwapWindow(window);
	}
};

class GLEW {
public:
	GLEW() {
		GLenum status = glewInit();
		if(status != GLEW_OK) {
			fprintf(stderr, "Could not init GLEW: %s\n", glewGetErrorString(status));
			exit(1);
		}
		if(!GLEW_VERSION_3_0) {
			fprintf(stderr, "OpenGL 3.0 support not found\n");
			exit(1);
		}
	}
	~GLEW() = default;
};

int main(int argc, char *argv[]) {
	SDL sdl;
	int width = 800, height = 600;
	Window window(
	  "Voxie",
	  SDL_WINDOWPOS_CENTERED,
	  SDL_WINDOWPOS_CENTERED,
	  width, height,
	  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	Context context(window);
	GLEW glew;
	
	int size[3] = {16,16,16};
	std::vector<unsigned char> data_vector;
	unsigned char *data = nullptr;
	
	if(argc >= 2) {
		FILE *file = fopen(argv[1], "rb");
		if(file != nullptr) {
			fseek(file, 0, SEEK_END);
			long s = ftell(file);
			fseek(file, 0, SEEK_SET);
			
			data_vector.resize(s);
			fread(data_vector.data(), s, 1, file);
			if(s >= 3) {
				size[0] = int(data_vector[0]) + 1;
				size[1] = int(data_vector[1]) + 1;
				size[2] = int(data_vector[2]) + 1;
				if(s >= 4 + 4*size[0]*size[1]*size[2]) {
					data = data_vector.data() + 4;
				}
			}
			fclose(file);
		} else if(argc >= 5) {
			size[0] = atoi(argv[2]);
			size[1] = atoi(argv[3]);
			size[2] = atoi(argv[4]);
		}
	}
	
	Graphics gfx(size, data);
	gfx.resize(width, height);
	
	bool done = false;
	bool mouse = true;
	char movebits = 0;
	Uint32 last_tick = SDL_GetTicks();
	
	SDL_SetRelativeMouseMode((SDL_bool)mouse);
	
	while(!done) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_MOUSEMOTION) {
				int mx = event.button.x, my = event.button.y;
				if(mouse) {
					gfx.get_camera()->move(event.motion.xrel, event.motion.yrel);
					mx = width/2;
					my = height/2;
				}
				gfx.highlight(mx,my);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				int mx = event.button.x, my = event.button.y;
				if(mouse) {
					mx = width/2;
					my = height/2;
				}
				if(event.button.button == SDL_BUTTON_MIDDLE) {
					mouse = !mouse;
					SDL_SetRelativeMouseMode((SDL_bool)mouse);
				} else if(event.button.button == SDL_BUTTON_LEFT) {
					gfx.remove(mx, my);
					gfx.highlight(mx,my);
				} else if(event.button.button == SDL_BUTTON_RIGHT) {
					gfx.add(mx, my);
					gfx.highlight(mx,my);
				}
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				//if(event.button.button == SDL_BUTTON_MIDDLE) {
				//	mouse = false;
				//}
			} else if(event.type == SDL_MOUSEWHEEL) {
				//if(event.wheel.y)
				//	gfx.get_camera()->zoom(-event.wheel.y);
			} else if(event.type == SDL_QUIT) {
				done = true;
			} else if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					mouse = !mouse;
					SDL_SetRelativeMouseMode((SDL_bool)mouse);
				} else if(event.key.keysym.sym == SDLK_w) {
					movebits |= (1<<0);
				} else if(event.key.keysym.sym == SDLK_a) {
					movebits |= (1<<1);
				} else if(event.key.keysym.sym == SDLK_s) {
					movebits |= (1<<2);
				} else if(event.key.keysym.sym == SDLK_d) {
					movebits |= (1<<3);
				} else if(event.key.keysym.sym == SDLK_SPACE) {
					movebits |= (1<<4);
				} else if(event.key.keysym.sym == SDLK_LCTRL) {
					movebits |= (1<<5);
				}
			} else if(event.type == SDL_KEYUP) {
				if(event.key.keysym.sym == SDLK_w) {
					movebits &= ~(1<<0);
				} else if(event.key.keysym.sym == SDLK_a) {
					movebits &= ~(1<<1);
				} else if(event.key.keysym.sym == SDLK_s) {
					movebits &= ~(1<<2);
				} else if(event.key.keysym.sym == SDLK_d) {
					movebits &= ~(1<<3);
				} else if(event.key.keysym.sym == SDLK_SPACE) {
					movebits &= ~(1<<4);
				} else if(event.key.keysym.sym == SDLK_LCTRL) {
					movebits &= ~(1<<5);
				}
			} else if(event.type == SDL_WINDOWEVENT) {
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					width = event.window.data1; 
					height = event.window.data2;
					gfx.resize(width, height);
				}
			}
		}
		
		Uint32 tick = SDL_GetTicks();
		if(movebits) {
			float spd = 4e-1;
			fvec3 md(
			  !!(movebits & (1<<0)) - !!(movebits & (1<<2)),
			  !!(movebits & (1<<3)) - !!(movebits & (1<<1)),
			  !!(movebits & (1<<4)) - !!(movebits & (1<<5))
			);
			fvec3 dir = gfx.get_camera()->dir;
			md = transpose(fmat3(dir, normalize(cross(dir, fvec3(0,0,1))), fvec3(0,0,1)))*md;
			md *= spd*0.001*(tick - last_tick);
			gfx.get_camera()->pos += md;
			gfx.get_camera()->update_view();
		}
		last_tick = tick;
		
		gfx.render();
		context.swap();
	}
	
	if(glGetError() != GL_NO_ERROR) {
		fprintf(stderr, "there are OpenGL errors!\n");
		return 1;
	}
	
	if(argc >= 2) {
		FILE *file = fopen(argv[1], "wb");
		if(file != nullptr) {
			const int *vs = gfx.vox_map_size;
			unsigned char hdr[4] = {(unsigned char)(vs[0] - 1), (unsigned char)(vs[1] - 1), (unsigned char)(vs[2] - 1), 0};
			fwrite(hdr, 4, 1, file);
			fwrite(gfx.vox_map_data.data(), gfx.vox_map_data.size(), 1, file);
			fclose(file);
		}
	}
	
	return 0;
}
