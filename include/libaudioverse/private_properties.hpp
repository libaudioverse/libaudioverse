/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>
#include <functional>
#include <vector>
#include <limits>
#include <algorithm>
#include "libaudioverse.h"
#include "private_errors.hpp"

union LavPropertyValue {
	float fval;
	int ival;
	double dval;
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};

//quick range check helper...
#define RC(val, fld) if(val > maximum_value.fld || val < minimum_value.fld) throw LavErrorException(Lav_ERROR_RANGE)

class LavProperty {
	public:
	LavProperty() = default;
	LavProperty(const LavProperty&) = default;
	explicit LavProperty(int property_type): type(property_type) {}
	void reset() {value = default_value; string_value = default_string_value; farray_value = default_farray_value; iarray_value = default_iarray_value;}
	int getType() { return type;}
	void setType(int t) {type = t;}
	int isType(int t) { return type == t;} //we have to typecheck these everywhere.
	const char* getName() {return name.c_str();}
	void setName(const char* n) { name = std::string(n);}
	int getTag() {return tag;}
	void setTag(int t) {tag = t;}

	//yes, really. This is as uggly as it looks.
	int getIntValue() { return value.ival;}
	void setIntValue(int v) {
		RC(v, ival);
		value.ival = v;
		if(post_changed_callback) post_changed_callback();
	}	
	int getIntDefault() {return default_value.ival;}
	void setIntDefault(int d) {default_value.ival = d;}
	int getIntMin() {return minimum_value.ival;}
	int getIntMax() {return maximum_value.ival;}
	void setIntRange(int a, int b) {minimum_value.ival = a; maximum_value.ival = b;}

	//floats...
	float getFloatValue() {return value.fval;}
	void setFloatValue(float v) {
	RC(v, fval);
		value.fval = v;
		if(post_changed_callback) post_changed_callback();
	}
	float getFloatDefault() {return default_value.fval;}
	void setFloatDefault(float v) {default_value.fval = v;}
	float getFloatMin() {return minimum_value.fval;}
	float getFloatMax() {return maximum_value.fval;}
	void setFloatRange(float a, float b) {minimum_value.fval = a; maximum_value.fval = b;}

	//doubles...
	double getDoubleValue() {return value.dval;}
	void setDoubleValue(double v) {
		RC(v, dval);
		value.dval = v;
		if(post_changed_callback) post_changed_callback();
	}
	double getDoubleMin() {return minimum_value.dval;}
	double getDoubleMax() {return maximum_value.dval;}
	void setDoubleRange(double a, double b) {minimum_value.dval = a; maximum_value.dval = b;}
	void setDoubleDefault(double v) {default_value.dval = v;}

	//float3 vectors.
	const float* getFloat3Value() {return value.f3val;}
	const float* getFloat3Default() {return default_value.f3val;}
	void setFloat3Value(const float* const v) {
		memcpy(value.f3val, v, sizeof(float)*3);
		if(post_changed_callback) post_changed_callback();
}
	void setFloat3Value(float v1, float v2, float v3) {
		value.f3val[0] = v1; value.f3val[1] = v2; value.f3val[2] = v3;
		if(post_changed_callback) post_changed_callback();
	}
	void setFloat3Default(const float* const v) {memcpy(default_value.f3val, v, sizeof(float)*3);}
	void setFloat3Default(float v1, float v2, float v3) {default_value.f3val[0] = v1; default_value.f3val[1] = v2; default_value.f3val[2] = v3;}

