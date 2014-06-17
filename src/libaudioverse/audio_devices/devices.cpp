/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_devices.hpp>
#include <stdlib.h>

LavDevice::LavDevice(unsigned int sr, unsigned int channels, unsigned int blockSize, unsigned int mixahead) {
	this->sr = (float)sr;
	this->channels = channels;
	this->block_size = blockSize;
	this->mixahead = mixahead;
}

