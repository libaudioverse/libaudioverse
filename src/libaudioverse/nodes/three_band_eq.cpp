/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/iir.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <limits>
#include <algorithm>


namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class ThreeBandEqNode: public Node {
	public:
	ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);
	~ThreeBandEqNode();
	void recompute();
	virtual void process() override;
	virtual void reset() override;
	float lowband_gain;
	IIRFilter** midband_peaks= nullptr;
	IIRFilter** highband_shelves = nullptr;
	int channels;
};

ThreeBandEqNode::ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_THREE_BAND_EQ_NODE, simulation, channels, channels) {
	if(channels <= 0) throw LavErrorException(Lav_ERROR_RANGE);
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	this->channels=channels;
	midband_peaks = new IIRFilter*[channels];
	highband_shelves= new IIRFilter*[channels];
	for(int i = 0; i < channels; i++) {
		midband_peaks[i] = new IIRFilter(simulation->getSr());
		highband_shelves[i] = new IIRFilter(simulation->getSr());
	}
	//Set ranges of the nyqiuist properties.
	getProperty(Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY).setFloatRange(0.0, simulation->getSr()/2.0);
	getProperty(Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY).setFloatRange(0.0, simulation->getSr()/2.0);
	recompute();
}

std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels) {
	std::shared_ptr<ThreeBandEqNode> retval = std::shared_ptr<ThreeBandEqNode>(new ThreeBandEqNode(simulation, channels), ObjectDeleter(simulation));
	simulation->associateNode(retval);
	return retval;
}

ThreeBandEqNode::~ThreeBandEqNode() {
	for(int i = 0; i < channels; i++) {
		delete midband_peaks[i];
		delete highband_shelves[i];
	}
	delete[] midband_peaks;
	delete[] highband_shelves;
}

void ThreeBandEqNode::recompute() {
	double lowbandFreq=getProperty(Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY).getFloatValue();
	double lowbandDb=getProperty(Lav_THREE_BAND_EQ_LOWBAND_DBGAIN).getFloatValue();
	double midbandDb=getProperty(Lav_THREE_BAND_EQ_MIDBAND_DBGAIN).getFloatValue();
	double highbandFreq = getProperty(Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY).getFloatValue();
	double highbandDb= getProperty(Lav_THREE_BAND_EQ_HIGHBAND_DBGAIN).getFloatValue();
	double midbandFreq = lowbandFreq+(highbandFreq-lowbandFreq)/2.0;
	//low band's gain is the simplest.
	lowband_gain=dbToScalar(lowbandDb, 1.0);
	//The peaking filter for the middle band needs to go from lowbandDb to midbandDb, i.e.:
	double peakingDbgain =midbandDb-lowbandDb;
	//And the highband needs to go from the middle band to the high.
	double highshelfDbgain=highbandDb-midbandDb;
	//Compute q from bw and s, using an arbetrary IIR filter.
	//The iir filters only care about sr, so we can just pick one.
	double peakingQ=midband_peaks[0]->qFromBw(midbandFreq, (highbandFreq-midbandFreq)*2);
	double highshelfQ = highband_shelves[0]->qFromS(highbandFreq, 1.0);
	for(int i = 0; i < channels; i++) {
		midband_peaks[i]->configureBiquad(Lav_BIQUAD_TYPE_PEAKING, midbandFreq, peakingDbgain, peakingQ);
		highband_shelves[i]->configureBiquad(Lav_BIQUAD_TYPE_HIGHSHELF, highbandFreq, highshelfDbgain, highshelfQ);
	}
}

void ThreeBandEqNode::process() {
	if(werePropertiesModified(this,
	Lav_THREE_BAND_EQ_LOWBAND_DBGAIN,
	Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY,
	Lav_THREE_BAND_EQ_MIDBAND_DBGAIN,
	Lav_THREE_BAND_EQ_HIGHBAND_DBGAIN,
	Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY
	)) recompute();
	for(int channel=0; channel < channels; channel++) {
		auto &peak= *midband_peaks[channel];
		auto &shelf = *highband_shelves[channel];
		for(int i= 0; i < block_size; i++) {
			output_buffers[channel][i] = lowband_gain*peak.tick(shelf.tick(input_buffers[channel][i]));
		}
	}
}

void ThreeBandEqNode::reset() {
	for(int i = 0; i < channels; i++) {
		midband_peaks[i]->clearHistories();
		highband_shelves[i]->clearHistories();
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createThreeBandEqNode(LavHandle simulationHandle, int channels, LavHandle* destination) {
	PUB_BEGIN
	auto simulation = incomingObject<Simulation>(simulationHandle);
	LOCK(*simulation);
	auto retval = createThreeBandEqNode(simulation, channels);
	*destination = outgoingObject<Node>(retval);
	PUB_END
}

}