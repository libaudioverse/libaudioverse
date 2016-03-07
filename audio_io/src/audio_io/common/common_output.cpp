#include <audio_io/audio_io.hpp>
#include <audio_io/private/audio_outputs.hpp>
#include <audio_io/private/sample_format_converter.hpp>
#include <audio_io/private/logging.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace audio_io {
namespace implementation {

/**Code common to all backends, i.e. enumeration.*/

//these are the two steps in initialization, and are consequently put before the destructor.
void OutputDeviceImplementation::init(std::function<void(float*, int)> callback, int inputFrames, int inputChannels, int inputSr, int outputChannels, int outputSr) {
	input_frames = inputFrames;
	input_channels = inputChannels;
	input_sr = inputSr;
	output_channels = outputChannels;
	output_sr = outputSr;
	this->callback = callback;
	sample_format_converter = std::make_shared<SampleFormatConverter>(callback, inputFrames, inputChannels, inputSr, outputChannels, outputSr);
	//Estimate: outputSr/inputSr is in output samples/input samples, the conversion factor by dimensional analysis.
	output_frames = input_frames*output_sr/input_sr;
	logDebug("DeviceFactoryImplementation initialized. "
	"input_frames = %i, input_sr = %i, input_channels = %i, output_frames = %i, output_sr = %i, output_channels = %i.",
	input_frames, input_sr, input_channels, output_frames, output_sr, output_channels);
}

OutputDeviceImplementation::~OutputDeviceImplementation() {
}

OutputDeviceFactoryImplementation::~OutputDeviceFactoryImplementation() {
	logInfo("Output device factory is dying.  Terminating all devices.");
	for(auto p: created_devices) {
		auto strong = p.lock();
		if(strong) strong->stop();
	}
}

unsigned int OutputDeviceFactoryImplementation::getOutputCount() {
	return (unsigned int)output_count;
}

std::string OutputDeviceFactoryImplementation::getName() {
	return "Invalid backend: subclass failed to implement";
}

} //end namespace implementation
} //end namespace audio_io