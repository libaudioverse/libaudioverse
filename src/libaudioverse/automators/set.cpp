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