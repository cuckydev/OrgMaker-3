/*
Project: OrgMaker 3

File: src/Organya.cpp
Purpose: Define Organya classes

Authors: Regan "cuckydev" Green, Daisuke "Pixel" Amaya
*/

//Standard library
#include <algorithm>
#include <iostream>

//Declaration
#include "Organya.h"

//Organya namespace
namespace Organya
{
	//Audio config
	static const int audio_frequency = 44100;
	static const uint16_t audio_frames = 0x200;
	
	//Organya constants
	const std::string default_path = "NewData.org";
	
	static const std::string drum_name[] = {
		"Bass01",
		"Bass02",
		"Snare01",
		"Snare02",
		"Tom01",
		
		"HiClose01",
		"HiOpen01",
		"Crash01",
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
		"RevSym",
		"Ride",
		"Tom03",
		"Tom04",
		
		"OrcDrm",
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
		"Clap",
		"Pesi",
		"Quick",
		"Bass08",
		
		"Snare08",
		"HiClose05",
	};
	
	//Instrument defaults
	uint8_t melody_default[8] = {0, 11, 22, 33, 44, 55, 66, 77};
	uint8_t drum_default[8] = {0, 2, 5, 6, 4, 8, 0, 0};
	
	//Playback tables
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
	
	static const int16_t freq_tbl[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
	static const int16_t pan_tbl[13] = {0, 43, 86, 129, 172, 215, 256, 297, 340, 383, 426, 469, 512};
	
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
				{
					event_state.length = 0;
					Stop();
				}
				else
				{
					event_state.length = event_check->length - (x - event_check->x);
					Play();
					Update();
				}
				break;
			}
			
