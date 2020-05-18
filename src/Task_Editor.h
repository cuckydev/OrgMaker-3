#pragma once

/*
Project: OrgMaker 3

File: src/Task_Editor.h
Purpose: Declare the editor task class

Authors: Regan "cuckydev" Green
*/

//OrgMaker classes
#include "Window.h"

//Base class
#include "Task.h"

//Editor window class
class Task_Editor : public Task
{
	private:
		//Window
		Window *window = nullptr;
		
	public:
		//Constructor and destructor
		Task_Editor();
		~Task_Editor();
		
		//Window interface
		bool PushEvent(const SDL_Event *event);
};
