/*
Project: OrgMaker 3

File: src/Main.cpp
Purpose: Provide the entry point for the program

Authors: Regan "cuckydev" Green
*/

//Library
#include <SDL.h>

//Standard library
#include <iostream>
#include <string>

//OrgMaker classes
#include "Task_Editor.h"

//Show error
void ShowError(std::string error)
{
	//Show a message box explaining the error
	std::cout << error << std::endl;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr);
}

//Entry point
int main(int argc, char *argv[])
{
	std::string error;
	
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
	{
		//SDL initialization error
		error = std::string(SDL_GetError());
	}
	else
	{
		//Create editor task
		Task *task = new Task_Editor();
		if (task == nullptr || task->GetError())
		{
			//Task creation error
			error = (task != nullptr ? task->GetError() : std::string("Failed to create editor task"));
		}
		else
		{
			//Enter event loop
			SDL_Event event;
			while (1)
			{
				//Wait for next event and send to editor task
				SDL_WaitEvent(&event);
				if (task->PushEvent(&event))
					break;
			}
			
			//Check if task errored
			if (task->GetError())
				error = (std::string)task->GetError();
			
			//Delete task and quit SDL
			delete task;
			SDL_Quit();
		}
	}
	
	//If an error occured, show it to the user and return -1, otherwise return 0
	if (error.empty())
		return 0;
	ShowError(error);
	return -1;
}
