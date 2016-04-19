/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#pragma once
#include "../private/file.hpp"
#include <speex_resampler_cpp.hpp>
#include <memory>
#include <algorithm>
#include <string>
#include <inttypes.h>

namespace libaudioverse_implementation {

/**This is a lot like a BufferPlayer, but channels matches the file always and we resample at run-time.*/

class FileStreamer {
	public:
	FileStreamer(std::string path, int _block_size, float _sr);
	~FileStreamer();
	void process(float** outputs);
	void setPosition(double position);
	double getPosition();
	double getDuration();
	void setIsLooping(bool l);
	bool getIsLooping();
	//Increments every time the buffer ends.
	//Only true if and only if we aren't looping, otherwise false.
	//This last fact is important for the node's process method.
	bool getEnded();
	int getChannels();
	private:
	void feedResampler();
	int block_size = 0;
	float sr = 0.0f;
	FileReader reader;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler = nullptr;
	float *workspace_before_resampling = nullptr, *workspace_after_resampling = nullptr;
	int channels = 0;
	double position = 0.0, duration  = 0.0, position_per_sample = 0.0;
	int64_t position_in_frames = 0;
	bool is_looping = false, ended_before_resampling = false, ended_after_resampling = false;
};

}