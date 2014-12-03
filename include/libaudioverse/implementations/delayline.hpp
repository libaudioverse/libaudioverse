/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

//used by all delay lines.
//This is a fixed-sized ringbuffer that can be advanced and written to as a single operation or read at a single offset.
class LavDelayRingbuffer {
	public:
	LavDelayRingbuffer(int length);
	~LavDelayRingbuffer();
	float read(int offset);
	int getLength();
	void advance(float sample);
	void write(int offset, float value);
	void add(int index, float value);
	void reset();
	private:
	float* buffer = nullptr;
	int buffer_length = 0, write_head = 0;
};

//A single-channel delay line, but with built-in crossfading.
//note: this is for the delay object. It does not always do the intuitive thing from a DSP perspective.
//The point is that it does the intuitive thing for external-facing components and users, and it is robust against even frequent delay changes.
class LavCrossfadingDelayLine {
	public:
	LavCrossfadingDelayLine(float maxDelay, float sr);
	void setDelay(float delay);
	float computeSample();
	void advance(float sample);
	void write(float delay, float value);
	void add(float delay, float value);
	void reset();
	void setInterpolationTime(float t);
	private:
	LavDelayRingbuffer line;
	unsigned int line_length = 0, delay = 0, new_delay = 0;
	bool is_interpolating = false;
	float interpolation_delta = 1.0f;
	float sr = 0.0f, weight1=1.0f, weight2=0.0f;
};
