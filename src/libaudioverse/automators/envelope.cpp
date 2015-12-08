/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/node.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

class EnvelopeAutomator: public Automator {
	public:
	EnvelopeAutomator(Property* p, double scheduledTime, double duration, int points, double* values);
	virtual double  getValue(double time) override;
	virtual double getFinalValue();
	double* envelope = nullptr;
	int points=0;
	//Number of intervals is points-1, this is the duration of each interval.
	double interval_duration = 0;
};

EnvelopeAutomator::EnvelopeAutomator(Property* p, double scheduledTime, double duration, int points, double* values): Automator(p, scheduledTime) {
	envelope = new double[points]();
	this->points = points;
	this->duration =duration;
	this->interval_duration = duration/(points-1);
	std::copy(values, values+points, envelope);
}

double EnvelopeAutomator::getValue(double time) {
	double delta = time-scheduled_time;
	if(delta < 0) return initial_value;
	if(delta > duration) return envelope[points-1];
	//linear interpolate.
	//This is the index of the first interval between points, not the first point.
	//The fact that it happens to be the first point of said interval is coincidental.
	int p1 = (int)((delta/duration)*(points-1));
	p1 =std::min(p1, points-1);
	int p2 = std::min(p1+1, points-1);
	if(p1==p2) return envelope[p1]; //because this only happens at the last point.
	double t1 = interval_duration*p1;
	double t2 = interval_duration*p2;
	double w1 =(t2-delta)/interval_duration;
	double w2= (delta-t1)/interval_duration;
	return envelope[p1]*w1+envelope[p2]*w2;
}

double EnvelopeAutomator::getFinalValue() {
	return envelope[points-1];
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_automationEnvelope(LavHandle nodeHandle, int slot, double time, double duration, int valuesLength, double *values) {
	PUB_BEGIN
	//preconditions first.
	if(valuesLength ==0) ERROR(Lav_ERROR_RANGE, "ENvelope must have values.");
	if(values == nullptr) ERROR(Lav_ERROR_RANGE, "Values cannot be null.");
	if(duration <= 0.0) ERROR(Lav_ERROR_RANGE, "Duration must be positive.");
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	auto &prop= node->getProperty(slot);
	EnvelopeAutomator* automator = new EnvelopeAutomator(&prop, prop.getTime()+time, duration, valuesLength, values);
	//the property will throw for us if any part of the next part goes wrong.
	prop.scheduleAutomator(automator);
	PUB_END
}

}