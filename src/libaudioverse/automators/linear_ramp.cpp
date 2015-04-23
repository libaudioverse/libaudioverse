/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>

class LavLinearRampAutomator: LavAutomator {
	public:
	LavLinearRampAutomator(double final_value, LavProperty* p, double delayTime);
	virtual double getNext() override;
	virtual void start(double currentValue, int earlyness) override;
	virtual void advance(double time) override;
	double current_value, delta, final_value;
	int ticks;
};

LavLinearRampAutomator::LavLinearRampAutomator(double finalValue, LavProperty* p, double delayTime): LavAutomator(p, delayTime)  {
	final_value = finalValue;
}

void LavLinearRampAutomator::start(double currentValue, int earlyness) {
	current_value = currentValue;
	delta = (final_value-current_value)/earlyness*sr;
	ticks = earlyness;
}

void LavLinearRampAutomator::advance(double time) {
	current_value += delta;
	ticks--;
	if(ticks == 0) setCompleted(true);
}

double LavLinearRampAutomator::getNext() {
	return current_value;
}

