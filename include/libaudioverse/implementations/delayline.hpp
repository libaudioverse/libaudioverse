/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

namespace libaudioverse_implementation {

//used by all delay lines.
//This is a fixed-sized ringbuffer that can be advanced and written to as a single operation or read at a single offset.
class DelayRingbuffer {
	public:
	DelayRingbuffer(int length);
	DelayRingbuffer(const DelayRingbuffer& other) = delete;
	~DelayRingbuffer();
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
//Changing delays are handled by crossfading, as though we had two delay lines.
class CrossfadingDelayLine {
	public:
	CrossfadingDelayLine(float maxDelay, float sr);
	void setDelay(float delay);
	float computeSample();
	void advance(float sample);
	void write(float delay, float value);
	void add(float delay, float value);
	void reset();
	void setInterpolationTime(float t);
	private:
	DelayRingbuffer line;
	unsigned int line_length = 0, delay = 0, new_delay = 0;
	bool is_interpolating = false;
	float interpolation_delta = 1.0f;
	float sr = 0.0f, weight1=1.0f, weight2=0.0f;
};

//This is nearly the same as the crossfading delay line except:
//No feedback, and changes in delay cause changes in pitch while the new delay settles.
class DoppleringDelayLine {
	public:
	DoppleringDelayLine(float maxDelay, float sr);
	void setDelay(float d);
	void setInterpolationTime(float t);
	float tick(float sample);
	float computeSample();
	void advance(float sample);
	private:
	int max_delay = 0, interpolating_direction = 0;
	double delay = 0.0, new_delay = 0.0;
	int counter = 0; //counts down delay changes
	float velocity = 0.0; //what to change the delay by.
	float interpolation_time =0.1f;
	float sr = 0;
	DelayRingbuffer line;
};

}