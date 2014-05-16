/**Reads an entire file with Libsndfile.  At the moment, this file must be specified at construction time.*/
#include <libaudioverse/private_all.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

struct fileinfo {
	float table_delta;
	LavTable **tables;
	float current_index;
	unsigned int channels, frames;
	float graphSr, fileSr;
};

Lav_PUBLIC_FUNCTION LavError fileNodeProcessor(LavNode* node, unsigned int samples);

Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavGraph *graph, const char* path, LavNode** destination) {
	WILL_RETURN(LavError);
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(path);
	CHECK_NOT_NULL(destination);
	LavNode* node;
	//the first thing we do is open the file. If this fails, we do nothing more.
	SF_INFO info;
	SNDFILE *handle = sf_open(path, SFM_READ, &info);
	if(handle == NULL) RETURN(Lav_ERROR_FILE);

	//For the next little bit, we're holding onto an open file handle.  Do this part and then get it closed, because this will allow continued use of RETURN macro without huge if blocks.
	sf_count_t fileBufferLength = info.channels*info.frames;
	float* fileBuffer = malloc((size_t)(fileBufferLength*sizeof(float)));
	if(fileBuffer == NULL) {
		sf_close(handle);
		return(Lav_ERROR_MEMORY);
	}

	//this is the only other file-sensitive thing in the function: read everything in, and error if we can't.
	sf_count_t readSoFar = 0, readThisTime = 0;
	do {
		readThisTime = sf_readf_float(handle, fileBuffer+readSoFar*info.channels, fileBufferLength/info.channels-readSoFar);
		readSoFar += readThisTime;
	} while(readThisTime > 0);
	if(readSoFar != fileBufferLength/info.channels) {
		sf_close(handle);
		free(fileBuffer);
		RETURN(Lav_ERROR_FILE);
	}

	//After this, we are done being concerned about files.
	sf_close(handle);

	unsigned int sr = (unsigned int)info.samplerate, channels = (unsigned int)info.channels, frames = (unsigned int)info.frames; //for sanity, and suppresses some unnecessary warnings.

	//Get some tables.
	LavTable** tables = calloc(channels, sizeof(LavTable*));
	for(unsigned int i = 0; i < channels; i++) {
		LavError err = Lav_createTable(&tables[i]);
		ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	}

	float** uninterleavedSamples = NULL;
	uninterleavedSamples = uninterleaveSamplesFast(channels, frames, fileBuffer);
	if(uninterleavedSamples == NULL) RETURN(Lav_ERROR_MEMORY);

	//Fill the tables.
	for(unsigned int i = 0; i < channels; i++) {
		LavError err = Lav_tableSetSamples(tables[i], frames, uninterleavedSamples[i]);
		ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	}

	//We have the tables.  Allocate and fill out a fileinfo struct with it all.
	struct fileinfo *f = calloc(1, sizeof(struct fileinfo));
	ERROR_IF_TRUE(f == NULL, Lav_ERROR_MEMORY);
	f->fileSr = (float)sr;
	f->graphSr = graph->sr;
	f->channels = channels;
	f->frames = frames;
	f->tables = tables;
	f->table_delta = sr/graph->sr;

	LavError err = Lav_createNode(0, 1, 0, Lav_NODETYPE_FILE, graph, &node);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	node->data = f;
	node->process = fileNodeProcessor;
	*destination = node;
	RETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError fileNodeProcessor(LavNode* node, unsigned int samples) {
	WILL_RETURN(LavError);
	struct fileinfo *data = node->data;
	for(unsigned int i = 0; i < samples; i++) {
		if(data->current_index >= data->frames) {
			for(unsigned int j = 0; j < node->num_outputs; j++) Lav_bufferWriteSample(node->outputs+j, 0.0f);
			continue;
		}
		for(unsigned int j = 0; j < node->num_outputs; j++) {
			LavTable *table = data->tables[j];
			float sample = tableGetSampleFast(table, data->current_index);
			Lav_bufferWriteSample(node->outputs+j, sample);
		}
	data->current_index += data->table_delta;
	}
	RETURN(Lav_ERROR_NONE);
	BEGIN_CLEANUP_BLOCK
	DO_ACTUAL_RETURN;
}
