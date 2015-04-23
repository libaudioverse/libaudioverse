/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private/properties.hpp>
#include <libaudioverse/private/automators.hpp>
#include <libaudioverse/private/node.hpp>
#include <libaudioverse/private/simulation.hpp>
#include <stdlib.h>
#include <string.h>

LavProperty::LavProperty(int property_type): type(property_type) {}

void LavProperty::associateNode(LavNode* node) {
	this->node = node;
}

void LavProperty::associateSimulation(std::shared_ptr<LavSimulation> simulation) {
	this->simulation = simulation;
}

void LavProperty::reset() {
	value = default_value;
	string_value = default_string_value;
	farray_value = default_farray_value;
	iarray_value = default_iarray_value;
	buffer_value=nullptr;
	if(post_changed_callback) post_changed_callback();
}

int LavProperty::getType() {
	return type;
}

void LavProperty::setType(int t) {
	type = t;
}

int LavProperty::isType(int t) { 
	return type == t;
}

const char* LavProperty::getName() {
	return name.c_str();
}

void LavProperty::setName(const char* n) {
	name = std::string(n);
}

int LavProperty::getTag() {
	return tag;
}

void LavProperty::setTag(int t) {
	tag = t;
}

bool LavProperty::isReadOnly() {
	return read_only;
}

void LavProperty::setReadOnly(bool what) {
	read_only = what;
}

int LavProperty::getIntValue() {
	return value.ival;
}

void LavProperty::setIntValue(int v) {
	RC(v, ival);
	value.ival = v;
	if(post_changed_callback) post_changed_callback();
}	

int LavProperty::getIntDefault() {
	return default_value.ival;
}

void LavProperty::setIntDefault(int d) {
	default_value.ival = d;
}

int LavProperty::getIntMin() {
	return minimum_value.ival;
}

int LavProperty::getIntMax() {
	return maximum_value.ival;
}

void LavProperty::setIntRange(int a, int b) {
	minimum_value.ival = a;
	maximum_value.ival = b;
}


float LavProperty::getFloatValue(int i) {
	return value.fval;
}

void LavProperty::setFloatValue(float v) {
	RC(v, fval);
	value.fval = v;
	if(post_changed_callback) post_changed_callback();
}

float LavProperty::getFloatDefault() {
	return default_value.fval;
}

void LavProperty::setFloatDefault(float v) {
	default_value.fval = v;
}

float LavProperty::getFloatMin() {
	return minimum_value.fval;
}

float LavProperty::getFloatMax() {
	return maximum_value.fval;
}

void LavProperty::setFloatRange(float a, float b) {
	minimum_value.fval = a; maximum_value.fval = b;
}

//doubles...
double LavProperty::getDoubleValue(int i) {
	return value.dval;
}

void LavProperty::setDoubleValue(double v) {
	RC(v, dval);
	value.dval = v;
	if(post_changed_callback) post_changed_callback();
}

double LavProperty::getDoubleMin() {
	return minimum_value.dval;
}

double LavProperty::getDoubleMax() {
	return maximum_value.dval;
}

void LavProperty::setDoubleRange(double a, double b) {
	minimum_value.dval = a;
	maximum_value.dval = b;
}

void LavProperty::setDoubleDefault(double v) {
	default_value.dval = v;
}

const float* LavProperty::getFloat3Value(int i) {
	return value.f3val;
}

const float* LavProperty::getFloat3Default() {
	return default_value.f3val;
}

