#pragma once

/*
Project: OrgMaker 3

File: src/Audio.h
Purpose: Declare audio classes

Authors: Regan "cuckydev" Green
*/

//Standard library
#include <stdint.h>
#include <stddef.h>

//Library
#include "SDL_audio.h"

//OrgMaker3 classes
#include "Error.h"

//Audio namespace
namespace Audio
{
	//Audio config
	template <typename T>
	struct Config
	{
		T userdata;
		
		int frequency;
		uint16_t frames;
		
		void (*callback)(const Config *config, uint8_t *stream);
	};
	
	//Middle audio callback
	template <typename T>
	void MiddleAudioCallback(void *userdata, uint8_t *stream, int len)
	{
		Config<T> *config = (Config<T>*)userdata;
		config->callback(config, stream);
	};
	
	//Audio instance class
	template <typename T>
	class Instance
	{
		private:
			//Error
			Error error;
			
			//Current config
			Config<T> config;
			
			//Audio device and state
			SDL_AudioDeviceID audio_device = 0;
			bool lock = false, pause = true;
			
		public:
			//Constructors and destructor
			Instance() {}
			Instance(Config<T> _config) { SetConfig(_config); }
			~Instance()
			{
				//Close audio device
				if (audio_device != 0)
				{
					SDL_PauseAudioDevice(audio_device, 1);
					if (lock)
						SDL_UnlockAudioDevice(audio_device);
					SDL_CloseAudioDevice(audio_device);
				}
			}
			
			//Audio interface
			bool SetConfig(const Config<T> &_config)
			{
				//Close previous audio device
				if (audio_device != 0)
				{
					SDL_PauseAudioDevice(audio_device, 1);
					if (lock)
						SDL_UnlockAudioDevice(audio_device);
					SDL_CloseAudioDevice(audio_device);
				}
				
				//Use given config
				config = _config;
				
				//Construct audio spec
				SDL_AudioSpec spec = {
					config.frequency,
					AUDIO_F32SYS,
					2,
					0,
					(uint16_t)config.frames,
					0, //undocumented `Uint16 padding`
					0,
					MiddleAudioCallback<T>,
					(void*)&config,
				};
				
				//Open audio device
				SDL_AudioSpec obtained;
				if ((audio_device = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE)) == 0)
					return error.Push(SDL_GetError());
				
				//Update config from obtained spec
				config.frequency = obtained.freq;
				
				//Reset audio state
				lock = false;
				pause = true;
				return false;
			}
			
			bool Pause()
			{
				if (!pause)
					SDL_PauseAudioDevice(audio_device, 1);
				pause = true;
				return false;
			}
			
			bool Resume()
			{
				if (pause)
					SDL_PauseAudioDevice(audio_device, 0);
				pause = false;
				return false;
			}
			
			bool Lock()
			{
				if (!lock)
					SDL_LockAudioDevice(audio_device);
				lock = true;
				return false;
			}
			
			bool Unlock()
			{
				if (lock)
					SDL_UnlockAudioDevice(audio_device);
				lock = false;
				return false;
			}
			
			//Get error
			const Error &GetError() const { return error; }
	};
	
	//Audio buffer class
	class Buffer
	{
		private:
			//Audio data
			float *data = nullptr;
			size_t size;
			double position;
			
			//Audio state
			bool play = false, loop = false;
			int frequency;
			float volume, volume_l, volume_r, pan_l, pan_r;
			
		public:
			//Constructors and destructor
			Buffer() { }
			Buffer(float *_data, size_t _size, int _frequency) { SetData(_data, _size, _frequency); };
			~Buffer();
			
			//Buffer interface
			void SetData(float *_data, size_t _size, int _frequency);
			
			void Mix(float *stream, int stream_frequency, size_t stream_frames);
			
			void Play();
			void Stop();
			
			void SetLoop(bool _loop);
			void SetPosition(float _position);
			void SetFrequency(int _frequency);
			void SetVolume(int32_t _volume);
			void SetPan(int32_t pan);
			
		private:
			//Volume convertion
			float ConvertVolume(int32_t volume)
			{
				if (volume < -10000)
					volume = -10000;
				else if (volume > 0)
					volume = 0;
				return powf(10.0f, volume / 2000.0f);
			}
	};
}
