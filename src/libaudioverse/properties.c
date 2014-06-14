/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

Lav_PUBLIC_FUNCTION LavError lav_resetProperty(LavObject *obj, unsigned int slot) {
		STANDARD_PREAMBLE;
	CHECK_NOT_NULL(obj);
	LOCK(obj->mutex);
	ERROR_IF_TRUE(slot >= obj->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);
	memcpy(&(obj->properties[slot]->value), &(obj->properties[slot]->default_value), sizeof(obj->properties[slot]->value)); //yes, really.
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

#define PROPERTY_SETTER_PREAMBLE(proptype) STANDARD_PREAMBLE;\
CHECK_NOT_NULL(obj);\
LOCK(obj->mutex);\
ERROR_IF_TRUE(slot >= obj->num_properties || slot<0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(obj->properties[slot]->type != proptype, Lav_ERROR_TYPE_MISMATCH)\

Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavObject *obj, unsigned int slot, int value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	obj->properties[slot]->value.ival = value;
	SAFERETURN(Lav_ERROR_NONE);
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavObject *obj, unsigned int slot, float value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	obj->properties[slot]->value.fval = value;
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavObject *obj, unsigned int slot, double value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	obj->properties[slot]->value.dval = value;
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavObject *obj, unsigned int slot, char* value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	CHECK_NOT_NULL(value);
	char* string = strdup(value);
	obj->properties[slot]->value.sval = string;
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloat3Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT3);
	obj->properties[slot]->value.f3val[0] = v1;
	obj->properties[slot]->value.f3val[1] = v2;
	obj->properties[slot]->value.f3val[2] = v3;
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloat6Property(LavObject* obj, unsigned int slot, float v1, float v2, float v3, float v4, float v5, float v6) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT6);
	obj->properties[slot]->value.f6val[0] = v1;
	obj->properties[slot]->value.f6val[1] = v2;
	obj->properties[slot]->value.f6val[2] = v3;
	obj->properties[slot]->value.f6val[3] = v4;
	obj->properties[slot]->value.f6val[4] = v5;
	obj->properties[slot]->value.f6val[5] = v6;
	if(obj->properties[slot]->post_changed_callback) {
		obj->properties[slot]->post_changed_callback(obj, slot, obj->is_in_processor);
	}
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

#define PROPERTY_GETTER_PREAMBLE(proptype) STANDARD_PREAMBLE;\
CHECK_NOT_NULL(obj);\
LOCK(obj->mutex);\
ERROR_IF_TRUE(slot >= obj->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(proptype != obj->properties[slot]->type, Lav_ERROR_TYPE_MISMATCH)

Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavObject *obj, unsigned int slot, int *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	CHECK_NOT_NULL(destination);
	*destination = obj->properties[slot]->value.ival;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavObject* obj, unsigned int slot, float *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
CHECK_NOT_NULL(destination);
	*destination = obj->properties[slot]->value.fval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavObject *obj, unsigned int slot, double *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	CHECK_NOT_NULL(destination);
	*destination = obj->properties[slot]->value.dval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavObject *obj, unsigned int slot, char** destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	CHECK_NOT_NULL(destination);
	*destination = obj->properties[slot]->value.sval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloat3Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT3);
	CHECK_NOT_NULL(v1);
	CHECK_NOT_NULL(v2);
	CHECK_NOT_NULL(v3);
	*v1 = obj->properties[slot]->value.f3val[0];
	*v2 = obj->properties[slot]->value.f3val[1];
	*v3 = obj->properties[slot]->value.f3val[2];
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloat6Property(LavObject* obj, unsigned int slot, float* v1, float* v2, float* v3, float* v4, float* v5, float* v6) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT6);
	CHECK_NOT_NULL(v1);
	CHECK_NOT_NULL(v2);
	CHECK_NOT_NULL(v3);
	CHECK_NOT_NULL(v4);
	CHECK_NOT_NULL(v5);
	CHECK_NOT_NULL(v6);
	*v1 = obj->properties[slot]->value.f6val[0];
	*v2 = obj->properties[slot]->value.f6val[1];
	*v3 = obj->properties[slot]->value.f6val[2];
	*v4 = obj->properties[slot]->value.f6val[3];
	*v5 = obj->properties[slot]->value.f6val[4];
	*v6 = obj->properties[slot]->value.f6val[5];
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

/**Functions to get property ranges.*/

Lav_PUBLIC_FUNCTION LavError Lav_getIntPropertyRange(LavObject* obj, unsigned int slot, int* lower, int* upper) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	CHECK_NOT_NULL(lower);
	CHECK_NOT_NULL(upper);
	*lower = obj->properties[slot]->minimum_value.ival;
	*upper = obj->properties[slot]->maximum_value.ival;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatPropertyRange(LavObject* obj, unsigned int slot, float* lower, float* upper) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	CHECK_NOT_NULL(lower);
	CHECK_NOT_NULL(upper);
	*lower = obj->properties[slot]->minimum_value.fval;
	*upper = obj->properties[slot]->maximum_value.fval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoublePropertyRange(LavObject* obj, unsigned int slot, double* lower, double* upper) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	CHECK_NOT_NULL(lower);
	CHECK_NOT_NULL(upper);
	*lower = obj->properties[slot]->minimum_value.dval;
	*upper = obj->properties[slot]->maximum_value.dval;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

//Knows how to sort these.
int compareLavPropertyTableEntries(const void* a, const void* b) {
	const LavPropertyTableEntry *i1 = a, *i2 = b;
	if(i1->slot < i2->slot) return -1;
	if(i1->slot > i2->slot) return 1;
	return 0;
}

Lav_PUBLIC_FUNCTION LavProperty **makePropertyArrayFromTable(unsigned int count, LavPropertyTableEntry *table) {
	LavPropertyTableEntry *sorted_table = calloc(count, sizeof(LavPropertyTableEntry));
	if(sorted_table == NULL) return NULL;
	memcpy(sorted_table, table, count*sizeof(LavPropertyTableEntry));
	qsort(sorted_table, count, sizeof(LavPropertyTableEntry), compareLavPropertyTableEntries);

	//Allocate and fill out our array by iterating over the sorted table.
	LavProperty **property_array= calloc(count, sizeof(LavProperty*));
	if(property_array == NULL) return NULL;
	for(unsigned int i = 0; i < count; i++) {
		property_array[i] = calloc(1, sizeof(LavProperty));
		if(property_array[i] == NULL) return NULL;
		property_array[i]->type = table[i].type;
		property_array[i]->name = table[i].name;
		property_array[i]->value = table[i].default_value;
		property_array[i]->default_value = table[i].default_value;
		property_array[i]->post_changed_callback = table[i].post_changed;
	}

	return property_array;
}