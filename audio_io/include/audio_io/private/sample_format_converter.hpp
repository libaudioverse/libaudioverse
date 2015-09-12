#pragma once
#include <functional>
#include <memory>

//forward cdeclare SpeexResampler.
namespace speex_resampler_cpp {
class Resampler;
}

namespace audio_io {
namespace implementation {

/**A sample format converter converts audio from one sample rate and channel count to another sample rate and sample count.

This class will also hold conversion from sample type to sample type, should such be needed in future.*/
class SampleFormatConverter {
	public:
	SampleFormatConverter(std::function<void(float*, int)> callback, int inputFrames, int inputChannels, int inputSr, int outputChannels, int outputSr);
	~SampleFormatConverter();
	void write(int frames, float* buffer);
	private:
	//Refills the output buffer as needed.
	void refillOutputBuffer();
	std::function<void(float*, int)> callback;
	//Holds data directly from callback.
	float* input_buffer = nullptr;
	//Holds downmixed or upmixed data, if needed.
	float* output_buffer = nullptr;
	//Holds resampled data, possibly before downmixing.
	float* resampler_workspace = nullptr;
	int input_sr, output_sr, input_channels, output_channels, input_frames;
	int output_frames, consumed_output_frames ;
	int resampler_workspace_frames;
	std::shared_ptr<speex_resampler_cpp::Resampler> resampler;
};

}
}