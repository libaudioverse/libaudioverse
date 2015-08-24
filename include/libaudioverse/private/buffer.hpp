/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "memory.hpp"
#include <memory>

namespace libaudioverse_implementation {
class Simulation;

class Buffer: public ExternalObject {
	public:
	Buffer(std::shared_ptr<Simulation> simulation);
	~Buffer();
	std::shared_ptr<Simulation> getSimulation();
	int getLength();
	double getDuration();
	int getChannels();
	//This can be used outside the lock; the only thing it does is read simulation's sr value which can never change by definition.
	void loadFromArray(int sr, int channels, int frames, float* inputData);
	//Writes uninterleaved data resampled appropriately for the simulation being used.
	//This remixes according to the built-in Libaudioverse remixing rules as necessary.
	//Returns count of written frames.
	int writeData(int startFrame, int channels, int frames, float** outputs);
	//Writes an output channel.  It is assumed the array is already zeroed.
	//maxChannels is the maximum channels of the node using this function. Buffers know how to upmix and downmix using the Libaudioverse rules.
	int writeChannel(int startFrame, int channel, int maxChannels, int frames, float* dest);
	//The following two functions do not check if the requested frame is past the end for efficiency.
	//It is possible the compiler would optimize this, but running  in debug mode is already really painful and the trade-off here is worth it.
	//a single sample without mixing:
	float getSample(int frame, int channel);
	//And with mixing:
	float getSampleWithMixingMatrix(int frame, int channel, int maxChannels);

	//meet lockable concept:
	void lock();
	void unlock();

	//Normalize the buffer: divide by the sample furthest from zero.
	//This can't be undone.
	void normalize();
	private:
	void ensureRemixingWorkspace(int length);
	int channels = 0;
	int frames = 0;
	int sr = 0;
	float* data = nullptr, *data_end = nullptr;
	std::shared_ptr<Simulation> simulation;
	float* remixing_workspace = nullptr;
	int remixing_workspace_length = 0;
};

std::shared_ptr<Buffer> createBuffer(std::shared_ptr<Simulation>simulation);

}