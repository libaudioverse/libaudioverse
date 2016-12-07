/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/node.hpp>
#include <algorithm>

namespace libaudioverse_implementation {

class SetAutomator: public Automator {
	public:
	SetAutomator(Property* p, double scheduledTime, double value);
	virtual double  getValue(double time) override;
	virtual double getFinalValue() override;
	double setting_to;
};

SetAutomator::SetAutomator(Property* p, double scheduledTime, double value): Automator(p, scheduledTime), setting_to(value) {
}

double SetAutomator::getValue(double time) {
	if(time < scheduled_time) return initial_value;
	else return setting_to;
}

double SetAutomator::getFinalValue() {
	return setting_to;
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_automationSet(LavHandle nodeHandle, int slot, double time, double value) {
	PUB_BEGIN
	if(time < 0.0) ERROR(Lav_ERROR_RANGE, "Time must be positive or 0.");
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	auto &prop= node->getProperty(slot);
	SetAutomator* automator = new SetAutomator(&prop, prop.getTime()+time, value);
	//the property will throw for us if any part of the next part goes wrong.
	prop.scheduleAutomator(automator);
	PUB_END
}

}