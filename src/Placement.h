#pragma once

/*
Project: OrgMaker 3

File: src/Placement.h
Purpose: Declare placement struct for component placement

Authors: Regan "cuckydev" Green
*/

//OrgMaker classes
#include "Window.h"

//Component structures
struct Placement
{
	//Placement
	struct
	{
		struct
		{
			int offset;
			float scale;
		} x, y;
	} position, size;
	
	struct
	{
		float x, y;
	} anchor;
	
	//Common functions
	SDL_Rect GetRect(const SDL_Rect &container) const
	{
		int width = container.w * size.x.scale + size.x.offset;
		int height = container.h * size.y.scale + size.y.offset;
		return {
			container.x + (int)(container.w * position.x.scale + position.x.offset) - (int)(width * anchor.x),
			container.y + (int)(container.h * position.y.scale + position.y.offset) - (int)(height * anchor.y),
			width,
			height,
		};
	}
	
	bool PointInside(const SDL_Rect &container, int x, int y) const
	{
		SDL_Rect rect = GetRect(container);
		if (x < rect.x || x >= (rect.x + rect.w) ||
			y < rect.y || y >= (rect.y + rect.h))
			return false;
		return true;
	}
};
