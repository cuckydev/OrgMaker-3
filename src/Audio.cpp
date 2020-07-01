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
	bool Buffer::SetData(int8_t *_data, size_t _size, unsigned int _frequency)
	{
		//Delete previous data
		if (data != nullptr)
			delete[] data;
		
		//Copy data
		size = _size;
		if ((data = new int8_t[size + 1]) == nullptr)
			return true;
		for (size_t i = 0; i < size; i++)
			data[i] = _data[i];
		data[size] = 0;
		
		//Initialize state
		position.value = 0;
		play = false;
		loop = false;
		
		frequency = _frequency;
		volume.value = 0x100;
		volume_l.value = 0x100;
		volume_r.value = 0x100;
		pan_l.value = 0x100;
		pan_r.value = 0x100;
		return false;
	}
	
	ATTRIBUTE_HOT void Buffer::Mix(int32_t *stream, unsigned int stream_frequency, size_t stream_frames)
	{
		//Don't mix if not playing or hasn't been setup yet
		if (play == false || data == nullptr)
			return;
		
		//Get sample advance delta
		uint32_t advance_delta = ((uint32_t)frequency << 16) / (stream_frequency);
		
		for (size_t i = 0; i < stream_frames; i++)
		{
			//Perform linear interpolation and mix
			int16_t sample = (data[position.fixed.upper] * (0x10000 - position.fixed.lower) + data[position.fixed.upper + 1] * position.fixed.lower) >> 8;
			*stream++ += (sample * volume_l.value) >> 8;
			*stream++ += (sample * volume_r.value) >> 8;
			
			//Increment sample
			position.value += advance_delta;
			
			//Stop or loop sample once it's reached its end
			while (position.fixed.upper >= size)
			{
				if (loop)
				{
					//Move back
					position.fixed.upper -= size;
				}
				else
				{
					//Stop playing
					play = false;
					position.value = 0;
					return;
				}
			}
		}
	}
	
	void Buffer::Play(bool _loop)
	{
		play = true;
		loop = _loop;
		if (data != nullptr)
			data[size] = loop ? data[0] : 0.0f;
	}
	
	void Buffer::Stop()
	{
		play = false;
	}
	
	void Buffer::SetFrequency(unsigned int _frequency)
	{
		frequency = _frequency;
	}
	
	void Buffer::SetPosition(uint16_t _position)
	{
		position.fixed.upper = _position;
		position.fixed.lower = 0;
	}
	
	void Buffer::SetVolume(int32_t _volume)
	{
		volume.value = ConvertVolume(_volume) * 0x100;
		volume_l.value = (volume.value * pan_l.value) >> 8;
		volume_r.value = (volume.value * pan_r.value) >> 8;
	}
	
	void Buffer::SetPan(int32_t pan)
	{
		pan_l.value = ConvertVolume(-pan) * 0x100;
		pan_r.value = ConvertVolume(pan) * 0x100;
		volume_l.value = (volume.value * pan_l.value) >> 8;
		volume_r.value = (volume.value * pan_r.value) >> 8;
	}
}
