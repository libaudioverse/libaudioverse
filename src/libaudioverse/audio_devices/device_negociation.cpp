/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_macros.hpp>
#include <stdlib.h>
#include <string.h>

/**This file handles device negociation: figuring out which device is the best for the current platform.

At the moment, this is always portaudio's default, and the setup is all done via the private API.
Read device creation is also here, because putting all the public device creation code in one place seemed like a good idea.*/

Lav_PUBLIC_FUNCTION LavError Lav_createDefaultAudioOutputDevice(LavDevice** destination) {
	//create a device if possible, giving it our desired settings, and then pass it to the portaudio thread builder, which redirects callbacks on the device as appropriate.
	LavDevice* retval;
	//we prefer 44100 sr, 2 channel, and block size of 1024 with mixahead 2.
	//this gives a granularity of 23 MS between hearing property changes and about 69 MS between the app setting a property and the properety's new value being sent to the sound card.
	retval = outgoingPointer<LavDevice>(createPortaudioDevice(44100, 2, 1024, 2));
	*destination = retval;
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_createReadDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, LavDevice** destination) {
	PUB_BEGIN
	auto shared = std::make_shared<LavDevice>(sr, channels, blockSize, 0);
	*destination = outgoingPointer(shared);
	PUB_END
}
