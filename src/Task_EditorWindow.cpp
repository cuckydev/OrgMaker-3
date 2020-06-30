/*
Project: OrgMaker 3

File: src/Task_EditorWindow.cpp
Purpose: Define the editor task class

Authors: Regan "cuckydev" Green
*/

//Declaration
#include "Task_EditorWindow.h"

//Constructor and destructor
Task_EditorWindow::Task_EditorWindow(const ContentProvider *_content_provider, int argc, char *argv[]) : content_provider(_content_provider)
{
	//Organya initialization
	if (organya.GetError() || organya.InitializeData(content_provider))
	{
		error.Push(organya.GetError());
		return;
	}
	
	//Load or start new org
	if (argc > 1)
	{
		//Load, save, and play given org
		if (organya.Load(argv[1]))
		{
			error.Push(organya.GetError());
			return;
		}
		organya.Play();
	}
	
	//Create window
	if ((window = new Window(	"OrgMaker 3 - Untitled",
								{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 854, 480},
								0 | SDL_WINDOW_RESIZABLE)) == nullptr ||
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

//Task interface
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
			
			//Present renderer
			SDL_RenderPresent(window->renderer);
			break;
		}
		default:
			break;
	}
	return false;
}
