/*
Project: OrgMaker 3

File: src/Organya.cpp
Purpose: Define the Organya song class

Authors: Regan "cuckydev" Green, Daisuke "Pixel" Amaya
*/

//Standard library
#include <iostream>

//Declaration
#include "Organya.h"

//Organya namespace
namespace Organya
{
	//Instrument class
	//Destructor
	Instrument::~Instrument()
	{
		//Delete linked list
		while (event != nullptr)
		{
			event = event->next;
			if (event != nullptr)
				delete event->prev;
			else
				break;
		}
	}
	
	//Instrument interface
	void Instrument::SetPosition(uint32_t _x)
	{
		//If event point is null, search from beginning
		if (event_point == nullptr)
		{
			event_point = event;
			x = 0;
		}
		
		if (_x > x)
		{
			//Move forward until we find the event to play
			while (event_point != nullptr && event_point->x < _x)
				event_point = event_point->next;
		}
		else if (_x < x)
		{
			//Move backward until we find the event to play
			while (event_point != nullptr && event_point->x > _x)
				event_point = event_point->prev;
			if (event_point == nullptr)
				event_point = event; //If behind all events, set event pointer to first event
		}
		
		//Remember given position
		x = _x;
	}
	
	void Instrument::GetState()
	{
		//Don't do anything if no event could be played
		if (event_point == nullptr)
			return;
		
		//Reset state
		event_state = {};
		
		//If we're behind the event (first event), don't play anything
		if (x < event_point->x || event_point->prev == nullptr)
			return;
		
		//Go backward filling last event parameters until we hit a note
		Event *event_check = event_point;
		
		while (event_check != nullptr)
		{
			//Update volume
			if (event_check->volume != 0xFF)
				event_state.volume = event_check->volume;
			
			//Update volume
			if (event_check->pan != 0xFF)
				event_state.pan = event_check->pan;
			
			//Update key
			if (event_check->y != 0xFF)
			{
				event_state.y = event_check->y;
				event_state.length = event_check->length - (x - event_check->x);
				break;
			}
			
			//Check previous event
			event_check = event_check->prev;
		}
	}
	
	void Instrument::PlayData()
	{
		//Don't do anything if no event could be played
		if (event_point == nullptr)
			return;
		
		//Play current event if x is equal to the event's x
		if (event_point->x == x)
		{
			//Change key
			if (event_point->y != 0xFF)
			{
				//Start playing note
				event_state.y = event_point->y;
				std::cout << " set Y to " << (int)event_state.y;
			}
			
			//Change volume
			if (event_point->volume != 0xFF)
			{
				event_state.volume = event_point->volume;
				std::cout << " set volume to " << (int)event_state.volume;
			}
			
			//Change panning
			if (event_point->pan != 0xFF)
			{
				event_state.pan = event_point->pan;
				std::cout << " set pan to " << (int)event_state.pan;
			}
		}
		else if (event_state.length > 0)
		{
			//Decrease event playing length
			if (--event_state.length == 0)
			{
				//Stop playing note
			}
		}
	}
	
	//Organya class
	//Destructor
	Organya::~Organya()
	{
		
	}
	
	//Loading
	struct VersionLUT
	{
		std::string name;
		Version version;
	};
	
	static const std::array<VersionLUT, 3> version_lut{{
		{"Org-01", OrgV01},
		{"Org-02", OrgV02},
		{"Org-03", OrgV03},
	}};
	
	bool Organya::ReadInstrument(std::istream &stream, Instrument &i, uint16_t &note_num)
	{
		//Read instrument data
		i.freq = ReadLE16(stream);
		i.wave_no = stream.get();
		i.pipi = stream.get();
		if (header.version == OrgV01)
			i.pipi = false; //OrgV01 has no pipi, but the byte is still there
		note_num = ReadLE16(stream);
		return false;
	}
	
	bool Organya::ReadEvents(std::istream &stream, Instrument &i, uint16_t note_num)
	{
		//Construct note linked list
		Event *event;
		Event *prev = nullptr;
		
		for (uint16_t v = 0; v < note_num; v++)
		{
			//Create a new event instance and link it to the list
			if ((event = new Event) == nullptr)
				return error.Push("Failed to create an event instance");
			if ((event->prev = prev) != nullptr)
				prev->next = event; //Link us to previous event
			else
				i.event = event; //Be head of instrument events if first
			prev = event;
		}
		
		//Read data into notes
		for (event = i.event; event != nullptr; event = event->next)
			event->x = ReadLE32(stream);
		for (event = i.event; event != nullptr; event = event->next)
			event->y = stream.get();
		for (event = i.event; event != nullptr; event = event->next)
			event->length = stream.get();
		for (event = i.event; event != nullptr; event = event->next)
			event->volume = stream.get();
		for (event = i.event; event != nullptr; event = event->next)
			event->pan = stream.get();
		return false;
	}
	
	bool Organya::Load(std::istream &stream)
	{
		//Read magic and version
		std::string magic = ReadString<6>(stream);
		for (auto &i : version_lut)
		{
			if (i.name == magic)
			{
				header.version = i.version;
				break;
			}
		}
		
		//Read rest of header
		header.wait = ReadLE16(stream);
		header.line = stream.get();
		header.dot = stream.get();
		header.repeat_x = ReadLE32(stream);
		header.end_x = ReadLE32(stream);
		
		//Read instrument data
		uint16_t note_num[16];
		uint16_t *note_num_p = note_num;
		
		for (auto &i : melody)
			if (ReadInstrument(stream, i, *note_num_p++))
				return true;
		for (auto &i : drum)
			if (ReadInstrument(stream, i, *note_num_p++))
				return true;
		
		//Read event data
		note_num_p = note_num;
		for (auto &i : melody)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		for (auto &i : drum)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		return false;
	}
	
	bool Organya::Load(std::string _path)
	{
		//Get the stream that represents the path given, and remember the path for later saving
		std::ifstream stream(_path);
		path = _path;
		if (stream.is_open())
			return Load(stream);
		else
			return error.Push("Failed to open " + path);
	}
}
