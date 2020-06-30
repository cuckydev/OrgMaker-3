/*
Project: OrgMaker 3

File: src/Organya.cpp
Purpose: Define Organya classes

Authors: Regan "cuckydev" Green, Daisuke "Pixel" Amaya
*/

//Standard library
#include <algorithm>
#include <iostream>

//Resources
#include "rWave.h"

//Declaration
#include "Organya.h"

//Organya namespace
namespace Organya
{
	//Audio config
	static const int audio_frequency = 44100;
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
		
		//Get new event state
		GetState();
	}
	
	void Instrument::GetState()
	{
		//Don't do anything if no event could be played
		if (event_point == nullptr)
			return;
		
		//Reset state
		event_state_prev = event_state;
		event_state = {};
		
		//Go backward filling last event parameters until we hit a note
		Event *event_check = event_point;
		if (x < event_check->x)
			return;
		
		while (event_check != nullptr)
		{
			//Update volume
			if (event_check->volume != 0xFF && event_state.volume == 0xFF)
				event_state.volume = event_check->volume;
			
			//Update volume
			if (event_check->pan != 0xFF && event_state.pan == 0xFF)
				event_state.pan = event_check->pan;
			
			//Update key
			if (event_check->y != 0xFF)
			{
				event_state.x = event_check->x;
				event_state.y = event_check->y;
				if (x >= (event_check->x + event_check->length))
					event_state.length = 0;
				else
					event_state.length = event_check->length - (x - event_check->x);
				break;
			}
			
			//Check previous event
			event_check = event_check->prev;
		}
	}
	
	void Instrument::PlayState()
	{
		//Don't do anything if no event could be played
		if (event_point == nullptr)
			return;
		
		//If new event has played, update
		if (x == event_point->x)
		{
			if (event_point->y != 0xFF)
				Play();
			Update();
		}
		
		//If no event is playing, stop
		if (event_state.length == 0)
			Stop();
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
	
	bool Melody::ConstructBuffers()
	{
		for (size_t i = 0; i < 8; i++)
		{
			//Get sizes
			size_t wave_size = oct_wave[i].wave_size, data_size = wave_size * (pipi ? oct_wave[i].oct_size : 1);
			
			//Allocate buffer data
			float *data = new float[data_size];
			if (data == nullptr)
				return true;
			float *datap = data;
			
			//Get wave position
			const int8_t *wave = (const int8_t*)rWave + 0x100 * wave_no;
			
			//Fill buffer data
			size_t wave_tp = 0;
			for (size_t j = 0; j < data_size; j++)
			{
				*datap++ = (float)wave[wave_tp] / 128.0f;
				wave_tp = (wave_tp + (0x100 / wave_size)) & 0xFF;
			}
			
			//Create buffers
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].SetData(data, data_size, 22050);
			delete[] data;
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
		//Update frequency for.. every buffer? WHAT? WHY?!
		if (event_state.y != 0xFF)
		{
			for (size_t i = 0; i < 8; i++)
				for (size_t v = 0; v < 2; v++)
					buffer[i][v].SetFrequency(((oct_wave[i].wave_size * freq_tbl[event_state.y % 12]) * oct_wave[i].oct_par) / 8 + (freq - 1000));
		}
		
		//If current buffer hasn't been decided yet, don't update
		if (current_buffer == nullptr)
			return;
		
		//Update volume and panning for current buffer
		if (event_state.volume != 0xFF)
			current_buffer->SetVolume((event_state.volume - 0xFF) * 8);
		if (event_state.pan != 0xFF)
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
	static const std::string drum_name[] = {
		"Bass01",
		"Bass02",
		"Snare01",
		"Snare02",
		"Tom01",
		
		"HiClose",
		"HiOpen",
		"Crash",
		"Per01",
		"Per02",
		
		"Bass03",
		"Tom02",
		"Bass04",
		"Bass05",
		"Snare03",
		
		"Snare04",
		"HiClose02",
		"HiOpen02",
		"HiClose03",
		"HiOpen03",
		
		"Crash02",
		"RevSym01",
		"Ride01",
		"Tom03",
		"Tom04",
		
		"OrcDrm01",
		"Bell",
		"Cat",
		"Bass06",
		"Bass07",
		
		"Snare05",
		"Snare06",
		"Snare07",
		"Tom05",
		"HiOpen04",
		
		"HiClose04",
		"Clap01",
		"Pesi01",
		"Quick01",
		"Bass08",
		
		"Snare08",
		"HiClose05",
	};
	bool Drum::ConstructBuffers()
	{
		std::string name = drum_name[wave_no];
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){return std::toupper(c);});
		std::ifstream stream("build/Data/Resources/" + name, std::ifstream::ate | std::ifstream::binary);
		size_t size = stream.tellg();
		stream.seekg(0, std::ifstream::beg);
		float *data = new float[size];
		float *datap = data;
		for (size_t i = 0; i < size; i++)
			*datap++ = (float)(int8_t)((uint8_t)stream.get() - 0x80) / 128.0f;
		buffer.SetData(data, size, 22050);
		delete[] data;
		return false;
	}
	
	void Drum::StopBuffers()
	{
		buffer.Stop();
	}
	
	void Drum::Mix(float *stream, int stream_frequency, size_t stream_frames)
	{
		buffer.Mix(stream, stream_frequency, stream_frames);
	}
	
	void Drum::Play()
	{
		//Rewind and play buffer
		buffer.Play();
		buffer.SetLoop(false);
		buffer.SetPosition(0.0);
	}
	
	void Drum::Update()
	{
		//Update frequency, volume, and panning for current buffer
		if (event_state.y != 0xFF)
			buffer.SetFrequency(100 + event_state.y * 800);
		if (event_state.volume != 0xFF)
			buffer.SetVolume((event_state.volume - 0xFF) * 8);
		if (event_state.pan != 0xFF)
			buffer.SetPan(pan_tbl[event_state.pan]);
	}
	
	void Drum::Stop()
	{
		//Nothing
		return;
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
		
		//Reset step frames counter
		step_frames_counter = 0;
		
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
		//Clear stream
		float *streamf = (float*)stream;
		for (size_t i = 0; i < config->frames * 2; i++)
			*streamf++ = 0.0f;
		streamf = (float*)stream;
		
		//Update and mix Organya
		int step_frames = header.wait * config->frequency / 1000;
		int frames_done = 0;
		
		while (frames_done != config->frames)
		{
			if (step_frames_counter == 0)
			{
				//Reset step frames counter
				step_frames_counter = step_frames;
				
				//Update instruments
				for (auto &i : melody)
					i.SetPosition(x);
				for (auto &i : drum)
					i.SetPosition(x);
				
				for (auto &i : melody)
					i.PlayState();
				for (auto &i : drum)
					i.PlayState();
				
				//Increment position in song
				if (++x >= header.end_x)
					x = header.repeat_x;
			}
			
			//Get frames to do
			int frames_to_do = config->frames - frames_done;
			if (frames_to_do > step_frames_counter)
				frames_to_do = step_frames_counter;
			
			//Mix instruments
			for (auto &i : melody)
				i.Mix(streamf, config->frequency, frames_to_do);
			for (auto &i : drum)
				i.Mix(streamf, config->frequency, frames_to_do);
			streamf += frames_to_do * 2;
			frames_done += frames_to_do;
			step_frames_counter -= frames_to_do;
		}
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
