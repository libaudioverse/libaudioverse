/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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