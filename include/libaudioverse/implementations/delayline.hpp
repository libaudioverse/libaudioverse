/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once

namespace libaudioverse_implementation {

//used by all delay lines.
//This is a fixed-sized ringbuffer that can be advanced and written to as a single operation or read at a single offset.
//The length is made to be a power of two when constructed, enabling use of bit tricks for performance.
class DelayRingbuffer {
	public:
	DelayRingbuffer(unsigned int length);
	DelayRingbuffer(const DelayRingbuffer& other) = delete;
	~DelayRingbuffer();
	float read(unsigned int offset);
	unsigned int getLength();
	void advance(float sample);
	// Extract a bunch of samples efficiently.
	// in-place usage is safe.
	void process(unsigned int offset, int count, float* in, float* out);
	void write(unsigned int offset, float value);
	void add(unsigned int index, float value);
	void reset();
	private:
	float* buffer = nullptr;
	//mask is used for modulus with bitwise and.
	unsigned int buffer_length = 0, write_head = 0, mask = 0;
};

//A single-channel delay line, but with built-in crossfading.
//Changing delays are handled by crossfading, as though we had two delay lines.
class CrossfadingDelayLine {
	public:
	CrossfadingDelayLine(float maxDelay, float sr);
	void setDelay(float delay);
	void setDelayInSamples(int newDelay);
	//convenience function: combination compute and advance.
	float tick(float sample);
	//feedback has to use the slow path, but this one is optimized.
	//in-place is okay.
	void processBuffer(int length, float* input, float* output);
	float computeSample();
	void advance(float sample);
	void write(float delay, float value);
	void add(float delay, float value);
	void reset();
	void setInterpolationTime(float t);
	CrossfadingDelayLine* getSlave();
	void setSlave(CrossfadingDelayLine* s);
	private:
	DelayRingbuffer line;
	unsigned int max_delay = 0, delay = 0, new_delay = 0;
	int counter;
	float interpolation_delta = 1.0f, interpolation_time=0.0f;
	float sr = 0.0f, weight1=1.0f, weight2=0.0f;
	CrossfadingDelayLine* slave = nullptr;
};

//This is nearly the same as the crossfading delay line except:
//No feedback, and changes in delay cause changes in pitch while the new delay settles.
class DoppleringDelayLine {
	public:
	DoppleringDelayLine(float maxDelay, float sr);
	void setDelay(double d, bool shouldCrossfade = true);
	void setDelayInSamples(double newDelay, bool shouldCrossfade = true);
	void setInterpolationTime(float t);
	float tick(float sample);
	float computeSample();
	void advance(float sample);
	void process(int count, float* in, float* out);
	void reset();
	DoppleringDelayLine* getSlave();
	void setSlave(DoppleringDelayLine* s);
	private:
	int max_delay = 0, interpolating_direction = 0;
	double delay = 0.0;
	int counter = 0; //counts down delay changes
	// new_delay-counter*delta = delay.
	double delta = 0.0;
	float interpolation_time =0.1f;
	float sr = 0;
	DelayRingbuffer line;
	DoppleringDelayLine* slave = nullptr;
};

class InterpolatedDelayLine {
	public:
	InterpolatedDelayLine(float maxDelay, float sr);
	void setDelay(float d);
	void setDelayInSamples(int samples);
	float tick(float sample);
	float computeSample();
	void advance(float sample);
	void reset();
	InterpolatedDelayLine* getSlave();
	void setSlave(InterpolatedDelayLine* s);
	private:
	int max_delay = 0;
	double delay = 0.0;
	float sr = 0;
	DelayRingbuffer line;
	InterpolatedDelayLine* slave = nullptr;
};

}