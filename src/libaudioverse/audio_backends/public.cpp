/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_physical_outputs.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_errors.hpp>
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

/**Public facing code.  This includes the rest of the library itself and the public API.*/

//list from greatest to least priority.
LavPhysicalOutputFactoryCreationFunction possible_backends[] = {
	createPortaudioPhysicalOutputFactory,
};
LavPhysicalOutputFactory* chosen_factory = nullptr;


void initializePhysicalOutputFactory() {
	for(unsigned int i = 0; i < sizeof(possible_backends)/sizeof(LavPhysicalOutputFactoryCreationFunction); i++) {
		LavPhysicalOutputFactory* possible = possible_backends[i]();
		if(possible != nullptr) {
			chosen_factory = possible;
			return;
		}
	}
	//none of them returned a factory, so we need to crash and burn.
	throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
}

