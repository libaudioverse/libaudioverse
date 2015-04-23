/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <string>
#include <functional>
#include <vector>
#include <limits>
#include <algorithm>
#include <memory>
#include "../libaudioverse.h"
#include "errors.hpp"

union LavPropertyValue {
	float fval; //Actual value is in automation_buffer, kept for range checking.
	int ival;
	double dval; //Also in automation_buffer, kept for range checking.
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};

//quick range check helper...
//this disables on readonly because it is expected that the library can handle that itself, and bumping ranges for writes on readonly properties would be annoying.
#define RC(val, fld) if((val > maximum_value.fld || val < minimum_value.fld) && read_only == false) throw LavErrorException(Lav_ERROR_RANGE)

class LavBuffer;
class LavSimulation;
class LavNode;

class LavProperty {
	public:
	LavProperty() = default;
	LavProperty(const LavProperty&) = default;
	explicit LavProperty(int property_type);
	~LavProperty();
	void associateNode(LavNode* node);
	void associateSimulation(std::shared_ptr<LavSimulation> simulation);

	void reset();
	int getType();
	void setType(int t);
	int isType(int t);
	const char* getName();
	void setName(const char* n);
	int getTag();
	void setTag(int t);
	bool isReadOnly();
	void setReadOnly(bool what);
	double getSr();

	//yes, really. This is as uggly as it looks.
	int getIntValue();
	void setIntValue(int v);
	int getIntDefault();
	void setIntDefault(int d);
	int getIntMin();
	int getIntMax();
	void setIntRange(int a, int b);

	//floats...
	float getFloatValue(int i = 0);
	void setFloatValue(float v);
	float getFloatDefault();
	void setFloatDefault(float v);
	float getFloatMin();
	float getFloatMax();
	void setFloatRange(float a, float b);

	//doubles...
	double getDoubleValue(int i = 0);
	void setDoubleValue(double v);
	double getDoubleMin();
	double getDoubleMax();
	void setDoubleRange(double a, double b);
	void setDoubleDefault(double v);

	//float3 vectors.
	const float* getFloat3Value();
	const float* getFloat3Default();
	void setFloat3Value(const float* const v);
	void setFloat3Value(float v1, float v2, float v3);
	void setFloat3Default(const float* const v);
	void setFloat3Default(float v1, float v2, float v3);

	//float6 vectors.
	const float* getFloat6Value();
	const float* getFloat6Default();
	void setFloat6Value(const float* const v);
	void setFloat6Value(float v1, float v2, float v3, float v4, float v5, float v6);
	void setFloat6Default(const float* const v);
	void setFloat6Default(float v1, float v2, float v3, float v4, float v5, float v6);

	//applies to both array properties.
	//note that in the below, we again disable all range checks for properties which are read_only.
	void setArrayLengthRange(unsigned int lower, unsigned int upper);
	void getArraylengthRange(unsigned int* min, unsigned int* max);

	//the float arrays.
	float readFloatArray(unsigned int index);
	void writeFloatArray(unsigned int start, unsigned int stop, float* values);
	void replaceFloatArray(unsigned int length, float* values);
	unsigned int getFloatArrayLength();
	std::vector<float> getFloatArrayDefault();
	void setFloatArrayDefault(std::vector<float> d);
	
	//the int arrays.
	int readIntArray(unsigned int index);
	void writeIntArray(unsigned int start, unsigned int stop, int* values);
	void replaceIntArray(unsigned int length, int* values);
	unsigned int getIntArrayLength();
	std::vector<int> getIntArrayDefault();
	void setIntArrayDefault(std::vector<int> d);

	//strings:
	const char* getStringValue();
	void setStringValue(const char* s);
	const char* getStringDefault();
	void setStringDefault(const char* s);

	//Buffer properties.
	std::shared_ptr<LavBuffer> getBufferValue();
	void setBufferValue(std::shared_ptr<LavBuffer> b);

	//set the callback...
	void setPostChangedCallback(std::function<void(void)> cb);

	//Can we assume that the value for i=0 in the get* functions is the value for the whole block?
	//In other words, can we optimize the application by not computing the same thing over and over?
	//Very important for add and mul.
	bool needsARate();

	//Advance time for this property 
	void tick();

	//get/set dynamic range status.
	bool getHasDynamicRange();
	void setHasDynamicRange(bool v);

	private:
	int type, tag;
	LavPropertyValue value, default_value, minimum_value, maximum_value;
	std::string name, string_value, default_string_value;
	std::vector<float> farray_value, default_farray_value;
	std::vector<int> iarray_value, default_iarray_value;
	std::shared_ptr<LavBuffer> buffer_value = nullptr;
	unsigned int min_array_length = 0, max_array_length = std::numeric_limits<unsigned int>::max();
	std::function<void(void)> post_changed_callback;
	bool read_only = false;
	bool has_dynamic_range = false;
	LavNode* node;
	std::shared_ptr<LavSimulation> simulation;

	//These are for automation and node connections:
	double* automation_buffer = nullptr;
	int block_size= 0;
};

//helper methods to quickly make properties.
LavProperty* createIntProperty(const char* name, int default, int min, int max);
LavProperty* createFloatProperty(const char* name, float default, float min, float max);
LavProperty* createDoubleProperty(const char* name, double default, double min, double max);
LavProperty* createFloat3Property(const char* name, float default[3]);
LavProperty* createFloat6Property(const char* name, float default[6]);
LavProperty* createStringProperty(const char* name, const char* default);
LavProperty* createIntArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, int* defaultData);
LavProperty* createFloatArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, float* defaultData);
LavProperty* createBufferProperty(const char* name);
