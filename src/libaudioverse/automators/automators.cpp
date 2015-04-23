/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/properties.hpp>

LavAutomator::LavAutomator(LavProperty* p, double delayTime): property(p), delay_time(delayTime) {
		sr = p->getSr();
}

LavAutomator::~LavAutomator() {
}

double LavAutomator::getDelayTime() {
	return delay_time;
}

void LavAutomator::setDelayTime(double t) {
	delay_time = t;
}

int LavAutomator::writeValues(int length, double* destination) {
	int count = 0;
	while(getCompleted() == false && length > 0) {
		*destination = getNext();
		advance(1/sr);
		destination++;
		length--;
		count++;
	}
	return count;
}

bool LavAutomator::getNeedsARate() {
	return needs_arate;
}

void LavAutomator::setNeedsARate(bool v) {
	needs_arate = v;
}

bool LavAutomator::getCompleted() {
	return completed;
}

void LavAutomator::setCompleted(bool v) {
	completed = v;
}
