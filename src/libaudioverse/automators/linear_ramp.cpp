/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/automators.hpp>

class LavLinearRampAutomator: LavAutomator {
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
