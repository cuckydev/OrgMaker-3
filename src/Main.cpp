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
#include "ContentProvider.h"
#include "Task_EditorWindow.h"

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
		//Initialize content provider
		ContentProvider content_provider;
		
		//Create editor window task
		Task *task = new Task_EditorWindow(&content_provider, argc, argv);
		if (task == nullptr || task->GetError())
		{
			//Task creation error
			error = (task != nullptr ? task->GetError() : std::string("Failed to create editor window task"));
		}
		else
		{
			//Enter event loop
			SDL_Event event;
			do
			{
				//Wait for next event before pushing to task
				SDL_WaitEvent(&event);
			}
			while (!task->PushEvent(&event));
			
			//Check if task errored
			if (task->GetError())
				error = (std::string)task->GetError();
			
			//Delete task
			delete task;
		}
		
		//Quit SDL
		SDL_Quit();
	}
	
	//If an error occured, show it to the user and return -1, otherwise return 0
	if (error.empty())
		return 0;
	std::cout << error << std::endl;
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr); //Yes you can call SDL_ShowSimpleMessageBox without SDL_Init and after SDL_Quit
	return -1;
}
