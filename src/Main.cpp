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
	int result = 0;
	
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0)
	{
		result = -1;
		ShowError(std::string(SDL_GetError()));
	}
	else
	{
		//Create editor task and run
		Task *task = new Task_Editor();
		if (task == nullptr || task->GetError())
		{
			result = -1;
			ShowError(task != nullptr ? task->GetError() : std::string("Failed to create editor task"));
		}
		
		//Quit SDL
		SDL_Quit();
	}
	
	return result;
}
