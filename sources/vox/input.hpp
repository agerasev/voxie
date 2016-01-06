#pragma once

#include "window.hpp"
#include "storage.hpp"
#include "graphics.hpp"

class Input {
public:
	class MouseListener {
	public:
		virtual void down(int b, int x, int y) = 0;
		virtual void up(int b, int x, int y) = 0;
		virtual void move(int x, int y) = 0;
	};
	
private:
	Window *window;
	Storage *storage;
	Graphics *graphics;
	MouseListener *mouse_listener = nullptr;
	
	int mousebits = 0;
	int movebits = 0;
	Uint32 last_tick = 0;
	
public:
	Input(Window *w, Storage *s, Graphics *g)
	  : window(w), storage(s), graphics(g)
	{
		last_tick = SDL_GetTicks();
		//SDL_SetRelativeMouseMode((SDL_bool)mouse);
	}
	~Input() {
		
	}
	
	void setMouseListener(MouseListener *ml) {
		mouse_listener = ml;
	}
	
	bool handle() {
		Camera &cam = storage->cam;
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_MOUSEMOTION) {
				if(mousebits & (1<<2)) {
					cam.move(event.motion.xrel, event.motion.yrel);
				}
				if(mouse_listener != nullptr) mouse_listener->move(event.motion.x, event.motion.y);
				// graphics->highlight(event.button.x, event.button.y);
			} else if(event.type == SDL_MOUSEBUTTONDOWN) {
				int mx = event.button.x, my = event.button.y;
				if(event.button.button == SDL_BUTTON_MIDDLE) {
					mousebits |= (1<<2);
					SDL_SetRelativeMouseMode((SDL_bool)true);
					if(mouse_listener != nullptr) mouse_listener->down(2, mx, my);
				} else if(event.button.button == SDL_BUTTON_LEFT) {
					mousebits |= (1<<0);
					if(mouse_listener != nullptr) mouse_listener->down(0, mx, my);
					//graphics->remove(mx, my);
					//graphics->highlight(mx,my);
				} else if(event.button.button == SDL_BUTTON_RIGHT) {
					mousebits |= (1<<1);
					if(mouse_listener != nullptr) mouse_listener->down(1, mx, my);
					//graphics->add(mx, my);
					//graphics->highlight(mx,my);
				}
			} else if(event.type == SDL_MOUSEBUTTONUP) {
				int mx = event.button.x, my = event.button.y;
				if(event.button.button == SDL_BUTTON_MIDDLE) {
					mousebits &= ~(1<<2);
					if(mouse_listener != nullptr) mouse_listener->up(2, mx, my);
					SDL_SetRelativeMouseMode((SDL_bool)false);
				} else if(event.button.button == SDL_BUTTON_LEFT) {
					mousebits &= ~(1<<0);
					if(mouse_listener != nullptr) mouse_listener->up(0, mx, my);
				} else if(event.button.button == SDL_BUTTON_RIGHT) {
					mousebits &= ~(1<<1);
					if(mouse_listener != nullptr) mouse_listener->up(1, mx, my);
				}
			} else if(event.type == SDL_MOUSEWHEEL) {
				//if(event.wheel.y)
				//	graphics->get_camera()->zoom(-event.wheel.y);
			} else if(event.type == SDL_QUIT) {
				return false;
			} else if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					return false;
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
					window->width = event.window.data1; 
					window->height = event.window.data2;
					graphics->resize(window->width, window->height);
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
			fvec3 dir = cam.dir;
			md = transpose(fmat3(dir, normalize(cross(dir, fvec3(0,0,1))), fvec3(0,0,1)))*md;
			md *= spd*0.001*(tick - last_tick);
			cam.pos += md;
			cam.update_view();
		}
		last_tick = tick;
		
		return true;
	}
};
