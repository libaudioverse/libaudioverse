/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_devices.hpp>
#include <stdlib.h>
#include <string.h>

/*Note: having properties be very very very fast is important, so consequently go see the header file.*/

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

//begin public api

Lav_PUBLIC_FUNCTION LavError Lav_resetProperty(LavObject *obj, unsigned int slot) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavObject* obj, unsigned int slot, int value) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavObject *obj, unsigned int slot, float value) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavObject *obj, unsigned int slot, double value) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavObject*obj, unsigned int slot, char* value) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloat3Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloat6Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavObject*obj, unsigned int slot, int *destination) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavObject* obj, unsigned int slot, float *destination) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavObject*obj, unsigned int slot, double *destination) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavObject* obj, unsigned int slot, char** destination) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloat3Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloat6Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getIntPropertyRange(LavObject* obj, unsigned int slot, int* lower, int* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatPropertyRange(LavObject* obj, unsigned int slot, float* lower, float* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoublePropertyRange(LavObject* obj, unsigned int slot, double* lower, double* upper) {
	LOCK(*(obj->getDevice()));
	return Lav_ERROR_NONE;
}
