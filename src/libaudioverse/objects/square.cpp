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
#include <limits>

/**Note.  We can't use floats. There's some instability with the accumulator model that was here before that shows up as audible artifacts.*/
class LavSquareObject: public LavObject {
	public:
	LavSquareObject(std::shared_ptr<LavSimulation> simulation);
	void recomputeCounters();
	virtual void process();
	int wave_length, on_for, counter = 0;
};

LavSquareObject::LavSquareObject(std::shared_ptr<LavSimulation> simulation): LavObject(Lav_OBJTYPE_SQUARE, simulation, 0, 1) {
	getProperty(Lav_SQUARE_FREQUENCY).setPostChangedCallback([=] (){recomputeCounters();});
	getProperty(Lav_SQUARE_DUTY_CYCLE).setPostChangedCallback([=] (){recomputeCounters();});
	recomputeCounters();
}

std::shared_ptr<LavObject> createSquareObject(std::shared_ptr<LavSimulation> simulation) {
	std::shared_ptr<LavSquareObject> retval = std::shared_ptr<LavSquareObject>(new LavSquareObject(simulation), LavObjectDeleter);
	simulation->associateObject(retval);
	return retval;
}

void LavSquareObject::recomputeCounters() {
	float freq= getProperty(Lav_SQUARE_FREQUENCY).getFloatValue();
	float dutyCycle = getProperty(Lav_SQUARE_DUTY_CYCLE).getFloatValue();
	wave_length = (int)(simulation->getSr()/freq);
	on_for=(int)(wave_length*dutyCycle);
}

void LavSquareObject::process() {
	for(int i= 0; i < block_size; i++) {
		outputs[0][i] = counter < on_for ? 1.0f : 0.0f;
		counter++;
		counter%= wave_length;
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createSquareObject(LavSimulation* simulation, LavObject **destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createSquareObject(incomingPointer<LavSimulation>(simulation));
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}