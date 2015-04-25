/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/properties.hpp>

LavAutomator::LavAutomator(LavProperty* p, double scheduledTime): property(p), scheduled_time(scheduledTime) {
}

LavAutomator::~LavAutomator() {
}

void LavAutomator::start(double initialValue, double initialTime) {
	initial_value = initialValue;
	initial_time = initialTime;
}

double LavAutomator::getDuration() {
return 0.0;
}

double LavAutomator::getScheduledTime() {
	return scheduled_time;
}

bool compareAutomators(LavAutomator *a, LavAutomator *b) {
	return a->getScheduledTime() < b->getScheduledTime();
}
