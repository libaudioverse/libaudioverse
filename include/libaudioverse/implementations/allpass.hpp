/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */
#pragma once

namespace libaudioverse_implementation {

/**This is an allpass filter in direct form II and using a specified delay line type.

The line can be accessed directly via .line, and all delay setting should be done through the line directly.
This filter knows only how to tick the line to build an allpass.
Via using lineRead and tick, it is possible to nest these allpasses.

Transfer function: \frac{coefficient+z^{-delay}}{1+coefficient*z^{-delay}}.*/
template<typename delay_type>
class AllpassFilter {
	public:
	template<typename... args>
	AllpassFilter(args... delay_args): line(delay_args...) {}
	void setCoefficient(float c);
	float tick(float input);
	//These are for nesting.  Call the first one, feed it through the nested filter, call the second one.
	//This returns the internal line value.
	float beginNestedTick();
	float endNestedTick(float input, float lineValue);
	void reset();
	AllpassFilter<delay_type>* getSlave();
	void setSlave(AllpassFilter<delay_type>* slave);
	float coefficient = 1.0;
	delay_type line;
	private:
	AllpassFilter<delay_type> *slave = nullptr;
};

template<typename delay_type>
void AllpassFilter<delay_type>::setCoefficient(float c) {
	coefficient = c;
	if(slave) slave->setCoefficient(c);
}

template<typename delay_type>
float AllpassFilter<delay_type>::tick(float input) {
	//No nesting.
	return endNestedTick(input, beginNestedTick());
}

template<typename delay_type>
float AllpassFilter<delay_type>::beginNestedTick() {
	return line.computeSample();
}

template<typename delay_type>
float AllpassFilter<delay_type>::endNestedTick(float input, float lineValue) {
	float rec = input-coefficient*lineValue;
	float out = coefficient*rec+lineValue;
	line.advance(rec);
	return out;
}

template<typename delay_type>
void AllpassFilter<delay_type>::reset() {
	line.reset();
}

template<typename delay_type>
AllpassFilter<delay_type>* AllpassFilter<delay_type>::getSlave() {
	return slave;
}

template<typename delay_type>
void AllpassFilter<delay_type>::setSlave(AllpassFilter<delay_type>* s) {
	slave = s;
	//Hook up the line, too.
	if(s) line.setSlave(&(s->line));
	else line.setSlave(nullptr);
}

}