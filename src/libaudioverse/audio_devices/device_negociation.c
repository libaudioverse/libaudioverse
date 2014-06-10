/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

/**This file handles device negociation: figuring out which device is the best for the current platform.

At the moment, this is always portaudio's default, and the setup is all done via the private API.*/

Lav_PUBLIC_FUNCTION LavError Lav_createDefaultAudioOutputDevice(LavDevice** destination) {
	STANDARD_PREAMBLE;
	//create a device if possible, giving it our desired settings, and then pass it to the portaudio thread builder, which redirects callbacks on the device as appropriate.
	CHECK_NOT_NULL(destination);
	LavDevice* retval;
	//we prefer 44100 sr, 2 channel, and block size of 1024 with mixahead 2.
	//this gives a granularity of 23 MS between hearing property changes and about 69 MS between the app setting a property and the properety's new value being sent to the sound card.
	LavError err = createGenericDevice(1024, 2, 44100, NULL, NULL, NULL, NULL, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	retval->mixahead = 2;
	err = portaudioDeviceConfigurer(retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	*destination = retval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}
