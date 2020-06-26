#pragma once

/*
Project: OrgMaker 3

File: src/Organya.h
Purpose: Declare the Organya song class

Authors: Regan "cuckydev" Green, Daisuke "Pixel" Amaya
*/

//Standard library
#include <cstdint>
#include <string>
#include <fstream>
#include <array>

//OrgMaker classes
#include "Error.h"

//Organya namespace
namespace Organya
{
	//Organya types
	enum Version
	{
		OrgV01, //Original
		OrgV02, //Added pipi
		OrgV03, //Extra drums
	};

	struct Header
	{
		Version version;	//Organya version
		uint16_t wait;				//Tempo
		uint8_t line, dot;			//Time signature
		uint32_t repeat_x;			//Repeat
		uint32_t end_x;				//End of song (Return to repeat)
	};

	struct Event
	{
		//Event information
		uint32_t x = 0;			//Position
		uint8_t length = 0;		//Length
		uint8_t y = 0xFF;		//Key
		uint8_t volume = 0xFF;	//Volume
		uint8_t pan = 0xFF;		//Panning
		
		//Next and previous event (double sided linked list)
		Event *prev = nullptr;
		Event *next = nullptr;
	};

	class Instrument
	{
		public:
			//Instrument info
			uint16_t freq;		//Frequency offset (1000 is 0hz offset)
			uint8_t wave_no;	//Waveform number
			bool pipi;			//Pizzicato
			
			//Music data
			Event *event = nullptr;	//All the events that belong to this instrument
			
		private:
			//Instrument state
			Event *event_point = nullptr; //Pointer to the current event being played
			Event event_state; //Current and previous event state (key, volume, and pan are changed individually)
			
			//Playback state
			uint32_t x;
			
		public:
			//Destructor
			~Instrument();
			
			//Instrument interface
			void SetPosition(uint32_t _x);
			void GetState();
			void PlayData();
	};

	struct Melody : Instrument
	{
		//Sound buffers for playback
		
	};

	struct Drum : Instrument
	{
		//Sound buffer for playback
		
	};

	//Organya class
	class Organya
	{
		private:
			//Error
			Error error;
			
			//Path to save song to
			std::string path;
			
			//Organya data and state
			Header header;
			std::array<Melody, 8> melody = {};
			std::array<Drum, 8> drum = {};
			
		public:
			//Constructor and destructor
			Organya() {}
			Organya(std::istream &stream) { Load(stream); }
			Organya(std::string _path) { Load(_path); }
			~Organya();
			
			//Loading
			bool Load(std::istream &stream);
			bool Load(std::string _path);
			
			//Get error
			const Error &GetError() const { return error; }
			
		private:
			//Internal reading functions
			bool ReadInstrument(std::istream &stream, Instrument &i, uint16_t &note_num);
			bool ReadEvents(std::istream &stream, Instrument &i, uint16_t note_num);
			
			//Read string of specified length from file
			template<unsigned length> std::string ReadString(std::istream &stream)
			{
				char buf[length + 1] = {0};
				stream.read(buf, length);
				return std::string(buf);
			}
			
			//Read specific sizes from stream
			uint16_t ReadLE16(std::istream &stream)
			{ return ((uint16_t)stream.get()) | ((uint16_t)stream.get() << 8); }
			uint32_t ReadLE32(std::istream &stream)
			{ return ((uint32_t)stream.get()) | ((uint32_t)stream.get() << 8) | ((uint32_t)stream.get() << 16) | ((uint32_t)stream.get() << 24); }
	};
}
