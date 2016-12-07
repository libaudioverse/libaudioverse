/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

/**Initialize libaudioverse.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/server.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/metadata.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/logging.hpp>
#include <libaudioverse/private/hrtf.hpp>
#include <libaudioverse/private/initialization.hpp>

#include <atomic>

namespace libaudioverse_implementation {

typedef void (*initfunc_t)();


struct InitInfo {
	const char* name;
	const initfunc_t func;
};

//These run in the order specified in this array with no parallelism.
//If adding new functionality here, simply add it and follow the above typedef.
//Errors will be returned as appropriate.
//Initialization stops at the first failed function and does not continue.
InitInfo initializers[] = {
	//Logging is implicit.
	{"Memory subsystem", initializeMemoryModule},
	{"Audio backend", initializeDeviceFactory},
	{"Metadata tables", initializeMetadata},
	{"HRTF caches", initializeHrtfCaches},
};

typedef void (*shutdownfunc_t)();

struct ShutdownInfo{
	const char* name;
	const shutdownfunc_t func;
};

//These run in the order specified in this array with no parallelism.
//If adding new functionality here, simply add it and follow the above typedef.
//Errors will be returned as appropriate.
//Termination never fails.
//logging must always be last.
ShutdownInfo shutdown_funcs[] = {
	{"memory module", shutdownMemoryModule},
	//Device factory needs to go near the end because it tries to log.
	{"audio backend", shutdownDeviceFactory},
	{"HRTF caches", shutdownHrtfCaches},
	{"logging", shutdownLogging},
};

std::atomic<int> initialization_count{0};

Lav_PUBLIC_FUNCTION LavError Lav_initialize() {
	PUB_BEGIN
	int ic = initialization_count.fetch_add(1)+1;
	if(ic > 1) {
		logDebug("Duplicate calls to initialization. This is call %i", ic+1);
		return Lav_ERROR_NONE;
	}
	logDebug("Beginning initialization of Libaudioverse, revision %s", getGitRevision());
	logDebug("Build type: %s", getBuildType());
	logDebug("C compiler flags: %s", getCompilerCFlags());
	logDebug("C++ flags: %s", getCompilerCxxFlags());
	logDebug("Linker flags: %s", getLinkerFlags());
	for(int i = 0; i < sizeof(initializers)/sizeof(initializers[0]); i++) {
		logDebug("Initializing %s.", initializers[i].name);
		initializers[i].func();
	}
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_shutdown() {
	using namespace libaudioverse_implementation;
	PUB_BEGIN
	INITCHECK;
	int ic = initialization_count.fetch_add(-1);
	ic--;
	if(ic) {
		logDebug("Not shutting down. %i more calls to shutdown required.");
		return Lav_ERROR_NONE;
	}
	logDebug("Beginning shutdown.");
	for(int i = 0; i < sizeof(shutdown_funcs)/sizeof(shutdown_funcs[0]); i++) {
		logDebug("Shutting down %s.", shutdown_funcs[i].name);
		shutdown_funcs[i].func();
	}
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_isInitialized(int* destination) {
	PUB_BEGIN
	*destination = (int)(initialization_count.load() > 0);
	PUB_END
}

}