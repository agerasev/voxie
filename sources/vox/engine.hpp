#pragma once

#include "window.hpp"
#include "input.hpp"

#include "graphics.hpp"
#include "storage.hpp"

class Engine {
private:
	SDL sdl;
	Window window;
	Context context;
	GLEW glew;
	
	Storage storage;
	Graphics graphics;
	Input input;
	
public:
	Engine() 
	  : window(800, 600), context(&window), 
	    graphics(&storage), input(&window, &storage, &graphics)
	{
		graphics.resize(window.width, window.height);
	}
	~Engine() {
		if(glGetError() != GL_NO_ERROR) {
			fprintf(stderr, "there are OpenGL errors!\n");
		}
	}
	
	void loop() {
		int frame_counter = 0;
		Uint32 last_time = SDL_GetTicks();
		while(input.handle()) {
			graphics.render();
			context.swap();
			++frame_counter;
			Uint32 time = SDL_GetTicks();
			if(time - last_time > 1000) {
				last_time = time;
				window.setTitle("FPS: " + std::to_string(frame_counter));
				frame_counter = 0;
			}
		}
	}
	
	Storage *getStorage() {
		return &storage;
	}
	
	Graphics *getGraphics() {
		return &graphics;
	}
	
	Input *getInput() {
		return &input;
	}
};
