/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
#include <libaudioverse/private/audio_thread.hpp>
#include <libaudioverse/private/logging.hpp>

namespace libaudioverse_implementation {
#if defined(LIBAUDIOVERSE_IS_WINDOWS)
#include <windows.h>
#include <avrt.h>

//AvSetMmThreadCharacteristics returns 0 on failure.
thread_local HANDLE task = 0;
void becomeAudioThread() {
	if(task) return; //We are already one.  This can sometimes happen.
	DWORD unused = 0;
	//yes this string is magic. See the MMCSS docs on MSDN.
	task = AvSetMmThreadCharacteristics("Pro Audio", &unused);
	if(task == 0) logDebug("Failed to make a thread a pro audio thread using MMCSS.");
}

void unbecomeAudioThread() {
	if(task) {
		AvRevertMmThreadCharacteristics(task);
		if(task == 0) logDebug("Failed to revert an MMCSS thread.");
		task = 0; //So that we aren't one anymore.
	}
}

#else
void becomeAudioThread() {
}

void unbecomeAudioThread() {
}
#endif
}