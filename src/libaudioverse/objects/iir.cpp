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
#include <libaudioverse/private_iir.hpp>

class LavIirObject: public LavObject {
	public:
	LavIirObject(std::shared_ptr<LavSimulation> simulation, int channels);
	virtual void process();
	void setCoefficients(int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory);
	std::vector<LavIIRFilter> filters;
};

LavIirObject::LavIirObject(std::shared_ptr<LavSimulation> simulation, int channels): LavObject(Lav_OBJTYPE_IIR, simulation, channels, channels) {
	if(channels <= 0) throw LavErrorException(Lav_ERROR_RANGE);
	filters.resize(channels);
	double defaultNumerator[] = {1.0};
	double defaultDenominator[] = {1.0, 0.0}; //identity filter.
	setCoefficients(1, defaultNumerator, 2, defaultDenominator, 1);
}

std::shared_ptr<LavObject> createIirObject(std::shared_ptr<LavSimulation> simulation, int channels) {
	std::shared_ptr<LavIirObject> retval = std::shared_ptr<LavIirObject>(new LavIirObject(simulation, channels), LavObjectDeleter);
	simulation->associateObject(retval);
	return retval;
}

void LavIirObject::process() {
	for(int i = 0; i < filters.size(); i++) {
		auto &f =filters[i];
		for(int j = 0; j < block_size; j++) outputs[i][j] = f.tick(inputs[i][j]);
	}
}

void LavIirObject::setCoefficients(int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	for(auto &i: filters) {
		i.configure(numeratorLength, numerator, denominatorLength, denominator);
		if(shouldClearHistory !=0) i.clearHistories();
	}
}

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_createIirObject(LavSimulation* simulation, int channels, LavObject **destination) {
	PUB_BEGIN
	LOCK(*simulation);
	auto retval = createIirObject(incomingPointer<LavSimulation>(simulation), channels);
	*destination = outgoingPointer<LavObject>(retval);
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_iirObjectSetCoefficients(LavObject* obj, int numeratorLength, double* numerator, int denominatorLength, double* denominator, int shouldClearHistory) {
	PUB_BEGIN
	LOCK(*obj);
	if(obj->getType() != Lav_OBJTYPE_IIR) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavIirObject*)obj)->setCoefficients(numeratorLength, numerator, denominatorLength, denominator, shouldClearHistory);
	PUB_END
}