			//Check previous event
			event_check = event_check->prev;
		}
	}
	
	void Instrument::UpdateState()
	{
		//Don't do anything if no event could be played
		if (event_point == nullptr)
			return;
		
		//If new event has played, update state
		if (x == event_point->x)
		{
			//Update and play key
			if (event_point->y != 0xFF)
			{
				event_state.y = event_point->y;
				event_state.length = event_point->length;
				Play();
			}
			
			//Update volume
			if (event_point->volume != 0xFF)
				event_state.volume = event_point->volume;
			
			//Update pan
			if (event_point->pan != 0xFF)
				event_state.pan = event_point->pan;
			
			//Update buffers
			Update();
		}
		
		//Stop once length reaches 0
		if (event_state.length == 0)
			Stop();
		else
			event_state.length--;
	}
	
	//Melody class
	bool Melody::ConstructBuffers(const Instance &organya)
	{
		for (size_t i = 0; i < 8; i++)
		{
			//Get sizes
			size_t wave_size = oct_wave[i].wave_size, data_size = wave_size * (pipi ? oct_wave[i].oct_size : 1);
			
			//Allocate buffer data
			int8_t *data = new int8_t[data_size];
			if (data == nullptr)
				return error.Push("Failed to allocate buffer data");
			int8_t *datap = data;
			
			//Get wave position
			const int8_t *wave;
			if ((wave = organya.GetWave(wave_no)) == nullptr)
				return error.Push("Failed to get waveform");
			
			//Fill buffer data
			size_t wave_tp = 0;
			for (size_t j = 0; j < data_size; j++)
			{
				*datap++ = wave[wave_tp];
				wave_tp = (wave_tp + (0x100 / wave_size)) & 0xFF;
			}
			
			//Create buffers
			for (size_t v = 0; v < 2; v++)
				if (buffer[i][v].SetData(data, data_size, 22050))
					return error.Push("Failed to set buffer data");
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
	
	void Melody::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		for (size_t i = 0; i < 8; i++)
			for (size_t v = 0; v < 2; v++)
				buffer[i][v].Mix(stream, stream_frequency, stream_frames);
	}
	
	void Melody::Play()
	{
		//Stop current buffer and switch to new one
		if (current_buffer != nullptr)
			current_buffer->Play(false);
		twin = !twin;
		current_buffer = &buffer[event_state.y / 12][twin];
		current_buffer->Play(!pipi);
	}
	
	void Melody::Update()
	{
		if (event_state.y != 0xFF)
		{
			//Update frequency for.. every buffer? WHAT? WHY?!
			for (size_t i = 0; i < 8; i++)
				for (size_t v = 0; v < 2; v++)
					buffer[i][v].SetFrequency(((oct_wave[i].wave_size * freq_tbl[event_state.y % 12]) * oct_wave[i].oct_par) / 8 + (freq - 1000));
		}
		
		//If not playing, don't update
		if (current_buffer == nullptr)
			return;
		
		//Update volume and panning for current buffer
		if (event_state.volume != 0xFF)
			current_buffer->SetVolume((event_state.volume - 0xFF) * 8);
		if (event_state.pan != 0xFF)
			current_buffer->SetPan((pan_tbl[event_state.pan] - 256) * 10);
	}
	
	void Melody::Stop()
	{
		//If not playing, don't stop
		if (current_buffer == nullptr)
			return;
		current_buffer->Play(false);
		current_buffer = nullptr;
	}
	
	//Drum class
	bool Drum::ConstructBuffers(const Instance &organya)
	{
		//Make sure content provider is available
		const ContentProvider *content_provider;
		if ((content_provider = organya.GetContentProvider()) == nullptr)
			return error.Push("No content provider was given");
		
		//Open drum file
		std::ifstream drum_stream = content_provider->OpenIn("Data/Resources/" + drum_name[wave_no], std::ifstream::ate | std::ifstream::binary);
		if (!drum_stream.is_open())
			return error.Push("Failed to open drum (" + drum_name[wave_no] + ") resource");
		
		//Get size of drum file
		drum_stream.seekg(0x36, std::ifstream::beg);
		size_t size = ((uint32_t)drum_stream.get() << 0) | ((uint32_t)drum_stream.get() << 8) | ((uint32_t)drum_stream.get() << 16) | ((uint32_t)drum_stream.get() << 24);
		
		//Read contents of drum file and use it for the sound buffer
		int8_t *data = new int8_t[size]{0};
		int8_t *datap = data;
		for (size_t i = 0; i < size && !drum_stream.eof(); i++)
			*datap++ = (int8_t)((uint8_t)drum_stream.get() - 0x80);
		if (buffer.SetData(data, size, 22050))
			return error.Push("Failed to set buffer data");
		delete[] data;
		return false;
	}
	
	void Drum::StopBuffers()
	{
		buffer.Stop();
	}
	
	void Drum::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		buffer.Mix(stream, stream_frequency, stream_frames);
	}
	
	void Drum::Play()
	{
		//Rewind and play buffer
		buffer.Play(false);
		buffer.SetFrequency(100 + event_state.y * 800);
		buffer.SetPosition(0.0);
	}
	
	void Drum::Update()
	{
		//Update volume and panning for buffer
		if (event_state.volume != 0xFF)
			buffer.SetVolume((event_state.volume - 0xFF) * 8);
		if (event_state.pan != 0xFF)
			buffer.SetPan((pan_tbl[event_state.pan] - 256) * 10);
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
	
	//Data initialization
	bool Instance::InitializeData(const ContentProvider *_content_provider)
	{
		//Use given content provider
		if ((content_provider = _content_provider) == nullptr)
			return error.Push("No content provider was given");
		
		//Open and read waveforms
		std::ifstream wave_stream = content_provider->OpenIn("Data/Resources/Wave100", std::ifstream::binary);
		if (!wave_stream.is_open())
			return error.Push("Failed to open Wave100 resource");
		
		int8_t *wavep = &wave[0][0];
		for (size_t i = 0; i < 0x100 * 100; i++)
			*wavep++ = (int8_t)wave_stream.get();
		return false;
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
		if ((i.wave_no = stream.get()) >= 100)
			return error.Push("Invalid wave number (" + std::to_string(i.wave_no) + ")");
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
			if ((event->y = stream.get()) >= 12 * 8 && event->y != 0xFF)
				return error.Push("Invalid event Y (" + std::to_string(event->y) + ")");
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
		//Fail if content provider wasn't given
		if (content_provider == nullptr)
			return error.Push("No content provider was given");
		
		//Unload previous data
		Unload();
		
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
			if (i.ConstructBuffers(*this))
				return error.Push(i.GetError());
		for (auto &i : drum)
			if (i.ConstructBuffers(*this))
				return error.Push(i.GetError());
		
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
	
	//Saving
	bool Instance::WriteInstrument(std::ostream &stream, Instrument &i)
	{
		//Write instrument data
		WriteLE16(stream, i.freq);
		if (i.wave_no >= 100)
			return error.Push("Invalid wave number (" + std::to_string(i.wave_no) + ")");
		stream.put(i.wave_no);
		stream.put(i.pipi);
		
		//Calculate note_num
		uint16_t note_num = 0;
		for (Event *event = i.event; event != nullptr; event = event->next)
			note_num++;
		WriteLE16(stream, note_num);
		return false;
	}
	
	bool Instance::WriteEvents(std::ostream &stream, Instrument &i)
	{
		//Read data into notes
		Event *event;
		for (event = i.event; event != nullptr; event = event->next)
			WriteLE32(stream, event->x);
		for (event = i.event; event != nullptr; event = event->next)
		{
			if (event->y >= 12 * 8 && event->y != 0xFF)
				return error.Push("Invalid event Y (" + std::to_string(event->y) + ")");
			stream.put(event->y);
		}
		for (event = i.event; event != nullptr; event = event->next)
			stream.put(event->length);
		for (event = i.event; event != nullptr; event = event->next)
			stream.put(event->volume);
		for (event = i.event; event != nullptr; event = event->next)
			stream.put(event->pan);
		return false;
	}
	
	bool Instance::Save(std::string _path)
	{
		//Open stream
		std::ofstream stream(_path, std::ofstream::binary);
		if (!stream.is_open())
			return error.Push("Failed to save to " + _path);
		
		//Save header
		for (auto &i : version_lut)
		{
			if (i.version == header.version)
			{
				stream << i.name;
				break;
			}
		}
		
		WriteLE16(stream, header.wait);
		stream.put(header.line);
		stream.put(header.dot);
		WriteLE32(stream, header.repeat_x);
		WriteLE32(stream, header.end_x);
		
		//Write instrument information
		for (auto &i : melody)
			if (WriteInstrument(stream, i))
				return true;
		for (auto &i : drum)
			if (WriteInstrument(stream, i))
				return true;
		
		//Write events
		for (auto &i : melody)
			if (WriteEvents(stream, i))
				return true;
		for (auto &i : drum)
			if (WriteEvents(stream, i))
				return true;
		return false;
	}
	
	bool Instance::Save()
	{
		return Save(path);
	}
	
	//New
	void Instance::ResetInstrument(Instrument &i)
	{
		i.freq = 1000;
		i.wave_no = 0;
		i.pipi = false;
	}
	
	bool Instance::New()
	{
		//Unload previous data
		Unload();
		
		//Reset header
		header.version = OrgV02;
		header.wait = 128;
		header.line = 4;
		header.dot = 4;
		header.repeat_x = 0;
		header.end_x = header.line * header.dot * 255;
		
		//Reset instrument info
		for (auto &i : melody)
			ResetInstrument(i);
		for (auto &i : drum)
			ResetInstrument(i);
		
		//Set instrument default waves
		uint8_t *defp = melody_default;
		for (auto &i : melody)
			i.wave_no = *defp++;
		defp = drum_default;
		for (auto &i : drum)
			i.wave_no = *defp++;
		
		//Construct instrument buffers
		for (auto &i : melody)
			if (i.ConstructBuffers(*this))
				return error.Push(i.GetError());
		for (auto &i : drum)
			if (i.ConstructBuffers(*this))
				return error.Push(i.GetError());
		return false;
	}
	
	//Other internal functions
	void Instance::UnloadInstrument(Instrument &i)
	{
		//Delete linked list
		while (i.event != nullptr)
		{
			i.event = i.event->next;
			if (i.event != nullptr)
				delete i.event->prev;
			else
				break;
		}
	}
	
	void Instance::Unload()
	{
		//Unload instruments
		for (auto &i : melody)
			UnloadInstrument(i);
		for (auto &i : drum)
			UnloadInstrument(i);
	}
	
	//Organya interface
	bool Instance::SetPosition(uint32_t _x)
	{
		//Lock audio
		if (audio.Lock())
			return error.Push(audio.GetError());
		
		//Set all instrument positions and get their states
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
	
	uint32_t Instance::GetPosition()
	{
		//Lock audio while getting position
		audio.Lock();
		uint32_t _x = x;
		audio.Unlock();
		return _x;
	}
	
	//Mixing interface
	void Instance::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		//Update and mix Organya
		size_t step_frames = header.wait * stream_frequency / 1000;
		size_t frames_done = 0;
		
		while (frames_done < stream_frames)
		{
			if (step_frames_counter >= step_frames)
			{
				//Reset step frames counter
				step_frames_counter = 0;
				
				//Increment position in song
				if (++x >= header.end_x)
					x = header.repeat_x;
				
				//Update instruments
				for (auto &i : melody)
					i.SetPosition(x);
				for (auto &i : drum)
					i.SetPosition(x);
				
				for (auto &i : melody)
					i.UpdateState();
				for (auto &i : drum)
					i.UpdateState();
			}
			
			//Get frames to do
			size_t frames_to_do = stream_frames - frames_done;
			if (frames_to_do > step_frames - step_frames_counter)
				frames_to_do = step_frames - step_frames_counter;
			
			//Mix instruments
			//for (auto &i : melody)
			//	i.Mix(stream, stream_frequency, frames_to_do);
			for (auto &i : drum)
				i.Mix(stream, stream_frequency, frames_to_do);
			stream += frames_to_do * 2;
			frames_done += frames_to_do;
			step_frames_counter += frames_to_do;
		}
	}
	
	int16_t *Instance::MixToBuffer(size_t &frames, unsigned int frequency, unsigned int repeats)
	{
		//Determine length
		size_t step_frames = header.wait * frequency / 1000;
		frames = (step_frames * header.end_x) + (step_frames * (header.end_x - header.repeat_x)) * repeats;
		
		//Allocate 32-bit buffer to mix into before clamping
		int32_t *buffer;
		if ((buffer = new int32_t[frames * 2]) == nullptr)
		{
			error.Push("Failed to allocate mix buffer");
			return nullptr;
		}
		
		//Prepare for playback
		if (Stop())
			return nullptr;
		uint32_t _x = x; //Remember previous X
		SetPosition(0);
		
		//Mix to mix buffer
		Mix(buffer, frequency, frames);
		
		//Reset state
		SetPosition(_x); //Go back to previous X
		
		//Allocate final buffer
		int16_t *fbuffer = new int16_t[frames * 2];
		int16_t *fbufferp = fbuffer;
		
		//Clamp and write buffer to final buffer
		int32_t *bufferp = buffer;
		for (size_t i = 0; i < frames * 2; i++)
		{
			if (*bufferp < -0x7FFF)
				*fbufferp++ = -0x7FFF;
			else if (*bufferp > 0x7FFF)
				*fbufferp++ = 0x7FFF;
			else
				*fbufferp++ = *bufferp;
			bufferp++;
		}
		
		//Delete mix buffer
		delete[] buffer;
		return fbuffer;
	}
	
	bool Instance::MixToStream(std::ostream &stream, unsigned int frequency, unsigned int repeats)
	{
		//Get mixed buffer
		size_t frames;
		int16_t *buffer;
		if ((buffer = MixToBuffer(frames, frequency, repeats)) == nullptr)
			return true;
		
		//Write mixed buffer to stream
		int16_t *bufferp = buffer;
		for (size_t i = 0; i < frames * 2; i++)
			WriteLE16(stream, *bufferp++);
		delete[] buffer;
	}
	
	//Audio
	void MiddleAudioCallback(const Audio::Config<Instance*> *config, int32_t *stream)
	{
		//Clear and mix stream
		memset(stream, 0, config->frames * 2 * sizeof(int32_t));
		config->userdata->Mix(stream, config->frequency, config->frames);
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
