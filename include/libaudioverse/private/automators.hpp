/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once

/**The automators.

The name for this component is borrowed from Webaudio.  This file only contains the abstract definition.

Adding new automators is similar to nodes: create a file, define the class, create a creation function.

We put the creeation functions for automators in this file.*/

class LavAutomator {
	public:
	//delayTime is how long until we should execute the automator.
	LavAutomator(double delayTime): delay_time(delayTime) {}
	virtual ~LavAutomator() {}

	double getDelayTime() {return delay_time;}
	void setDelayTime(double t) {delay_time = t;}

	//Write samples to the buffer specified.
	//Return number of samples written.
	//This function is not allowed to fail, and returning 0 indicates that this automator is completed.
	//We use double for convenience.
	virtual int writeValues(int length, double* destination) = 0;

	//Get the next value that would be written.
	virtual double getNext() = 0;

	//Advance by some amount of time in seconds.
	virtual void advance(double time);

	//Whether or not we need A-rate processing.
	//Any automator that detects that it can do block-rate processing should call setNeedsARate(false).
	bool getNeedsARate() {return needs_arate;}
	void setNeedsARate(bool v) {needs_arate = v;}

	//Is this automator completed?
	bool getCompleted() {return completed;}

	//Mark this automator as completed.
	void setCompleted(bool v) {completed=v;}

	private:
	bool completed = false, needs_arate = true;
	double delay_time = 0.0;
};
