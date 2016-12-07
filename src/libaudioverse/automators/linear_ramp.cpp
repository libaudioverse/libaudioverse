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

namespace libaudioverse_implementation {

class LinearRampAutomator: public Automator {
	public:
	LinearRampAutomator(Property* p, double scheduledTime, double finalValue);
	virtual void start(double initialTime, double initialValue) override;
	virtual double  getValue(double time) override;
	virtual double getFinalValue();
	double delta, final_value;
};

LinearRampAutomator::LinearRampAutomator(Property* p, double scheduledTime, double finalValue): Automator(p, scheduledTime), final_value(finalValue) {
}

void LinearRampAutomator::start(double initialValue, double initialTime) {
	Automator::start(initialValue, initialTime);
	delta = (final_value-initial_value)/(scheduled_time-initial_time);
}

double LinearRampAutomator::getValue(double time) {
	return initial_value+(time-initial_time)*delta;
}

double LinearRampAutomator::getFinalValue() {
	return final_value;
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_automationLinearRampToValue(LavHandle nodeHandle, int slot, double time, double value) {
	PUB_BEGIN
	auto node = incomingObject<Node>(nodeHandle);
	LOCK(*node);
	auto &prop= node->getProperty(slot);
	LinearRampAutomator* automator = new LinearRampAutomator(&prop, prop.getTime()+time, value);
	//the property will throw for us if any part of the next part goes wrong.
	prop.scheduleAutomator(automator);
	PUB_END
}

}