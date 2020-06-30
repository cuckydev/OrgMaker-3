/*
Project: OrgMaker 3

File: src/Audio.cpp
Purpose: Define audio classes

Authors: Regan "cuckydev" Green
*/

//Declaration
#include "Audio.h"

//Audio namespace
namespace Audio
{
	//Audio buffer class
	//Destructor
	Buffer::~Buffer()
	{
		//Delete data
		delete[] data;
	}
	
	//Audio buffer interface
	bool Buffer::SetData(float *_data, size_t _size, unsigned int _frequency)
	{
		//Delete previous data
		if (data != nullptr)
			delete[] data;
		
		//Copy data
		size = _size;
		if ((data = new float[size + 1]) == nullptr)
			return true;
		for (size_t i = 0; i < size; i++)
			data[i] = _data[i];
		data[size] = 0.0f;
		
		//Initialize state
		position = 0.0f;
		play = false;
		loop = false;
		
		frequency = _frequency;
		volume = 1.0f;
		volume_l = 1.0f;
		volume_r = 1.0f;
		pan_l = 1.0f;
		pan_r = 1.0f;
		return false;
	}
	
	void Buffer::Mix(float *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		//Don't mix if not playing or hasn't been setup yet
		if (play == false || data == nullptr)
			return;
		
		//Get sample advance delta
		double advance_delta = (double)frequency / (double)stream_frequency;
		
		for (size_t i = 0; i < stream_frames; i++)
		{
			//Perform linear interpolation and mix
			double subsample = fmod(position, 1.0);
			size_t int_position = (size_t)position;
			float sample = (data[int_position] * (1.0 - subsample) + data[int_position + 1] * subsample);
			*stream++ += sample * volume_l;
			*stream++ += sample * volume_r;
			
			//Increment sample
			position += advance_delta;
			
			//Stop or loop sample once it's reached its end
			while (position >= size)
			{
				if (loop)
				{
					//Move back
					position -= size;
				}
				else
				{
					//Stop playing
					play = false;
					position = 0;
					return;
				}
			}
		}
	}
	
	void Buffer::Play()
	{
		play = true;
	}
	
	void Buffer::Stop()
	{
		play = false;
	}
	
	void Buffer::SetLoop(bool _loop)
	{
		loop = _loop;
		if (data != nullptr)
			data[size] = loop ? data[0] : 0.0f;
	}
	
	void Buffer::SetFrequency(unsigned int _frequency)
	{
		frequency = _frequency;
	}
	
	void Buffer::SetPosition(double _position)
	{
		position = _position;
	}
	
	void Buffer::SetVolume(int32_t _volume)
	{
		volume = ConvertVolume(_volume);
		volume_l = volume * pan_l;
		volume_r = volume * pan_r;
	}
	
	void Buffer::SetPan(int32_t pan)
	{
		pan_l = ConvertVolume(pan);
		pan_r = ConvertVolume(-pan);
		volume_l = volume * pan_l;
		volume_r = volume * pan_r;
	}
}