void LavProperty::setFloat3Value(const float* const v) {
	memcpy(value.f3val, v, sizeof(float)*3);
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::setFloat3Value(float v1, float v2, float v3) {
	value.f3val[0] = v1;
	value.f3val[1] = v2;
	value.f3val[2] = v3;
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::setFloat3Default(const float* const v) {
	memcpy(default_value.f3val, v, sizeof(float)*3);
}

void LavProperty::setFloat3Default(float v1, float v2, float v3) {
	default_value.f3val[0] = v1;
	default_value.f3val[1] = v2;
	default_value.f3val[2] = v3;
}

const float* LavProperty::getFloat6Value(int i) {
	return value.f6val;
}

const float* LavProperty::getFloat6Default() {
	return default_value.f6val;
}

void LavProperty::setFloat6Value(const float* const v) {
	memcpy(&value.f6val, v, sizeof(float)*6);
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::setFloat6Value(float v1, float v2, float v3, float v4, float v5, float v6) {
	value.f6val[0] = v1;
	value.f6val[1] = v2;
	value.f6val[2] = v3;
	value.f6val[3] = v4;
	value.f6val[4] = v5;
	value.f6val[5] = v6;
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::setFloat6Default(const float* const v) {
	memcpy(default_value.f6val, v, sizeof(float)*6);
}

void LavProperty::setFloat6Default(float v1, float v2, float v3, float v4, float v5, float v6) {
	default_value.f6val[0] = v1;
	default_value.f6val[1] = v2;
	default_value.f6val[2] = v3;
	default_value.f6val[3] = v4;
	default_value.f6val[4] = v5;
	default_value.f6val[5] = v6;
}

void LavProperty::setArrayLengthRange(unsigned int lower, unsigned int upper) {
	min_array_length = lower;
	max_array_length = upper;
}

void LavProperty::getArraylengthRange(unsigned int* min, unsigned int* max) {
	*min = min_array_length;
	*max = max_array_length;
}

float LavProperty::readFloatArray(unsigned int index) {
	if(index >= farray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return farray_value[index];
}

void LavProperty::writeFloatArray(unsigned int start, unsigned int stop, float* values) {
	if(start >= farray_value.size() || stop > farray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
	for(unsigned int i = start; i < stop; i++) {
		farray_value[start] = values[i];
	}
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::replaceFloatArray(unsigned int length, float* values) {
	if((length < min_array_length || length > max_array_length) && read_only == false) throw LavErrorException(Lav_ERROR_RANGE);
	farray_value.resize(length);
	std::copy(values, values+length, farray_value.begin());
	if(post_changed_callback) post_changed_callback();
}

unsigned int LavProperty::getFloatArrayLength() {
	return farray_value.size();
}

std::vector<float> LavProperty::getFloatArrayDefault() {
	return default_farray_value;
}

void LavProperty::setFloatArrayDefault(std::vector<float> d) {
	default_farray_value = d;
}

int LavProperty::readIntArray(unsigned int index) {
	if(index >= iarray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
	return iarray_value[index];
}

void LavProperty::writeIntArray(unsigned int start, unsigned int stop, int* values) {
	if(start >= iarray_value.size() || stop > iarray_value.size()) throw LavErrorException(Lav_ERROR_RANGE);
	for(unsigned int i = start; i < stop; i++) {
		iarray_value[start] = values[i];
	}
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::replaceIntArray(unsigned int length, int* values) {
	if((length < min_array_length || length > max_array_length) && read_only == false) throw LavErrorException(Lav_ERROR_RANGE);
	iarray_value.resize(length);
	std::copy(values, values+length, iarray_value.begin());
	if(post_changed_callback) post_changed_callback();
}

unsigned int LavProperty::getIntArrayLength() {
	return iarray_value.size();
}

std::vector<int> LavProperty::getIntArrayDefault() {
	return default_iarray_value;
}

void LavProperty::setIntArrayDefault(std::vector<int> d) {
	default_iarray_value = d;
}

const char* LavProperty::getStringValue() {
	return string_value.c_str();
}

void LavProperty::setStringValue(const char* s) {
	string_value = s;
	if(post_changed_callback) post_changed_callback();
}

const char* LavProperty::getStringDefault() {
	return default_string_value.c_str();
}

void LavProperty::setStringDefault(const char* s) {
	default_string_value = s;
}

std::shared_ptr<LavBuffer> LavProperty::getBufferValue() {
	return buffer_value;
}

void LavProperty::setBufferValue(std::shared_ptr<LavBuffer> b) {
	buffer_value=b;
	if(post_changed_callback) post_changed_callback();
}

void LavProperty::setPostChangedCallback(std::function<void(void)> cb) {
	post_changed_callback = cb;
}

bool LavProperty::needsARate() {
	return false;
}

void LavProperty::tick() {
}

bool LavProperty::getHasDynamicRange() {
	return has_dynamic_range;
}

void LavProperty::setHasDynamicRange(bool v) {
	has_dynamic_range = v;
}

//Property creators.


LavProperty* createIntProperty(const char* name, int default, int min, int max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_INT);
	retval->setIntDefault(default);
	retval->setIntRange(min, max);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloatProperty(const char* name, float default, float min, float max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT);
	retval->setName(name);
	retval->setFloatDefault(default);
	retval->setFloatRange(min, max);
	retval->reset();
	return retval;
}

LavProperty* createDoubleProperty(const char* name, double default, double min, double max) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_DOUBLE);
	retval->setDoubleDefault(default);
	retval->setDoubleRange(min, max);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloat3Property(const char* name, float default[3]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT3);
	retval->setFloat3Default(default);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createFloat6Property(const char* name, float v[6]) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_FLOAT6);
	retval->setFloat6Default(v);
	retval->setName(name);
	retval->reset();
	return retval;
}	

LavProperty* createStringProperty(const char* name, const char* default) {
	LavProperty* retval = new LavProperty(Lav_PROPERTYTYPE_STRING);
	retval->setStringDefault(default);
	retval->setName(name);
	retval->reset();
	return retval;
}

LavProperty* createIntArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, int* defaultData) {
	auto prop = new LavProperty(Lav_PROPERTYTYPE_INT_ARRAY);
	prop->setArrayLengthRange(minLength, maxLength);
	std::vector<int> new_default;
	new_default.resize(defaultLength);
	std::copy(defaultData, defaultData+defaultLength, new_default.begin());
	prop->setIntArrayDefault(new_default);
	prop->setName(name);
	prop->reset();	
	return prop;
}

LavProperty* createFloatArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, float* defaultData) {
	auto prop = new LavProperty(Lav_PROPERTYTYPE_FLOAT_ARRAY);
	prop->setArrayLengthRange(minLength, maxLength);
	std::vector<float> new_default;
	new_default.resize(defaultLength);
	std::copy(defaultData, defaultData+defaultLength, new_default.begin());
	prop->setFloatArrayDefault(new_default);
	prop->setName(name);
	prop->reset();	
	return prop;
}

LavProperty* createBufferProperty(const char* name) {
	LavProperty* prop=new LavProperty(Lav_PROPERTYTYPE_BUFFER);
	prop->reset();
	return prop;
}
