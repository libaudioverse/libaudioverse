/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include "memory.hpp"
#include "simulation.hpp"
#include <memory>


class LavBuffer: public LavExternalObject {
	public:
	LavBuffer(std::shared_ptr<LavSimulation> simulation);
	~LavBuffer();
	int getLength();
	double getDuration();
	int getChannels();
	//This can be used outside the lock; the only thing it does is read simulation's sr value which can never change by definition.
	void setContents(int sr, int channels, int frames, float* inputData);
	//Writes uninterleaved data resampled appropriately for the simulation being used.
	//This remixes according to the built-in Libaudioverse remixing rules as necessary.
	//Returns count of written frames.
	int writeData(int startFrame, int channels, int frames, float** outputs);

	//meet lockable concept:
	void lock() {simulation->lock();}
	void unlock() {simulation->unlock();}

	private:
	int channels = 0;
	int frames = 0;
	int sr = 0;
	float* data = nullptr, *data_end = nullptr;
	std::shared_ptr<LavSimulation> simulation;
};
