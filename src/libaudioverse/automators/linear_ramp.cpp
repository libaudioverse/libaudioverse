/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/macros.hpp>
#include <libaudioverse/private/memory.hpp>
#include <libaudioverse/private/node.hpp>

class LavLinearRampAutomator: public LavAutomator {
	public:
	LavLinearRampAutomator(LavProperty* p, double endTime, double finalValue);
	virtual void start(double initialTime, double initialValue) override;
	virtual double  getValueAtTime(double time) override;
	virtual double getFinalValue();
	double delta, final_value;
};

LavLinearRampAutomator::LavLinearRampAutomator(LavProperty* p, double endTime, double finalValue): LavAutomator(p, endTime), final_value(finalValue) {
}

void LavLinearRampAutomator::start(double initialValue, double initialTime) {
	LavAutomator::start(initialValue, initialTime);
	delta = (final_value-initial_value)/(end_time-initial_time);
}

double LavLinearRampAutomator::getValueAtTime(double time) {
	return initial_value+(time-initial_time)*delta;
}

double LavLinearRampAutomator::getFinalValue() {
	return final_value;
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_node_LinearRampToValue(LavHandle nodeHandle, int slot, double time, double value) {
	PUB_BEGIN
	auto node = incomingObject<LavNode>(nodeHandle);
	LOCK(*node);
	auto &prop= node->getProperty(slot);
	LavLinearRampAutomator* automator = new LavLinearRampAutomator(&prop, time, value);
	//the property will throw for us if any part of the next part goes wrong.
	prop.scheduleAutomator(automator, time);
	PUB_END
}
