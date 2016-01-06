#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>

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
	int width, height;
	Window(int w, int h) {
		width = w;
		height = h;
		window = SDL_CreateWindow(
		"Voxie",
	    SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED,
	    width, height,
	    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
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
	Context(const Window *w) {
		window = w->window;
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
