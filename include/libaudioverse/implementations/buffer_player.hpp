/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/buffer.hpp"
#include "../private/memory.hpp"
#include <audio_io/audio_io.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace libaudioverse_implementation {

class Simulation;

class BufferPlayer {
	public:
	BufferPlayer(int _block_size, float _sr);
	~BufferPlayer();
	void process(int channels, float** outputs);
	void setBuffer(std::shared_ptr<Buffer> buff);
	std::shared_ptr<Buffer> getBuffer();
	void setPosition(double position);
	double getPosition();
	void setIsLooping(bool l);
	bool getIsLooping();
	void setRate(double rate);
	double getRate();
	//Increments every time the buffer ends.
	int getEndedCount();
	void resetEndedCount();
	private:
	std::shared_ptr<Buffer> buffer = nullptr;
	int frame = 0;
	int buffer_length=0;
	double offset=0.0, rate = 1.0;
	bool ended = true; //True if the buffer is ended or does not exist, allows for early quitting in tick.
	bool is_looping = false;
	float sr;
	int block_size;
	int ended_count = 0;
	//Channels of the current buffer.
	//This is used in a for loop in process, we don't want to call buffer->getChannels() hundreds of times a second.
	int buffer_channels = 0;
	//We get our output, and then downmix it ourselves.
	std::vector<float*> intermediate_destination;
};

inline BufferPlayer::BufferPlayer(int _block_size, float _sr): sr(_sr), block_size(_block_size) {}

inline BufferPlayer::~BufferPlayer() {
	for(auto &i: intermediate_destination) freeArray(i);
	if(buffer) buffer->decrementUseCount();
}

inline void BufferPlayer::process(int channels, float** outputs) {
	if(buffer == nullptr) return; //no buffer.
	if(buffer_length== 0) return;
	if(ended) return;
	//This is an optimizable case, and a fairly common one.  It degrades to a memcpy when we can do it.
	if(rate == 1.0 && offset < 1e-5 && frame+block_size < buffer_length) {
		for(int ch = 0; ch < buffer_channels; ch++) {
			float* p=buffer->getPointer(frame, ch);
			std::copy(p, p+block_size, intermediate_destination[ch]);
		}
		frame += block_size;
	}
	else {
		int computed_samples = 0;
		//We do the looping check first so we can break out if we have issues.
		for(int i =0; i < block_size; i++) {
			if(frame >= buffer_length) { //past end.
				ended_count ++;
				if(is_looping == false) {
					ended = true;
					break;
				}
				//Otherwise we loop.
				frame= 0;
			}
			for(int chan =0; chan < buffer_channels; chan++) {
				//This is standard linear interpolation.
				double a = buffer->getSample(frame, chan);
				double b;
				if(frame+1 < buffer_length) b = buffer->getSample(frame+1, chan); //okay, we have one more sample after this one.
				else if(is_looping) b =buffer->getSample(0, chan); //We have a next sample, but it's looped to the beginning.
				else b = 0.0; //no next sample.
				double weight1 = 1-offset;
				double weight2 = offset;
				intermediate_destination[chan][i] = (float)(weight1*a+weight2*b);
			}
			offset+=rate;
			frame += (int)floor(offset);
			offset = offset-floor(offset);
			computed_samples += 1;
		}
		//We have to zero the rest out.
		for(int ch = 0; ch < buffer_channels; ch++) {
			for(int i = computed_samples; i < block_size; i++) intermediate_destination[ch][i] = 0.0;
		}
	}
	//Remix to the destination.
	audio_io::remixAudioUninterleaved(block_size, buffer_channels, &intermediate_destination[0], channels, outputs);
}

inline void BufferPlayer::setBuffer(std::shared_ptr<Buffer> b) {
	if(buffer) buffer->decrementUseCount();
	buffer = b;
	if(b) b->incrementUseCount();
	frame = 0;
	offset = 0.0;
	resetEndedCount();
	if(b) ended = false;
	else ended = true;
	buffer_length = b ? b->getLength() : 0;
	//Ensure we have enough intermediate buffers.
	if(b) {
		while(intermediate_destination.size() < (unsigned int)b->getChannels()) intermediate_destination.push_back(allocArray<float>(block_size));
	}
	buffer_channels = b ? b->getChannels() : 0;
}

inline std::shared_ptr<Buffer> BufferPlayer::getBuffer() {
	return buffer;
}

inline void BufferPlayer::setPosition(double p) {
	frame = (int)(p*sr);
	//We want this to be sample-perfect whenever possible.
	offset = 0.0;
	ended = frame >= buffer_length;
}

inline double BufferPlayer::getPosition() {
	return frame/sr+offset/sr;
}

inline void BufferPlayer::setIsLooping(bool l) {
	is_looping = l;
	if(l) ended = false;
}

inline bool BufferPlayer::getIsLooping() {
	return is_looping;
}

inline void BufferPlayer::setRate(double r) {
	rate = r;
}

inline double BufferPlayer::getRate() {
	return rate;
}

inline int BufferPlayer::getEndedCount() {
	return ended_count;
}

inline void BufferPlayer::resetEndedCount() {
	ended_count = 0;
}

}