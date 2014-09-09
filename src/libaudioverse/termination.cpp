/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Shut down Libaudioverse.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_functiontables.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_metadata.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_audio_devices.hpp>
#include <libaudioverse/private_logging.hpp>

typedef void (*shutdownfunc_t)();

//These run in the order specified in this array with no parallelism.
//If adding new functionality here, simply add it and follow the above typedef.
//Errors will be returned as appropriate.
//Termination never fails.
//logging must always be last.
shutdownfunc_t shutdown_funcs[] = {
shutdownLogging,
};

Lav_PUBLIC_FUNCTION LavError Lav_shutdown() {
	PUB_BEGIN
	for(int i = 0; i < sizeof(shutdown_funcs)/sizeof(shutdown_funcs[0]); i++) {
		shutdown_funcs[i]();
	}
	PUB_END
}
