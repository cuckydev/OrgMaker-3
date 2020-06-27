/*
Project: OrgMaker 3

File: src/Organya.cpp
Purpose: Define Organya classes

Authors: Regan "cuckydev" Green
*/

//Standard library
#include <iostream>

//Declaration
#include "Organya.h"

//Organya namespace
namespace Organya
{
	//Audio config
	static const int audio_frequency = 48000;
	static const uint16_t audio_frames = 0x200;
	
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
			if ((event_point = event) == nullptr)
				return;
			x = 0;
		}
		
		if (_x > x)
		{
			//Move forward until we find the event to play
			while (event_point->next != nullptr && _x >= event_point->next->x)
				event_point = event_point->next;
		}
		else if (_x < x)
		{
			//Move backward until we find the event to play
			while (event_point->prev != nullptr && _x < event_point->x)
				event_point = event_point->prev;
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
		event_state_prev = event_state;
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
		if (x == event_point->x)
		{
			//Change key
			if (event_point->y != 0xFF)
			{
				//Start playing instrument
				event_state.y = event_point->y;
				event_state.length = event_point->length;
				Play();
			}
			
			//Change volume
			if (event_point->volume != 0xFF)
				event_state.volume = event_point->volume;
			
			//Change panning
			if (event_point->pan != 0xFF)
				event_state.pan = event_point->pan;
			
			//Update instrument
			Update();
		}
		
		if (event_state.length == 0)
			Stop();
		if (event_state.length > 0)
			event_state.length--;
	}
	
	//Melody class
	static const struct
	{
		int16_t wave_size;
		int16_t oct_par;
		int16_t oct_size;
	} oct_wave[8] =
	{
		{256, 1,   4 }, // 0 Oct
		{256, 2,   8 }, // 1 Oct
		{128, 4,   12}, // 2 Oct
		{128, 8,   16}, // 3 Oct
		{64,  16,  20}, // 4 Oct
		{32,  32,  24}, // 5 Oct
		{16,  64,  28}, // 6 Oct
		{8,   128, 32}, // 7 Oct
	};
	
	#include "rWave.h"
	
	bool Melody::ConstructBuffers()
	{
		for (size_t i = 0; i < 8; i++)
		{
			for (size_t v = 0; v < 2; v++)
			{
				//Get sizes
				size_t wave_size = oct_wave[i].wave_size, data_size = wave_size * (pipi ? oct_wave[i].oct_size : 1);
				
				//Allocate buffer data
				float *data = new float[data_size * 2];
				float *datap = data;
				
				//Get wave position
				const uint8_t *wave = rWave + 0x100 * wave_no;
				
				//Fill buffer data
				size_t wave_tp = 0;
				for (size_t j = 0; j < data_size; j++)
				{
					*datap++ = ((float)wave[wave_tp] - 0x80) / 0x100;
					*datap++ = ((float)wave[wave_tp] - 0x80) / 0x100;
					
					//Move forward in wave
					wave_tp = (wave_tp + (0x100 / wave_size)) & 0xFF;
				}
				
				//Create buffer
				buffer[i][v].SetData(data, data_size, 0);
			}
		}
		return false;
	}
	
	void Melody::StopBuffers()
	{
		for (size_t i = 0; i < 8; i++)
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].Stop();
	}
	
	void Melody::Mix(float *stream, int stream_frequency, size_t stream_frames)
	{
		for (size_t i = 0; i < 8; i++)
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].Mix(stream, stream_frequency, stream_frames);
	}
	
	void Melody::Play()
	{
		//Stop current buffer and switch to second one
		Stop();
		twin = !twin;
		current_buffer = &buffer[event_state.y / 12][twin];
		current_buffer->Play();
		current_buffer->SetLoop(!pipi);
	}
	
	static const int16_t freq_tbl[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
	static const int16_t pan_tbl[13] = {0, 43, 86, 129, 172, 215, 256, 297, 340, 383, 426, 469, 512};
	
	void Melody::Update()
	{
		//If current buffer hasn't been decided yet or instrument isn't playing, don't update
		if (current_buffer == nullptr || event_state.y == 0xFF)
			return;
		
		if (event_state.y != event_state_prev.y)
		{
			//Update frequency for.. every buffer? WHAT? WHY?!
			for (size_t i = 0; i < 8; i++)
				for (size_t v = 0; v < 2; v++)
					buffer[i][v].SetFrequency(((oct_wave[i].wave_size * freq_tbl[event_state.y % 12]) * oct_wave[i].oct_par) / 8 + (freq - 1000));
		}
		
		//Update volume and panning for current buffer
		current_buffer->SetVolume((event_state.volume - 0xFF) * 8);
		current_buffer->SetPan(pan_tbl[event_state.pan]);
	}
	
	void Melody::Stop()
	{
		//If current buffer hasn't been decided yet, don't stop
		if (current_buffer == nullptr)
			return;
		current_buffer->SetLoop(false);
	}
	
	//Drum class
	bool Drum::ConstructBuffers()
	{
		return false;
	}
	
	void Drum::StopBuffers()
	{
		
	}
	
	void Drum::Mix(float *stream, int stream_frequency, size_t stream_frames)
	{
		buffer.Mix(stream, stream_frequency, stream_frames);
	}
	
	void Drum::Play()
	{
		
	}
	
	void Drum::Update()
	{
		
	}
	
	void Drum::Stop()
	{
		
	}
	
	//Organya instance class
	//Destructor
	Instance::~Instance()
	{
		//Stop audio before destroying instruments
		audio.Pause();
		return;
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
	
	bool Instance::ReadInstrument(std::istream &stream, Instrument &i, uint16_t &note_num)
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
	
	bool Instance::ReadEvents(std::istream &stream, Instrument &i, uint16_t note_num)
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
	
	bool Instance::Load(std::istream &stream)
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
		
		//Construct instrument buffers
		for (auto &i : melody)
			if (i.ConstructBuffers())
				return true;
		for (auto &i : drum)
			if (i.ConstructBuffers())
				return true;
		
		//Read event data
		note_num_p = note_num;
		for (auto &i : melody)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		for (auto &i : drum)
			if (ReadEvents(stream, i, *note_num_p++))
				return true;
		
		//Start at position 0
		SetPosition(0);
		return false;
	}
	
	bool Instance::Load(std::string _path)
	{
		//Get the stream that represents the path given, and remember the path for later saving
		std::ifstream stream(_path);
		path = _path;
		if (stream.is_open())
			return Load(stream);
		else
			return error.Push("Failed to open " + path);
	}
	
	//Organya interface
	bool Instance::SetPosition(uint32_t _x)
	{
		//Lock audio
		if (audio.Lock())
			return error.Push(audio.GetError());
		
		//Set all instrument positions
		x = _x;
		
		for (auto &i : melody)
			i.SetPosition(x);
		for (auto &i : drum)
			i.SetPosition(x);
		
		for (auto &i : melody)
			i.GetState();
		for (auto &i : drum)
			i.GetState();
		
		//Reset step frames counter
		step_frames_counter = -1.0L;
		
		//Unlock audio
		if (audio.Unlock())
			return error.Push(audio.GetError());
		return false;
	}
	
	bool Instance::Play()
	{
		if (!playing)
		{
			//Start playing
			playing = true;
			SetPosition(x);
			
			//Resume audio
			if (audio.Resume())
				return error.Push(audio.GetError());
		}
		return false;
	}
	
	bool Instance::Stop()
	{
		if (playing)
		{
			//Pause audio
			if (audio.Pause())
				return error.Push(audio.GetError());
			
			//Stop playing
			playing = false;
			for (auto &i : melody)
				i.StopBuffers();
			for (auto &i : drum)
				i.StopBuffers();
		}
		return false;
	}
	
	//Audio
	void Instance::AudioCallback(const Audio::Config<Instance*> *config, uint8_t *stream)
	{
		//Update Organya
		step_frames = (long double)header.wait / 1000.0L * (long double)config->frequency;
		
		while (step_frames_counter < 0.0L || (step_frames_counter += config->frames) >= step_frames)
		{
			//Increment song position and update instruments
			if (step_frames_counter < 0.0L)
				step_frames_counter = 0.0L;
			else
				step_frames_counter -= step_frames;
			
			for (auto &i : melody)
				i.SetPosition(x);
			for (auto &i : drum)
				i.SetPosition(x);
			
			for (auto &i : melody)
				i.PlayData();
			for (auto &i : drum)
				i.PlayData();
			
			if (++x >= header.end_x)
				x = header.repeat_x;
		}
		
		//Clear stream
		float *streamf = (float*)stream;
		for (size_t i = 0; i < config->frames * 2; i++)
			*streamf++ = 0.0f;
		
		//Mix instruments
		streamf = (float*)stream;
		for (auto &i : melody)
			i.Mix(streamf, config->frequency, config->frames);
		//for (auto &i : drum)
		//	i.Mix(streamf, config->frequency, config->frames);
	}
	
	void MiddleAudioCallback(const Audio::Config<Instance*> *config, uint8_t *stream)
	{
		//Use the callback from the specific instance in the config
		config->userdata->AudioCallback(config, stream);
	}
	
	bool Instance::InitializeAudio()
	{
		//Construct audio config
		Audio::Config<Instance*> config
		{
			this, //userdata
			audio_frequency, //frequency
			audio_frames, //frames
			MiddleAudioCallback, //callback
		};
		
		//Send config to audio instance
		if (audio.SetConfig(config))
			return true;
		return false;
	}
}
