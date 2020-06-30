/*
Project: OrgMaker 3

File: src/ContentProvider.cpp
Purpose: Define content provider class

Authors: Regan "cuckydev" Green
*/

//Library
#include "SDL_filesystem.h"

//Declaration
#include "ContentProvider.h"

//Content provider class
//Constructor
ContentProvider::ContentProvider()
{
	//Get base path through SDL
	char *sdl_base_path = SDL_GetBasePath();
	if (sdl_base_path != nullptr)
	{
		//Convert base path to std::string and free base path
		base_path = std::string(sdl_base_path);
		SDL_free(sdl_base_path);
	}
	else
	{
		//Default if SDL_GetBasePath failed
		base_path = "./";
	}
}

//Content provider interface
std::ifstream ContentProvider::OpenIn(std::string path, std::ifstream::openmode openmode = std::ifstream::in) const
{
	std::ifstream stream(base_path + path, openmode);
	return stream;
}

std::ofstream ContentProvider::OpenOut(std::string path, std::ofstream::openmode openmode = std::ofstream::out) const
{
	std::ofstream stream(base_path + path, openmode);
	return stream;
}
