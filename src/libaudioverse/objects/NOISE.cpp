/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

#include <math.h>
#include <stdlib.h>
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_functiontables.hpp>
#include <libaudioverse/private_dspmath.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_iir.hpp>
#include <limits>
#include <random>

class LavNoiseObject: public LavObject {
	public:
	LavNoiseObject(std::shared_ptr<LavSimulation> simulation);
	virtual void process();
	void white();
	void pink();
	void brown();
	std::mt19937_64 random_number_generator;
	std::uniform_real_distribution<float> uniform_distribution;
	LavIIRFilter pinkifier; //filter to turn white noise into pink noise.
};

//we give the random number generator a fixed seed for debugging purposes.
LavNoiseObject::LavNoiseObject(std::shared_ptr<LavSimulation> simulation): LavObject(Lav_OBJTYPE_NOISE, simulation, 0, 1),
random_number_generator(1234), uniform_distribution(-1.0f, 1.0f)  {
	/**We have to configure the pinkifier.
This was originally taken from Spectral Audio processing by JOS.*/
	//zeros
	double pinkNumer[] = {0.049922035, -0.095993537, 0.050612699, -0.004408786};
	//and the poles.
	double pinkDenom[] = {1, -2.494956002,   2.017265875,  -0.522189400};
	pinkifier.configure(sizeof(pinkNumer)/sizeof(double), pinkNumer, sizeof(pinkDenom)/sizeof(double), pinkDenom);
}

std::shared_ptr<LavObject> createNoiseObject(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavNoiseObject> retval = std::shared_ptr<LavNoiseObject>(new LavNoiseObject(simulation), LavObjectDeleter);
	simulation->associateObject(retval);
	return retval;
}

void LavNoiseObject::white() {
	for(int i = 0; i < block_size; i++) {
		float noiseSample = 0.0f;
		//use the central limit theorem to generate white noise.
		//if this proves too costly, revisit and examine the box-muller method.
		for(int j = 0; j < 20; j++) noiseSample += uniform_distribution(random_number_generator);
		//divide by 20 to get it back into range.
		noiseSample /= 20.0f;
		outputs[0][i] = noiseSample;
	}
}

void LavNoiseObject::pink() {
	white();
	for(int i = 0; i < block_size; i++) outputs[0][i] =pinkifier.tick(outputs[0][i]);
}

void LavNoiseObject::brown() {
}

void LavNoiseObject::process() {
	int type = getProperty(Lav_NOISE_NOISE_TYPE).getIntValue();
	switch(type) {
		case Lav_NOISE_TYPE_WHITE: white(); break;
		case Lav_NOISE_TYPE_PINK: pink(); break;
		case Lav_NOISE_TYPE_BROWN: brown(); break;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createNoiseObject(LavSimulation* simulation, LavObject **destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createNoiseObject(incomingPointer<LavSimulation>(simulation));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}