/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
class LavObject;

typedef void (*LavPropertyChangedCallback)(LavObject* obj, unsigned int slot, int isFromProcessMethod);

union LavPropertyValue {
	float fval;
	int ival;
	double dval;
	char* sval;
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};

//you're supposed to set the ranges and default via the returned references here.
//syntactically, this was the best I could come up with without accessing member variables; and I have decided that member variable access is bad.
class LavProperty {
	public:
	void reset() {value = default_value;}
	LavPropertyValue& getValue() {return value;}
	void setValue(const LavPropertyValue &v) {value = v;}
	LavPropertyValue& getMin() {return minimum_value;}
	LavPropertyValue& getMax() {return maximum_value;}
	LavPropertyValue& getDefault() {return default_value;}
	int getType() { return type;}
	void setType(int t) {type = t;}
	int isType(int t) { return type == t;} //we have to typecheck these everywhere.
	const char* getName() {return name;}
	void setName(const char* name); //this one is not inlined.
	int getTag() {return tag;}
	void setTag(int t) {tag = t;}

	private:
	int type, tag;
	LavPropertyValue value, default_value, minimum_value, maximum_value;
	char* name;
	LavPropertyChangedCallback post_changed_callback;
};