#pragma once

/*
Project: OrgMaker 3

File: src/Window.h
Purpose: Define the window class for tasks

Authors: Regan "cuckydev" Green
*/

//Library
#include "SDL_render.h"
#include "SDL_events.h"

//Standard library
#include <iostream>

//OrgMaker classes
#include "Error.h"

//Window event results
enum Window_EventResult
{
	Window_EventResult_None,
	Window_EventResult_Close,
	Window_EventResult_Resized,
	Window_EventResult_Redraw,
};

//Window class
class Window
{
	private:
		//Error
		Error error;
		
	public:
		//Window and renderer
		SDL_Window *window = nullptr;
		int width, height;
		SDL_Renderer *renderer = nullptr;
		
	public:
		//Constructor and destructor
		Window(std::string name, const SDL_Rect &rect, unsigned long flags) : width(rect.w), height(rect.h)
		{
			//Create window and renderer
			if ((window = SDL_CreateWindow(name.c_str(), rect.x, rect.y, rect.w, rect.h, flags)) == nullptr ||
				(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC)) == nullptr)
			{
				error.Push(SDL_GetError());
				return;
			}
		}
		
		~Window()
		{
			//Delete window and renderer
			if (renderer != nullptr)
				SDL_DestroyRenderer(renderer);
			if (window != nullptr)
				SDL_DestroyWindow(window);
		}
		
		//Window interface
		Window_EventResult PushEvent(const SDL_Event *event)
		{
			if (event->type == SDL_WINDOWEVENT && event->window.windowID == SDL_GetWindowID(window))
			{
				switch (event->window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
						return Window_EventResult_Close;
					case SDL_WINDOWEVENT_RESIZED:
						width = event->window.data1;
						height = event->window.data2;
						return Window_EventResult_Resized;
					case SDL_WINDOWEVENT_EXPOSED:
						return Window_EventResult_Redraw;
					default:
						break;
				}
			}
			
			return Window_EventResult_None;
		}
		
		//Get error
		const Error &GetError() const { return error; }
};
