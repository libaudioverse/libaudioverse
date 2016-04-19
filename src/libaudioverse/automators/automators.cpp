/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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