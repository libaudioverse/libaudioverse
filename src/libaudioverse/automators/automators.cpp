/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/macros.hpp>

namespace libaudioverse_implementation {

Automator::Automator(Property* p, double scheduledTime): property(p), scheduled_time(scheduledTime) {
}

Automator::~Automator() {
}

void Automator::start(double initialValue, double initialTime) {
	initial_value = initialValue;
	initial_time = initialTime;
}

double Automator::getDuration() {
	return duration;
}

double Automator::getScheduledTime() {
	return scheduled_time;
}

bool compareAutomators(Automator *a, Automator *b) {
	return a->getScheduledTime() < b->getScheduledTime();
}

Lav_PUBLIC_FUNCTION LavError Lav_automationCancelAutomators(LavHandle nodeHandle, int slot, double time) {
	PUB_BEGIN
	if(time < 0.0) ERROR(Lav_ERROR_RANGE, "Time must be positive or zero.");
	auto n = incomingObject<Node>(nodeHandle);
	LOCK(*n);
	auto &prop = n->getProperty(slot);
	prop.cancelAutomators(time);
	PUB_END
}

}