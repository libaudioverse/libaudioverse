/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

/**Initialize libaudioverse.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/metadata.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/logging.hpp>
#include <libaudioverse/private/hrtf.hpp>

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
	{"Error handling", initializeErrorModule},
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
	{"Error handling subsystem", shutdownErrorModule},
	{"memory module", shutdownMemoryModule},
	//Device factory needs to go near the end because it tries to log.
	{"audio backend", shutdownDeviceFactory},
	{"HRTF caches", shutdownHrtfCaches},
	{"logging", shutdownLogging},
};

unsigned int isInitialized = 0;

Lav_PUBLIC_FUNCTION LavError Lav_initialize() {
	PUB_BEGIN
	if(isInitialized == 1) {
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
	isInitialized = 1;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_shutdown() {
	using namespace libaudioverse_implementation;
	PUB_BEGIN
	logDebug("Beginning shutdown.");
	for(int i = 0; i < sizeof(shutdown_funcs)/sizeof(shutdown_funcs[0]); i++) {
		logDebug("Shutting down %s.", shutdown_funcs[i].name);
		shutdown_funcs[i].func();
	}
	isInitialized = 0;
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_isInitialized(int* destination) {
	PUB_BEGIN
	*destination = isInitialized;
	PUB_END
}

}