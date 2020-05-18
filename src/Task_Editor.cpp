/*
Project: OrgMaker 3

File: src/Task_Editor.cpp
Purpose: Define the editor task class

Authors: Regan "cuckydev" Green
*/

//Declaration
#include "Task_Editor.h"

Task_Editor::Task_Editor()
{
	//Create window
	window = new Window("OrgMaker 3 - Untitled", {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 854, 480}, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
}

Task_Editor::~Task_Editor()
{
	//Delete window
	delete window;
}

bool Task_Editor::PushEvent(const SDL_Event *event)
{
	return event->type == SDL_QUIT;
}
