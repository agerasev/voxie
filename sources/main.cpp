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
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
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
	Graphics gfx;
	gfx.resize(width, height);
	
	bool done = false;
	bool mouse = false;
	while(!done) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_MOUSEMOTION) {
				if(mouse)
					gfx.get_camera()->move(event.motion.xrel, event.motion.yrel);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				if(event.button.button == SDL_BUTTON_LEFT)
					mouse = true;
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				if(event.button.button == SDL_BUTTON_LEFT)
					mouse = false;
			} else if(event.type == SDL_MOUSEWHEEL) {
				if(event.wheel.y)
					gfx.get_camera()->zoom(-event.wheel.y);
			} else if(event.type == SDL_QUIT) {
				done = true;
			} else if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
				}
			} else if(event.type == SDL_WINDOWEVENT) {
				if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					gfx.resize(event.window.data1, event.window.data2);
				}
			}
		}
		
		gfx.render();
		context.swap();
	}
	
	return 0;
}

