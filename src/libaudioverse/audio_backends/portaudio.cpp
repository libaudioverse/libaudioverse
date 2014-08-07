/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <string>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <string.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <portaudio.h>

class LavPortaudioPhysicalOutput: public  LavPhysicalOutput {
	public:
	virtual void startup_hook();
	virtual void shutdown_hook();
	LavPortaudioPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead, PaDeviceIndex which);
};

LavPortaudioPhysicalOutput::LavPortaudioPhysicalOutput(std::shared_ptr<LavDevice> dev, unsigned int mixAhead, PaDeviceIndex which):  LavPhysicalOutput(dev, mixAhead) {
}
void LavPortaudioPhysicalOutput::startup_hook() {
}

void LavPortaudioPhysicalOutput::shutdown_hook() {
}
