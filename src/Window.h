#pragma once

/*
Project: OrgMaker 3

File: src/Window.h
Purpose: Define the window class for tasks

Authors: Regan "cuckydev" Green
*/

//Library
#include "SDL_render.h"

//Standard library
#include <iostream>

//OrgMaker classes
#include "Error.h"

//Window class
class Window
{
	private:
		//Error
		Error error;
		
	public:
		//Window and renderer
		SDL_Window *window = nullptr;
		SDL_Renderer *renderer = nullptr;
		
	public:
		//Constructor and destructor
		Window(std::string name, const SDL_Rect &rect, unsigned long flags)
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
		
		//Get error
		const Error &GetError() const { return error; }
};
