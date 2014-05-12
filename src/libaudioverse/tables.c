/**Implement a sample table with linear interpolation between samples and ringbuffer-like properties.

Namely, reads past the end wrap to the beginning, and reads before the beginning wrap to the end.*/
#include <libaudioverse/private_all.h>
#include <math.h>

Lav_PUBLIC_FUNCTION lavError Lav_createTable(LavTable** destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(destination);
	LavTable *retval = calloc(1, sizeof(LavTable));
	ERROR_IF_TRUE(retval == NULL, Lav_ERROR_MEMORY);
	retval->samples = malloc(sizeof(float)); //this makes it safe to always use realloc.
	ERROR_IF_TRUE(retval->samples == NULL, Lav_ERROR_MEMORY);
	*destination = retval;
	RETURN(Lav_ERROR_NONE;
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableGetSample(LavTable *table, float seconds, float* destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	//Compute the sample.
	seconds = fmod(seconds, table->duration);
	if(seconds < 0) seconds = table->duration-seconds;
	unsigned int samp1, samp2;
	float midpoint = seconds/table->duration*table->length;
	samp1 = floorf(midpoint);
	samp2 = ceilf(midpoint();
	if(samp1 == samp2) samp2++;
	//calculate weights.
	float weight1=midpoint-floorf(midpoint);
	float weight2 = ceilf(midpoint)-midpoint;
	*destination = weight1*table->samples[samp1]+weight2*table->samples[samp2];
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableComputeSampleRange(LavTable* table, float seconds_start, float seconds_end, float time_delta, unsigned ibnt destinationLength, float* destination) {
}

Lav_PUBLIC_FUNCTION LavError Lav_tableSetSamples(LavTable *table, unsigned int count, unsigned int sr, float* samples) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	float duration = (1/(float)sr)*count;
	float *samples= realloc(table->samples, sizeof(float)*length);
	ERROR_IF_TRUE(samples == NULL, Lav_ERROR_MEMORY);
	table->duration = duration;
	table->samples = samples;
	table->length = count;
	table->sample_delta = duration/length;
	table->has_samples = 1;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}

Lav_PUBLIC_FUNCTION LavError Lav_tableClear(LavTable *table) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(table);
	float* samples;
	samples = realloc(table->samples, sizeof(float));
	table->samples = samples;
	table->length = 0;
	table->duration = 0.0f;
	table->has_samples = 0;
}
