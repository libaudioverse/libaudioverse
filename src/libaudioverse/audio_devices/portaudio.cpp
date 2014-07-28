/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_devices.hpp>
#include <libaudioverse/private_errors.hpp>
#include <portaudio.h>
#include <libaudioverse/private_resampler.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <atomic>
#include <set>
#include <chrono>
#include <utility>

int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

class LavPortaudioDevice: public LavDevice {
	public:
	LavPortaudioDevice(unsigned int sr, unsigned int blockSize, unsigned int mixahead);
	void doPortaudioDefaultDeviceNegotiation(unsigned int sr, unsigned int blockSize);
	void audioOutputThreadFunction(); //the function that runs as our output thread.
	std::thread audioOutputThread;
	std::atomic_flag runningFlag; //when this clears, the audio thread self-terminates.
	PaStream *stream = nullptr; //the portaudio stream we work with.  Started and stopped by the background thread for us.
	//a ringbuffer of dispatched buffers.
	//the elements are protected by the atomic ints.
	float** buffers = nullptr;
	std::atomic<int> *buffer_statuses = nullptr;
	int callback_buffer_index = 0; //the index the callback will go to on its next invocation.
	LavResampler *resampler = nullptr;
	unsigned int output_block_size = 0; //the block size after resampling.
	friend int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
};

void initializeAudioBackend() {
	PaError err = Pa_Initialize();
	if(err < 0) {
		throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	}
}

void LavPortaudioDevice::doPortaudioDefaultDeviceNegotiation(unsigned int sr, unsigned int blockSize) {
	/**We need to find the default devices for all APIs.*/
	std::vector<std::tuple<PaDeviceIndex, PaDeviceInfo*>> candidates;
	PaHostApiIndex maxApi = Pa_GetHostApiCount();
	for(PaHostApiIndex i = 0; i < maxApi; i++) {
		const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
		PaDeviceIndex ind = info->defaultOutputDevice;
		const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(ind);
		candidates.emplace_back(ind, const_cast<PaDeviceInfo*>(devinfo));
	}
	//sort by suggested latency.
	std::sort(candidates.begin(), candidates.end(), [](decltype(candidates)::value_type a, decltype(candidates)::value_type b) {
		return std::get<1>(a)->defaultLowOutputLatency < std::get<1>(b)->defaultLowOutputLatency;
	});
	if(candidates.size() == 0) {
		throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	}
	auto &needed = candidates[0];
	const PaDeviceInfo *neededInfo = std::get<1>(needed);
	const PaDeviceIndex neededIndex = std::get<0>(needed);
	PaStreamParameters outParams;
	outParams.device = neededIndex;
	outParams.channelCount = neededInfo->maxOutputChannels;
	outParams.sampleFormat = paFloat32;
	outParams.suggestedLatency = neededInfo->defaultLowOutputLatency;
	outParams.hostApiSpecificStreamInfo = nullptr;
	double neededSr = neededInfo->defaultSampleRate;
	resampler = new LavResampler(blockSize, outParams.channelCount, sr, (int)neededSr);
	unsigned int neededBlockSize = resampler->getOutputFrameCount();
	printf("Needed block size:%i\n", neededBlockSize);
	printf("Needed samplerate:%f", neededSr);
	printf("API and device indices: %i, %i\n", std::get<1>(needed)->hostApi, outParams.device);
	printf("Device name: %s\n", std::get<1>(needed)->name);
	printf("API name: %s\n", Pa_GetHostApiInfo(std::get<1>(needed)->hostApi)->name);
	PaError err = Pa_OpenStream(&stream, nullptr, &outParams, (double)neededSr, neededBlockSize, 0, portaudioOutputCallback, this);
	if(err < 0) throw LavErrorException(Lav_ERROR_CANNOT_INIT_AUDIO);
	channels = outParams.channelCount;
	output_block_size = neededBlockSize;
}

LavPortaudioDevice::LavPortaudioDevice(unsigned int sr, unsigned int blockSize, unsigned int mixahead): LavDevice(0, 0, 0, 0) { //todo: this is a hack to get around a deficiency.
	doPortaudioDefaultDeviceNegotiation(sr, blockSize);
	buffers = new float*[mixahead+1];
	for(unsigned int i = 0; i < mixahead+1; i++) buffers[i] = new float[output_block_size*channels];
	buffer_statuses = new std::atomic<int>[mixahead+1];
	for(unsigned int i = 0; i < mixahead+1; i++) buffer_statuses[i].store(0); //make sure they're all 0.  If not, bad things are going to happen.
	this->block_size = blockSize;
	this->sr = (float)sr;
	this->mixahead = mixahead;
	//set the background thread on its way.
	runningFlag.test_and_set();
	audioOutputThread = std::thread([this] () {audioOutputThreadFunction();});
}

std::shared_ptr<LavDevice> createDefaultPortaudioDevice(unsigned int sr, unsigned int blockSize, unsigned int mixahead) {
	return std::make_shared<LavPortaudioDevice>(sr, blockSize, mixahead);
}

/**This algorithm is complex and consequently requires some explanation.
- At the beginning, i.e. when this thread starts, all of the buffers point to valid memory locations big enough for one block.  In addition, all atomic ints are set to 0, meaning unprocessed.
- This thread processes buffers as fast as it possibly can.  If it sees an atomic int of 0, it assumes unprocessed and goes to sleep for a bit.
- The callback will set the output buffer to 0 if it sees an atomic int of 0 at its current reading position.
- If the callback sees any other value, it will copy the memory out and flip that value to 0.
Basically, this is a one-reader one-writer lock-free ringbuffer with an additional set of flags to tell who last touched something, and we don't consider the two variables buffers and buffer_statuses to be protected by our mutex.
*/
void LavPortaudioDevice::audioOutputThreadFunction() {
	int rb_index = 0; //our index into the buffers array.
	float* tempBuffer  = new float[block_size*channels];
	PaError err = Pa_StartStream(stream);
	printf("Initializing stream: %i\n", (int)err);
	printf("Last error text: %s", Pa_GetLastHostErrorInfo()->errorText);
	while(runningFlag.test_and_set()) {
		if(buffer_statuses[rb_index].load()) { //we just caught up and the queue is full.
			std::this_thread::sleep_for(std::chrono::milliseconds((block_size*1000)/(int)sr));
			continue;
		}
		//process into this buffer.
		lock();
		getBlock(tempBuffer);
		unlock();
		//tick our resampler.
		resampler->tick(tempBuffer, buffers[rb_index]);
		//mark it as safe for the audio callback.
		buffer_statuses[rb_index].store(1);
		//and compute the next index.
		rb_index += 1;
		rb_index %= (mixahead+1);
	}
	Pa_StopStream(stream);
}

int portaudioOutputCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	LavPortaudioDevice * const dev = (LavPortaudioDevice*)userData;
	const int haveBuffer = dev->buffer_statuses[dev->callback_buffer_index].load();
	if(haveBuffer) {
		memcpy(output, dev->buffers[dev->callback_buffer_index], sizeof(float)*frameCount*dev->channels);
		dev->buffer_statuses[dev->callback_buffer_index].store(0);
		dev->callback_buffer_index++;
		dev->callback_buffer_index %= dev->mixahead+1;
	}
	else {
		memset(output, 0, dev->channels*frameCount*sizeof(float));
	}
	return paContinue;
}
