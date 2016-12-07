/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
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