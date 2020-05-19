/*
Project: OrgMaker 3

File: src/Task_EditorWindow.cpp
Purpose: Define the editor task class

Authors: Regan "cuckydev" Green
*/

//Declaration
#include "Task_EditorWindow.h"

Task_EditorWindow::Task_EditorWindow()
{
	//Create window
	if ((window = new Window(	"OrgMaker 3 - Untitled",
								{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 854, 480},
								SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE)) == nullptr ||
		window->GetError())
	{
		error.Push(window->GetError());
		return;
	}
}

Task_EditorWindow::~Task_EditorWindow()
{
	//Delete window
	if (window != nullptr)
		delete window;
}

bool Task_EditorWindow::PushEvent(const SDL_Event *event)
{
	//Push event to window
	Window_EventResult window_result = window->PushEvent(event);
	
	switch (window_result)
	{
		case Window_EventResult_Close:
			return true;
		case Window_EventResult_Redraw:
		{
			//Clear background
			SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0xFF);
			SDL_RenderClear(window->renderer);
			
			//Update placement rects
			SDL_Rect window_rect = {0, 0, window->width, window->height};
			topbar_rect = editor.GetRect(window_rect);
			editor_rect = editor.GetRect(window_rect);
			
			//Present renderer BITCH
			SDL_RenderPresent(window->renderer);
			break;
		}
		default:
			break;
	}
	return false;
}
