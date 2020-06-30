#pragma once

/*
Project: OrgMaker 3

File: src/ContentProvider.h
Purpose: Declare content provider class

Authors: Regan "cuckydev" Green
*/

//Standard library
#include <string>
#include <fstream>

//OrgMaker classes
#include "Error.h"

//Content provider class
class ContentProvider
{
	private:
		//Paths to reference
		std::string base_path;
		
	public:
		//Constructor and destructor
		ContentProvider();
		ContentProvider(std::string _base_path) : base_path(_base_path) {}
		
		//Content provider interface
		std::ifstream OpenIn(std::string path, std::ifstream::openmode openmode) const;
		std::ofstream OpenOut(std::string path, std::ofstream::openmode openmode) const;
};
