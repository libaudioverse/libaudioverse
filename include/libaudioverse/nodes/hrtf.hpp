/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "../private/node.hpp"
#include "../implementations/delayline.hpp"
#include <kiss_fftr.h>
#include <memory>

namespace libaudioverse_implementation {
class Simulation;
class HrtfData;
class FftConvolver;

class HrtfNode: public Node {
	public:
	HrtfNode(std::shared_ptr<Simulation> simulation, std::shared_ptr<HrtfData> hrtf);
	~HrtfNode();
	virtual void process();
	void reset();
	//the difference between the time the sound would reach the left ear and the time it would reach the right.
	//returns positive values if the right ear is greater, negative if the left ear is greater.
	float computeInterauralDelay();
	void applyIdtChanged();
	private:
	//the hrtf.
	std::shared_ptr<HrtfData> hrtf = nullptr;
	//for determining when we should and shouldn't crossfade.
	float prev_azimuth = 0.0f, prev_elevation = 0.0f;
	//Force a recompute of the HRIR.  Used when resetting.
	bool force_recompute = true;
	//buffers and length for the convolvers.
	float* left_response, *right_response;
	int response_length;
	//the convolvers themselves.
	FftConvolver *left_convolver, *right_convolver, *new_left_convolver, *new_right_convolver;
	//A delta used in crossfading.
	float crossfade_delta=0.0f;
	float* crossfade_workspace;
	//variables for the interaural time difference.
	CrossfadingDelayLine left_delay_line, right_delay_line;
	const float max_interaural_delay = 0.02f;
	//Used to guarantee that we only compute the fft once.
	kiss_fftr_cfg fft = nullptr;
	float* fft_workspace=nullptr;
	kiss_fft_cpx *input_fft = nullptr;
};

std::shared_ptr<Node>createHrtfNode(std::shared_ptr<Simulation>simulation, std::shared_ptr<HrtfData> hrtf);
}