#include <libaudioverse/private_all.h>
#include <stdlib.h>
#include <string.h>

Lav_PUBLIC_FUNCTION LavError lav_resetProperty(LavNode *node, unsigned int slot) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(node);
	LOCK(node->graph->mutex);
	ERROR_IF_TRUE(slot >= node->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);
	memcpy(&(node->properties[slot]->value), &(node->properties[slot]->default_value), sizeof(node->properties[slot]->value)); //yes, really.
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

#define PROPERTY_SETTER_PREAMBLE(proptype) WILL_RETURN(LavError);\
CHECK_NOT_NULL(node);\
LOCK(node->graph->mutex);\
ERROR_IF_TRUE(slot >= node->num_properties || slot<=0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(node->properties[slot]->type != proptype, Lav_ERROR_TYPE_MISMATCH)\

Lav_PUBLIC_FUNCTION LavError Lav_setIntProperty(LavNode* node, unsigned int slot, int value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	node->properties[slot]->value.ival = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setFloatProperty(LavNode *node, unsigned int slot, float value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	node->properties[slot]->value.fval = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setDoubleProperty(LavNode *node, unsigned int slot, double value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	node->properties[slot]->value.dval = value;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_setStringProperty(LavNode *node, unsigned int slot, char* value) {
	PROPERTY_SETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	CHECK_NOT_NULL(value);
	char* string = strdup(value);
	node->properties[slot]->value.sval = string;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

#define PROPERTY_GETTER_PREAMBLE(proptype) WILL_RETURN(LavError);\
CHECK_NOT_NULL(node);\
CHECK_NOT_NULL(destination);\
LOCK(node->graph->mutex);\
ERROR_IF_TRUE(slot >= node->num_properties || slot < 0, Lav_ERROR_INVALID_SLOT);\
ERROR_IF_TRUE(proptype != node->properties[slot]->type, Lav_ERROR_TYPE_MISMATCH)

Lav_PUBLIC_FUNCTION LavError Lav_getIntProperty(LavNode *node, unsigned int slot, int *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_INT);
	*destination = node->properties[slot]->value.ival;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getFloatProperty(LavNode* node, unsigned int slot, float *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_FLOAT);
	*destination = node->properties[slot]->value.fval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getDoubleProperty(LavNode *node, unsigned int slot, double *destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_DOUBLE);
	*destination = node->properties[slot]->value.dval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError Lav_getStringProperty(LavNode* node, unsigned int slot, char** destination) {
	PROPERTY_GETTER_PREAMBLE(Lav_PROPERTYTYPE_STRING);
	*destination = node->properties[slot]->value.sval;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(node->graph->mutex);
}

Lav_PUBLIC_FUNCTION LavProperty *makePropertyArrayFromTable(unsigned int count, LavPropertyTableEntry *table) {
}