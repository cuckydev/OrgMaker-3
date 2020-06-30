#pragma once

/*
Project: OrgMaker 3

File: src/Task_EditorWindow.h
Purpose: Declare the editor task class

Authors: Regan "cuckydev" Green
*/

//OrgMaker classes
#include "ContentProvider.h"
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
		
		//Content provider
		const ContentProvider *content_provider = nullptr;
		
		//Organya instance
		Organya::Instance organya;
		
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
		Task_EditorWindow(const ContentProvider *_content_provider);
		~Task_EditorWindow();
		
		//Task interface
		bool PushEvent(const SDL_Event *event);
};
