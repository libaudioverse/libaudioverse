/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "memory.hpp"
#include <memory>

class LavSimulation;

class LavBuffer: public LavExternalObject {
	public:
	LavBuffer(std::shared_ptr<LavSimulation> simulation);
	~LavBuffer();
	int getLength();
	double getDuration();
	int getChannels();
	void setContents(int channels, int sr, int length, float* data);
	//Writes uninterleaved data resampled appropriately for the simulation being used.
	void writeData(int frames, float** buffers);
	private:
	int channels;
	int frames;
	int sr;
	float* data;
};
