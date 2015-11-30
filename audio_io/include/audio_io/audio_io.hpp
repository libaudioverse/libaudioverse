#pragma once
#include <logger_singleton/logger_singleton.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <exception>

namespace audio_io {

/**Note: all strings are encoded as UTF8.
This library will decode as necessary.*/

/**Call initialize first; call  shutdown last.
These are primarily for setting up logging.  The main interface of the library is through getOutputDevice factory and similar.

Be sure that you delete all objects that you got from this library before calling shutdown.  Failure to do so will cause breakage as some destructors try to log.*/

void initialize();
void shutdown();

/**Retrieve the logger_singleton logger.

You can use this before initialization.*/
std::shared_ptr<logger_singleton::Logger> getLogger();

/**Exceptions.*/

/**This exception is thrown when we don't have any more information to give.

This is regrettably a common case.  Be prepared to catch this exception and show what() to the user.
audio_io attempts to make what() human friendly when it can.
The more specific subclasses below catch specific errors, but platforms vary wildly as to what counts as an error condition, and sometimes things just go wrong.*/
class AudioIOError: std::exception {
	public:
	AudioIOError(std::string m): message(m) {}
	const char* what() const override { return message.c_str();}
	private:
	std::string message;
};

/**Generally indicates that a device was unplugged during initialization or that the system is running in i.e. Wasapi exclusive mode.*/
class DeviceUnavailableError: public AudioIOError {
	public:
	DeviceUnavailableError(std::string m): AudioIOError(m) {}
};

/**Thrown by getFactory if factory initialization fails.*/
class NoBackendError: public AudioIOError {
	public:
	NoBackendError(): AudioIOError("No audio_io backend is available on the current system.") {}
};

/**A physical output.*/
class OutputDevice {
	public:
	virtual ~OutputDevice() {}
	//These are usually accessed via shared pointers. This function makes sure that the output device is stopped, blocking until it stops.
	//TODO: we need to be able to restart them.
	virtual void stop() = 0;
};

class OutputDeviceFactory {
	public:
	OutputDeviceFactory() = default;
	virtual ~OutputDeviceFactory() {}
	virtual std::vector<std::string> getOutputNames() = 0;
	virtual std::vector<int> getOutputMaxChannels() = 0;
	/**Get a device.
	minLatency is the minimum allowed latency. startLatency is the latency at which the device starts.
	maxLatency is the maximum allowed latency.
	audio_io does not guarantee that these will be respected.  They are hints.*/
	virtual std::shared_ptr<OutputDevice> createDevice(std::function<void(float*, int)> getBuffer, int index, unsigned int channels, unsigned int sr, unsigned int blockSize, float minLatency, float startLatency, float maxLatency) = 0;
	virtual unsigned int getOutputCount() = 0;
	virtual std::string getName() = 0;
};

std::shared_ptr<OutputDeviceFactory> getOutputDeviceFactory();

/**Remix a buffer.  These can be used without initialization.
- The standard channel counts 1, 2, 6 (5.1) and 8 (7.1) are mixed between each other appropriately.
- Otherwise, if input is mono and output is not mono, the input is copied to all channels.
- Otherwise, we keep min(inputChannels, outputChannels) channels of audio data, and fill any remaining output channels with zero.

These functions come in two variants.
- The uninterleaved version expects audio data as contiguous frames.
- The interleaved version expects audio data as an array of buffers.

In-place usage is not safe.

zeroFirst is intended for applications that need to accumulate data; if true (the default) buffers are zeroed before use.
*/

void remixAudioInterleaved(int frames, int inputChannels, float* input, int outputChannels, float* output, bool zeroFirst = true);
void remixAudioUninterleaved(int frames, int inputChannels, float** inputs, int outputChannels, float** outputs, bool zeroFirst = true);

}