/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>

union LavPropertyValue {
	float fval;
	int ival;
	double dval;
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};

class LavProperty {
	public:
	LavProperty(int property_type): type(property_type) {}
	void reset() {value = default_value; string_value = default_string_value;}
	int getType() { return type;}
	void setType(int t) {type = t;}
	int isType(int t) { return type == t;} //we have to typecheck these everywhere.
	std::string getName() {return name;}
	void setName(std::string n) { name = n;}
	int getTag() {return tag;}
	void setTag(int t) {tag = t;}

	//yes, really. This is as uggly as it looks.
	int getIntValue() { return value.ival;}
	void setIntValue(int v) {value.ival = v;}
	int getIntDefault() {return default_value.ival;}
	void setIntDefault(int d) {default_value.ival = d;}
	int getIntMin() {return minimum_value.ival;}
	int getIntMax() {return maximum_value.ival;}
	void setIntRange(int a, int b) {minimum_value.ival = a; maximum_value.ival = b;}

	//floats...
	float getFloatValue() {return value.fval;}
	void setFloatValue(float v) {value.fval = v;}
	float getFloatDefault() {return default_value.fval;}
	void setFloatDefault(float v) {default_value.fval = v;}
	float getFloatMin() {return minimum_value.fval;}
	float getFloatMax() {return maximum_value.fval;}
	void setFloatRange(float a, float b) {minimum_value.fval = a; maximum_value.fval = b;}

	//doubles...
	double getDoubleValue() {return value.dval;}
	void setDoubleValue(double v) {value.dval = v;}
	double getDoubleMin() {return minimum_value.dval;}
	double getDoubleMax() {return maximum_value.dval;}
	void setDoubleRange(double a, double b) {minimum_value.dval = a; maximum_value.dval = b;}
	void setDoubleDefault(double v) {default_value.dval = v;}

	//float3 vectors.
	const float* getFloat3value() {return value.f3val;}
	const float* getFloat3Default() {return default_value.f3val;}
	void setFloat3Value(const float* const v) {memcpy(value.f3val, v, sizeof(float)*3);}
	void setFloat3Value(float v1, float v2, float v3) {value.f3val[0] = v1; value.f3val[1] = v2; value.f3val[2] = v3;}
	void setFloat3Default(const float* const v) {memcpy(default_value.f3val, v, sizeof(float)*3);}
	void setFloat3Default(float v1, float v2, float v3) {default_value.f3val[0] = v1; default_value.f3val[1] = v2; default_value.f3val[2] = v3;}

	//float6 vectors.
	const float* getFloat6value() {return value.f6val;}
	const float* getFloat6Default() {return default_value.f6val;}
	void setFloat6Value(const float* const v) {memcpy(&value.f6val, v, sizeof(float)*6);}
	void setFloat6Value(float v1, float v2, float v3, float v4, float v5, float v6) {value.f6val[0] = v1; value.f6val[1] = v2; value.f6val[2] = v3; value.f6val[3] = v4; value.f6val[4] = v5; value.f6val[5] = v6;}
	void setFloat6Default(const float* const v) {memcpy(default_value.f6val, v, sizeof(float)*6);}
	void setFloat6Default(float v1, float v2, float v3, float v4, float v5, float v6) {default_value.f6val[0] = v1; default_value.f6val[1] = v2; default_value.f6val[2] = v3; default_value.f6val[3] = v4; default_value.f6val[4] = v5; default_value.f6val[5] = v6;}

	//strings:
	std::string getStringValue() { return string_value;}
	void setStringValue(std::string s) { string_value = s;}
	std::string getStringDefault() { return default_string_value;}
	void setStringDefault(std::string s) { default_string_value = s;}

	private:
	int type, tag;
	LavPropertyValue value, default_value, minimum_value, maximum_value;
	std::string name, string_value, default_string_value;
};

//helper methods to quickly make properties.
LavProperty* createIntProperty(int default, int min, int max);
LavProperty* createFloatProperty(float default, float min, float max);
LavProperty* createDoubleProperty(float default, float min, float max);
LavProperty* createFloat3Property(float default[3]);
LavProperty* createFloat6Property(float default[6]);
LavProperty* createStringProperty(std::string default);