/**Implement a sample table with linear interpolation between samples and ringbuffer-like properties.

Namely, reads past the end wrap to the beginning, and reads before the beginning wrap to the end.*/
#include <libaudioverse/private_all.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Lav_PUBLIC_FUNCTION LavError Lav_createTable(LavTable** destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(destination);
	LavTable *retval = calloc(1, sizeof(LavTable));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->samples = malloc(sizeof(float)*2); //this makes it safe to always use realloc.
	retval->length = 2;
	ERROR_IF_TRUE(retval->samples == NULL, Lav_ERROR_MEMORY);
	*destination = retval;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

float tableGetSampleFast(LavTable *table, float index) {
	index = fmodf(index, table->length);
	unsigned int samp1 = (unsigned int)floorf(index);
	unsigned int samp2 = (unsigned int)ceilf(index);
	if(samp1==samp2) samp2++;
	while(index < 0) index += table->length; //wrap, if needed, so this is positive.
	float weight1 = samp2-index;
	float weight2 = index-samp1;
	return weight1*table->samples[samp1]+weight2*table->samples[samp2];
}

Lav_PUBLIC_FUNCTION LavError Lav_tableGetSample(LavTable *table, float index, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	*destination = tableGetSampleFast(table, index);
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableGetSamples(LavTable* table, float index, float delta, unsigned int count, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	CHECK_NOT_NULL(destination);
	for(unsigned int i = 0; i < count; i++) {
		*destination = tableGetSampleFast(table, index+i*delta);
		++destination;
	}
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableSetSamples(LavTable *table, unsigned int count, float* samples) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	CHECK_NOT_NULL(samples);
	ERROR_IF_TRUE(count <= 0, Lav_ERROR_RANGE);
	float *new_sample_buffer = realloc(table->samples, sizeof(float)*(count+1));
	ERROR_IF_TRUE(new_sample_buffer== NULL, Lav_ERROR_MEMORY);
	table->samples = new_sample_buffer;
	memcpy(table->samples, samples, sizeof(float)*count);
	table->samples[count] = samples[0]; //the extra slot.
	table->length = count;
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableClear(LavTable *table) {
	WILL_RETURN(LavError);
	static float clearedSample = 0.0f;
	CHECK_NOT_NULL(table);
	Lav_tableSetSamples(table, 1, &clearedSample);
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}
