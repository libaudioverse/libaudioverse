/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/connections.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/audio_devices.hpp>
#include <libaudioverse/private/data.hpp>
#include <audio_io/audio_io.hpp>
#include <stdlib.h>
#include <functional>
#include <algorithm>
#include <iterator>
#include <thread>
#include <tuple>
#include <map>

namespace libaudioverse_implementation {

Simulation::Simulation(unsigned int sr, unsigned int blockSize, unsigned int mixahead): ExternalObject(Lav_OBJTYPE_SIMULATION) {
	if(blockSize%4 || blockSize== 0) throw LavErrorException(Lav_ERROR_RANGE); //only afe to have this be a multiple of four.
	this->sr = (float)sr;
	this->block_size = blockSize;
	this->mixahead = mixahead;
	registerDefaultMixingMatrices();
	//fire up the background thread.
	backgroundTaskThread = std::thread([this]() {backgroundTaskThreadFunction();});
	start();
}
void Simulation::completeInitialization() {
	final_output_connection =std::make_shared<InputConnection>(std::static_pointer_cast<Simulation>(this->shared_from_this()), nullptr, 0, 0);
}

Simulation::~Simulation() {
	//enqueue a task which will stop the background thread.
	enqueueTask([]() {throw ThreadTerminationException();});
	backgroundTaskThread.join();
}

//Yes, this uses goto. Yes, goto is evil. We need a single point of exit.
void Simulation::getBlock(float* out, unsigned int channels, bool mayApplyMixingMatrix) {
	if(out == nullptr || channels == 0) goto end; //nothing to do.
	if(block_callback) block_callback(outgoingObject(this->shared_from_this()), block_callback_time, block_callback_userdata);
	//configure our connection to the number of channels requested.
	final_output_connection->reconfigure(0, channels);
	//append buffers to the final_outputs vector until it's big enough.
	//in a sane application, we'll never go above 8 channels so keeping them around is no big deal.
	while(final_outputs.size() < channels) final_outputs.push_back(allocArray<float>(block_size));
	//zero the outputs we need.
	for(unsigned int i= 0; i < channels; i++) memset(final_outputs[i], 0, sizeof(float)*block_size);
	//write, applying mixing matrices as needed.
	final_output_connection->addNodeless(&final_outputs[0], true);
	//interleave the samples.
	interleaveSamples(channels, block_size, channels, &final_outputs[0], out);
	//handle the always playing nodes.
	//recall that nodes will only allow themselves to tick once in each block.
	for(auto &i: nodes) {
		auto i_s = i.lock();
		if(i_s== nullptr) continue;
		if(i_s->getState()!=Lav_NODESTATE_ALWAYS_PLAYING) continue;
		i_s->tick();
	}
	block_callback_time +=block_size/sr;
	end:
	int maintenance_count=maintenance_start;
	for(auto &i: nodes) {
		auto i_s=i.lock();
		if(i_s == nullptr) continue;
		if(maintenance_count % maintenance_rate== 0) i_s->doMaintenance();
		maintenance_count++;
	}
	maintenance_start++;
	//and ourselves.
	if(maintenance_start%maintenance_rate == 0) doMaintenance();
	tick_count ++;
}

void Simulation::doMaintenance() {
	decltype(nodes) to_remove;
	for(auto &n: nodes) {
		if(n.lock() == nullptr) to_remove.insert(n);
	}
	for(auto &n: to_remove) {
		nodes.erase(n);
	}
}

std::shared_ptr<InputConnection> Simulation::getFinalOutputConnection() {
	return final_output_connection;
}

LavError Simulation::start() {
	is_started = 1;
	return Lav_ERROR_NONE;
}

LavError Simulation::stop() {
	is_started = 0;
	return Lav_ERROR_NONE;
}

LavError Simulation::associateNode(std::shared_ptr<Node> node) {
	nodes.insert(std::weak_ptr<Node>(node));
	return Lav_ERROR_NONE;
}

void Simulation::enqueueTask(std::function<void(void)> cb) {
	tasks.enqueue(cb);
}

void Simulation::associateDevice(std::shared_ptr<audio_io::OutputDevice> what) {
	device = what;
}

void Simulation::registerMixingMatrix(unsigned int inChannels, unsigned int outChannels, float* matrix) {
	mixing_matrices[std::tuple<unsigned int, unsigned int>(inChannels, outChannels)] = matrix;
	if(inChannels > largest_seen_mixing_matrix_input) {
		if(mixing_matrix_workspace) delete[] mixing_matrix_workspace;
		mixing_matrix_workspace = new float[block_size*inChannels]();
		largest_seen_mixing_matrix_input = inChannels;
	}
}

void Simulation::resetMixingMatrix(unsigned int  inChannels, unsigned int outChannels) {
	for(MixingMatrixInfo* i = mixing_matrix_list; i->pointer; i++) {
		if(i->in_channels == inChannels && i->out_channels == outChannels) {
			mixing_matrices[std::tuple<unsigned int, unsigned int>(inChannels, outChannels)] = i->pointer;
			return;
		}
	}
	if(mixing_matrices.count(std::tuple<unsigned int, unsigned int>(inChannels, outChannels)) != 0) mixing_matrices.erase(std::tuple<unsigned int, unsigned int>(inChannels, outChannels));
}

void Simulation::registerDefaultMixingMatrices() {
	for(MixingMatrixInfo* i = mixing_matrix_list; i->pointer; i++) registerMixingMatrix(i->in_channels, i->out_channels, i->pointer);
}

const float* Simulation::getMixingMatrix(unsigned int inChannels, unsigned int outChannels) {
	std::tuple<unsigned int, unsigned int> key(inChannels, outChannels);
	if(mixing_matrices.count(key) != 0) return mixing_matrices[key];
	else return nullptr;
}

//Default callback implementation.
void Simulation::backgroundTaskThreadFunction() {
	try {
		for(;;) {
			auto task = tasks.dequeue();
			task();
		}
	}
	catch(ThreadTerminationException) {
		return;
	}
}

void Simulation::setBlockCallback(LavBlockCallback callback, void* userdata) {
	block_callback = callback;
	block_callback_time = 0.0;
	block_callback_userdata=userdata;
}

//begin public API

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlock(LavHandle simulationHandle, unsigned int channels, int mayApplyMixingMatrix, float* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	simulation->getBlock(destination, channels, mayApplyMixingMatrix != 0);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetBlockSize(LavHandle simulationHandle, int* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = simulation->getBlockSize();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationGetSr(LavHandle simulationHandle, int* destination) {
	PUB_BEGIN
	auto simulation =incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	*destination = (int)simulation->getSr();
	PUB_END
}

//atomic blocks

Lav_PUBLIC_FUNCTION LavError Lav_simulationBeginAtomicBlock(LavHandle simulationHandle) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	simulation->lock();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationEndAtomicBlock(LavHandle simulationHandle) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	simulation->unlock();
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_simulationSetBlockCallback(LavHandle handle, LavBlockCallback callback, void* userdata) {
	PUB_BEGIN
incomingObject<Simulation>(handle)->setBlockCallback(callback, userdata);
	PUB_END
}

}