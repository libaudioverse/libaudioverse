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
#include <map>
#include "../libaudioverse.h"
#include "errors.hpp"

namespace libaudioverse_implementation {

union PropertyValue {
	float fval; //Actual value is in automation_buffer, kept for range checking.
	int ival;
	double dval; //Also in automation_buffer, kept for range checking.
	float f3val[3]; //vectors.
	float f6val[6]; //orientations.
};

//quick range check helper...
//this disables on readonly because it is expected that the library can handle that itself, and bumping ranges for writes on readonly properties would be annoying.
#define RC(val, fld) if((val > maximum_value.fld || val < minimum_value.fld) && read_only == false) throw LavErrorException(Lav_ERROR_RANGE)

class Buffer;
class Simulation;
class Node;
class Automator;
class InputConnection;

class Property {
	public:
	Property() = default;
	Property(const Property&) = default;
	explicit Property(int property_type);
	~Property();
	void associateNode(Node* node);
	void associateSimulation(std::shared_ptr<Simulation> simulation);

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
	double getTime();
	std::shared_ptr<InputConnection> getInputConnection();

	void updateAutomatorIndex(double t);
	void scheduleAutomator(Automator* automator);
	//Cancels all automation after time t. T is relative to the property's current time.
	void cancelAutomators(double time);
	//yes, really. This is as uggly as it looks.
	int getIntValue();
	void setIntValue(int v);
	int getIntDefault();
	void setIntDefault(int d);
	int getIntMin();
	int getIntMax();
	void setIntRange(int a, int b);

	//floats...
	//Within one block and two reads r1 and r2, such that i1 happens-before i2:
	//The index of r1 must be strictly less than or equal to the index of r2.
	//This condition may be lifted in future.
	float getFloatValue(int i = 0);
	void setFloatValue(float v);
	float getFloatDefault();
	void setFloatDefault(float v);
	float getFloatMin();
	float getFloatMax();
	void setFloatRange(float a, float b);

	//doubles...
	//As with getFloatValue, within one block and two reads r1 and r2, such that i1 happens-before i2:
	//The index of r1 must be strictly less than or equal to the index of r2.
	//This condition may be lifted in future.
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
	float* getFloatArrayPtr();
	std::vector<float> getFloatArrayDefault();
	void setFloatArrayDefault(std::vector<float> d);
	
	//the int arrays.
	int readIntArray(unsigned int index);
	void writeIntArray(unsigned int start, unsigned int stop, int* values);
	void replaceIntArray(unsigned int length, int* values);
	unsigned int getIntArrayLength();
	int* getIntArrayPtr();
	std::vector<int> getIntArrayDefault();
	void setIntArrayDefault(std::vector<int> d);

	//strings:
	const char* getStringValue();
	void setStringValue(const char* s);
	const char* getStringDefault();
	void setStringDefault(const char* s);

	//Buffer properties.
	std::shared_ptr<Buffer> getBufferValue();
	void setBufferValue(std::shared_ptr<Buffer> b);

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
	PropertyValue value, default_value, minimum_value, maximum_value;
	std::string name, string_value, default_string_value;
	std::vector<float> farray_value, default_farray_value;
	std::vector<int> iarray_value, default_iarray_value;
	std::shared_ptr<Buffer> buffer_value = nullptr;
	unsigned int min_array_length = 0, max_array_length = std::numeric_limits<unsigned int>::max();
	std::function<void(void)> post_changed_callback;
	bool read_only = false;
	bool has_dynamic_range = false;
	Node* node;
	std::shared_ptr<Simulation> simulation;

	//These are for automation and node connections:
	int block_size= 0;
	unsigned int automator_index = 0;
	double time = 0.0, sr = 0.0;
	std::vector<Automator*> automators;
	double* value_buffer = nullptr;
	bool should_use_value_buffer = false;
	float* node_buffer=nullptr; //temporary place for putting node outputs.
	std::shared_ptr<InputConnection> incoming_nodes = nullptr; //The nodes connected to this property. Pointer to break an include cycle.
	
	//Allows protecting against duplicate ticks.
	int last_ticked=-1; //simulation starts at zero.
};

//helper methods to quickly make properties.
Property* createIntProperty(const char* name, int default, int min, int max);
Property* createFloatProperty(const char* name, float default, float min, float max);
Property* createDoubleProperty(const char* name, double default, double min, double max);
Property* createFloat3Property(const char* name, float default[3]);
Property* createFloat6Property(const char* name, float default[6]);
Property* createStringProperty(const char* name, const char* default);
Property* createIntArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, int* defaultData);
Property* createFloatArrayProperty(const char* name, unsigned int minLength, unsigned int maxLength, unsigned int defaultLength, float* defaultData);
Property* createBufferProperty(const char* name);

}