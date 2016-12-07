/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once

namespace libaudioverse_implementation {

class Property;

/**The automators.

The name for this component is borrowed from Webaudio.  This file only contains the abstract definition.

Adding new automators is similar to nodes: create a file, define the class, create a creation function.

An automator itself must be equivalent to a pure function with some parameters. bound.
Put another way, multiple calls to start *must* be supported.
Furthermore, all automators must have a well-defined final value.

Subclasses should set duration in their constructors, should they need to continue past their scheduled time.*/

class Automator {
	public:
	Automator(Property* p, double scheduledTime);
	virtual ~Automator();

	//default implementation: initial_value and initial_time are set.
	virtual void start(double initialValue, double initialTime) ;
	virtual double getValue(double time) = 0;
	virtual double getFinalValue() = 0;
	double getDuration();
	double getScheduledTime();

	protected:
	double initial_value = 0.0, initial_time = 0.0, scheduled_time = 0.0, duration = 0.0;
	Property* property = nullptr;
};

bool compareAutomators(Automator *a, Automator *b);

}