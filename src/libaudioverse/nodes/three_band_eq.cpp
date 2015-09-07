/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/implementations/biquad.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/kernels.hpp>
#include <libaudioverse/private/dspmath.hpp>
#include <libaudioverse/private/multichannel_filter_bank.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class ThreeBandEqNode: public Node {
	public:
	ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels);
	void recompute();
	virtual void process() override;
	virtual void reset() override;
	float lowband_gain;
	MultichannelFilterBank<BiquadFilter> midband_peaks, highband_shelves;
};

ThreeBandEqNode::ThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels): Node(Lav_OBJTYPE_THREE_BAND_EQ_NODE, simulation, channels, channels),
midband_peaks(simulation->getSr()),
highband_shelves(simulation->getSr()) {
	if(channels <= 0) ERROR(Lav_ERROR_RANGE, "Channels must be greater 0.");
	appendInputConnection(0, channels);
	appendOutputConnection(0, channels);
	midband_peaks.setChannelCount(channels);
	highband_shelves.setChannelCount(channels);
	//Set ranges of the nyqiuist properties.
	getProperty(Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY).setFloatRange(0.0, simulation->getSr()/2.0);
	getProperty(Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY).setFloatRange(0.0, simulation->getSr()/2.0);
	recompute();
}

std::shared_ptr<Node> createThreeBandEqNode(std::shared_ptr<Simulation> simulation, int channels) {
	return standardNodeCreation<ThreeBandEqNode>(simulation, channels);
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
	//Compute q from bw and s, using an arbetrary biquad filter.
	//The biquad filters only care about sr, so we can just pick one.
	double peakingQ = midband_peaks->qFromBw(midbandFreq, (highbandFreq-midbandFreq)*2);
	double highshelfQ = highband_shelves->qFromS(highbandFreq, 1.0);
	midband_peaks->configure(Lav_BIQUAD_TYPE_PEAKING, midbandFreq, peakingDbgain, peakingQ);
	highband_shelves->configure(Lav_BIQUAD_TYPE_HIGHSHELF, highbandFreq, highshelfDbgain, highshelfQ);
}

void ThreeBandEqNode::process() {
	if(werePropertiesModified(this,
	Lav_THREE_BAND_EQ_LOWBAND_DBGAIN,
	Lav_THREE_BAND_EQ_LOWBAND_FREQUENCY,
	Lav_THREE_BAND_EQ_MIDBAND_DBGAIN,
	Lav_THREE_BAND_EQ_HIGHBAND_DBGAIN,
	Lav_THREE_BAND_EQ_HIGHBAND_FREQUENCY
	)) recompute();
	for(int channel=0; channel < midband_peaks.getChannelCount(); channel++) scalarMultiplicationKernel(block_size, lowband_gain, input_buffers[channel], output_buffers[channel]);
	midband_peaks.process(block_size, &output_buffers[0], &output_buffers[0]);
	highband_shelves.process(block_size, &output_buffers[0], &output_buffers[0]);
}

void ThreeBandEqNode::reset() {
	midband_peaks.reset();
	highband_shelves.reset();
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