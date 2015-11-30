#pragma once
#include "../audio_io.hpp"
#include <speex_resampler_cpp.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace audio_io {
namespace implementation {

class SampleFormatConverter;

class OutputDeviceImplementation: public OutputDevice {
	protected:
	OutputDeviceImplementation() = default;
	virtual ~OutputDeviceImplementation();
	//Callback parameters: output buffer, number of channels to write.
	virtual void init(std::function<void(float*, int)> callback, int inputFrames, int inputChannels, int inputSr, int outputChannels, int outputSr);
	int input_frames, input_channels, input_sr, output_channels, output_sr;
	//This one is an estimate.  The amount of frames to write to trigger the callback approximately once.
	int output_frames;
	std::shared_ptr<SampleFormatConverter> sample_format_converter;
	std::function<void(float*, int)> callback;
	bool stopped = false; //Used by subclasses.
	friend class OutputDeviceFactoryImplementation;
};

class OutputDeviceFactoryImplementation: public OutputDeviceFactory {
	public:
	virtual ~OutputDeviceFactoryImplementation();
	//these two are overridden here, others come later.
	virtual unsigned int getOutputCount();
	virtual std::string getName();
	protected:
	std::vector<std::weak_ptr<OutputDeviceImplementation>> created_devices;
	int output_count = 0;
};

typedef OutputDeviceFactory* (*OutputDeviceFactoryCreationFunction)();
OutputDeviceFactory* createWinmmOutputDeviceFactory();
OutputDeviceFactory* createWasapiOutputDeviceFactory();
	
}
}