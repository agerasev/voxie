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
		while(input.handle()) {
			graphics.render();
			context.swap();
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
