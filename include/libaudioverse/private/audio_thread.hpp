/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once
namespace libaudioverse_implementation {
//Called on a thread that process audio. Attempts to turn us itno an audio thread.
//This functionn may raise our priority or otherwise register us.
void becomeAudioThread();
void unbecomeAudioThread();

}