/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Initialize libaudioverse.*/
#include <libaudioverse/private_all.h>

typedef LavError (*initfunc_t)();

//These run in the order specified in this array with no parallelism.
//If adding new functionality here, simply add it and follow the above typedef.
//Errors will be returned as appropriate.
//Initialization stops at the first failed function and does not continue.
initfunc_t initializers[] = {
	initializeGlobalMemoryManager,
	initializeFunctionTables,
};

LavError Lav_initializeLibrary() {
	for(int i = 0; i < sizeof(initializers)/sizeof(initializers[0]); i++) {
		LavError err = initializers[i]();
		if(err != Lav_ERROR_NONE) {
			return err;
		}
	}
	return Lav_ERROR_NONE;
}