	//float6 vectors.
	const float* getFloat6Value() {return value.f6val;}
	const float* getFloat6Default() {return default_value.f6val;}
	void setFloat6Value(const float* const v) {
		memcpy(&value.f6val, v, sizeof(float)*6);
		if(post_changed_callback) post_changed_callback();
	}
	void setFloat6Value(float v1, float v2, float v3, float v4, float v5, float v6) {
		value.f6val[0] = v1; value.f6val[1] = v2; value.f6val[2] = v3; value.f6val[3] = v4; value.f6val[4] = v5; value.f6val[5] = v6;
		if(post_changed_callback) post_changed_callback();
	}
	void setFloat6Default(const float* const v) {memcpy(default_value.f6val, v, sizeof(float)*6);}
	void setFloat6Default(float v1, float v2, float v3, float v4, float v5, float v6) {default_value.f6val[0] = v1; default_value.f6val[1] = v2; default_value.f6val[2] = v3; default_value.f6val[3] = v4; default_value.f6val[4] = v5; default_value.f6val[5] = v6;}

	//applies to both array properties.
	void setArrayLengthRange(unsigned int lower, unsigned int upper) { min_array_length = lower; max_array_length = upper;}
	void getArraylengthRange(unsigned int* min, unsigned int* max) {*min = min_array_length; *max = max_array_length;}

	//the float arrays.
	float readFloatArray(unsigned int index) {
		if(index >= farray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
		return farray_value[index];
	}
	void writeFloatArray(unsigned int start, unsigned int stop, float* values) {
		if(start >= farray_value.size() || stop > farray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
		for(unsigned int i = start; i < stop; i++) {
			farray_value[start] = values[i];
		}
		if(post_changed_callback) post_changed_callback();
	}
	void replaceFloatArray(unsigned int length, float* values) {
		if(length < min_array_length || length > max_array_length) throw LavErrorException(Lav_ERROR_RANGE);
		farray_value.resize(length);
		std::copy(values, values+length, farray_value.begin());
		if(post_changed_callback) post_changed_callback();
	}
	std::vector<float> getFloatArrayDefault() {
		return default_farray_value;
	}
	void setFloatArrayDefault(std::vector<float> d) {
		default_farray_value = d;
	}
	
	//the int arrays.
	int readIntArray(unsigned int index) {
		if(index >= iarray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
		return iarray_value[index];
	}
	void writeIntArray(unsigned int start, unsigned int stop, int* values) {
		if(start >= iarray_value.size() || stop > iarray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
		for(unsigned int i = start; i < stop; i++) {
			iarray_value[start] = values[i];
		}
		if(post_changed_callback) post_changed_callback();
	}
	void replaceIntArray(unsigned int length, int* values) {
		if(length < min_array_length || length > max_array_length) throw LavErrorException(Lav_ERROR_RANGE);
		iarray_value.resize(length);
		std::copy(values, values+length, iarray_value.begin());
		if(post_changed_callback) post_changed_callback();
	}
	std::vector<int> getIntArrayDefault() {
		return default_iarray_value;
	}
	void setIntArrayDefault(std::vector<int> d) {
		default_iarray_value = d;
	}

	//strings:
	const char* getStringValue() { return string_value.c_str();}
	void setStringValue(const char* s) {
		string_value = s;
		if(post_changed_callback) post_changed_callback();
	}
	const char* getStringDefault() { return default_string_value.c_str();}
	void setStringDefault(const char* s) { default_string_value = s;}

	//set the callback...
	void setPostChangedCallback(std::function<void(void)> cb) {post_changed_callback = cb;}

	private:
	int type, tag;
	LavPropertyValue value, default_value, minimum_value, maximum_value;
	std::string name, string_value, default_string_value;
	std::vector<float> farray_value, default_farray_value;
	std::vector<int> iarray_value, default_iarray_value;
	unsigned int min_array_length = 0, max_array_length = std::numeric_limits<unsigned int>::max();
	std::function<void(void)> post_changed_callback;
};

//helper methods to quickly make properties.
LavProperty* createIntProperty(const char* name, int default, int min, int max);
LavProperty* createFloatProperty(const char* name, float default, float min, float max);
LavProperty* createDoubleProperty(const char* name, double default, double min, double max);
LavProperty* createFloat3Property(const char* name, float default[3]);
LavProperty* createFloat6Property(const char* name, float default[6]);
LavProperty* createStringProperty(const char* name, const char* default);