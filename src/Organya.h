#pragma once

/*
Project: OrgMaker 3

File: src/Organya.h
Purpose: Declare Organya classes

Authors: Regan "cuckydev" Green
*/

//Standard library
#include <cstdint>
#include <string>
#include <fstream>
#include <array>

//OrgMaker classes
#include "Error.h"
#include "Audio.h"

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
			
		protected:
			//Instrument state
			Event *event_point = nullptr; //Pointer to the current event being played
			Event event_state, event_state_prev; //Current and previous event state (key, volume, and pan are changed individually)
			
			//Playback state
			uint32_t x;
			
		public:
			//Destructor
			~Instrument();
			
			//Instrument interface
			virtual bool ConstructBuffers() = 0;
			virtual void StopBuffers() = 0;
			virtual void Mix(float *stream, int stream_frequency, size_t stream_frames) = 0;
			
			void SetPosition(uint32_t _x);
			void GetState();
			void PlayData();
			
		private:
			//Internal instrument interface
			virtual void Play() = 0;
			virtual void Update() = 0;
			virtual void Stop() = 0;
	};

	class Melody : public Instrument
	{
		private:
			//Sound buffers for playback
			Audio::Buffer buffer[8][2];
			Audio::Buffer *current_buffer = nullptr;
			bool twin = false;
			
		public:
			//Instrument interface
			bool ConstructBuffers();
			void StopBuffers();
			void Mix(float *stream, int stream_frequency, size_t stream_frames);
			
		private:
			//Internal instrument interface
			void Play();
			void Update();
			void Stop();
	};

	class Drum : public Instrument
	{
		private:
			//Sound buffer for playback
			Audio::Buffer buffer;
			
		public:
			//Instrument interface
			bool ConstructBuffers();
			void StopBuffers();
			void Mix(float *stream, int stream_frequency, size_t stream_frames);
			
		protected:
			//Internal instrument interface
			void Play();
			void Update();
			void Stop();
	};
	
	//Organya instance class
	class Instance
	{
		private:
			//Error
			Error error;
			
			//Path to save song to
			std::string path;
			
			//Audio instance
			Audio::Instance<Instance*> audio;
			
			//Organya data
			Header header;
			std::array<Melody, 8> melody = {};
			std::array<Drum, 8> drum = {};
			
			//Playback state
			bool playing = false;
			double step_frames, step_frames_counter;
			uint32_t x;
			
		public:
			//Constructor and destructor
			Instance() { InitializeAudio(); }
			Instance(std::istream &stream) { Load(stream); InitializeAudio(); }
			Instance(std::string _path) { Load(_path); InitializeAudio(); }
			~Instance();
			
			//Loading
			bool Load(std::istream &stream);
			bool Load(std::string _path);
			
			//Organya interface
			bool SetPosition(uint32_t x);
			bool Play();
			bool Stop();
			
			//Get error
			const Error &GetError() const { return error; }
			
		private:
			//Audio
			void AudioCallback(const Audio::Config<Instance*> *config, uint8_t *stream);
			friend void MiddleAudioCallback(const Audio::Config<Instance*> *config, uint8_t *stream);
			bool InitializeAudio();
			
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
