/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
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