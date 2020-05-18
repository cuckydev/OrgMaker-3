#pragma once

/*
Project: OrgMaker 3

File: src/Task.h
Purpose: Define the task base class

Authors: Regan "cuckydev" Green
*/

//Library
#include "SDL_events.h"

//OrgMaker classes
#include "Error.h"

//Task base class
class Task
{
	protected:
		//Error
		Error error;
		
	public:
		//Virtual destructor
		virtual ~Task() {}
		
		//Window interface
		virtual bool PushEvent(const SDL_Event *event) = 0;
		
		//Get error
		const Error &GetError() const { return error; }
};
