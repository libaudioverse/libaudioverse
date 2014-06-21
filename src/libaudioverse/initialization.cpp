/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Initialize libaudioverse.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_functiontables.hpp>
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_macros.hpp>

typedef void (*initfunc_t)();

//These run in the order specified in this array with no parallelism.
//If adding new functionality here, simply add it and follow the above typedef.
//Errors will be returned as appropriate.
//Initialization stops at the first failed function and does not continue.
initfunc_t initializers[] = {
	initializeFunctionTables,
	initializeAudioBackend,
};
unsigned int isInitialized = 0;

Lav_PUBLIC_FUNCTION LavError Lav_initializeLibrary() {
	PUB_BEGIN
	if(isInitialized == 1) {
		return Lav_ERROR_NONE;
	}
	for(int i = 0; i < sizeof(initializers)/sizeof(initializers[0]); i++) {
		initializers[i]();
	}
	isInitialized = 1;
	PUB_END
}
