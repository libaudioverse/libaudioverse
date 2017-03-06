#include <chrono>
namespace audio_io {
namespace implementation {

//This is part of what controls how we raise the minimum latency on underruns.
//see LatencyPredictor::hadUnderrun.
const double latency_increment_for_underrun = 0.005;

/**Predict latency requirements.*/
class LatencyPredictor {
	public:
	LatencyPredictor(int historyLength, double minAllowedLatency, double startLatency, double maxAllowedLatency);
	LatencyPredictor(const LatencyPredictor& other) = delete;
	LatencyPredictor& operator=(const LatencyPredictor& other) = delete;
	~LatencyPredictor();
	//Call before processing a chunk of audio.
	void beginPass();
	//Call after processing a chunk of audio.
	void endPass();
	//call on an underrun, but not during a pass.
	void hadUnderrun();
	//Call only once between passes.
	//It is safe to call this before the first pass.
	double predictLatency();
	//Uses predictLatency.
	int predictLatencyInBlocks(int blockFrames, int sr);
	private:
	void doPrediction();
	double* history;
	int history_length, write_pointer = 0;
	double min_allowed_latency, max_allowed_latency;
	double last_predicted_latency;
	std::chrono::high_resolution_clock::time_point pass_start_time;
};

}
}