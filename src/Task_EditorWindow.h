#pragma once

/*
Project: OrgMaker 3

File: src/Task_EditorWindow.h
Purpose: Declare the editor task class

Authors: Regan "cuckydev" Green
*/

//OrgMaker classes
#include "Window.h"
#include "Organya.h"
#include "Placement.h"

//Base class
#include "Task.h"

//Editor window class
class Task_EditorWindow : public Task
{
	private:
		//Window
		Window *window = nullptr;
		
		//Organya
		Organya organya;
		
		//Area placements
		Placement topbar = {
			{{0, 0.0f}, {0, 0.0f}},
			{{0, 1.0f}, {16, 0.0f}},
			{0.0f, 0.0f},
		};
		SDL_Rect topbar_rect;
		
		Placement editor = {
			{{0, 0.0f}, {0, 1.0f}},
			{{0, 1.0f}, {-16, 1.0f}},
			{0.0f, 1.0f},
		};
		SDL_Rect editor_rect;
		
	public:
		//Constructor and destructor
		Task_EditorWindow();
		~Task_EditorWindow();
		
		//Task interface
		bool PushEvent(const SDL_Event *event);
};
