/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "libaudioverse.h"
#include "private_sourcemanager.hpp"
#include <vector>
#include <set>
#include <memory>
#include <utility>

/**A physical output.*/
class LavPhysicalOutput {
	public:
	private:
	std::function<void(LavPhysicalOutput*, float*)> get_audio_callback;
};

class LavPhysicalOutputFactory {
	public:
	LavPhysicalOutputFactory() = delete;
	virtual ~LavPhysicalOutputFactory() {}
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<float> getOutputLatencies() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	private:
};

bool portaudioBackendAvailable();
LavPhysicalOutputFactory* createPortaudioPhysicalOutputFactory();
