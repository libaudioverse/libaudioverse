/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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