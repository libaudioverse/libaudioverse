/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/
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