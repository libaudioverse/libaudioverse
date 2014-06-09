/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

/**Reads an entire file with Libsndfile.  At the moment, this file must be specified at construction time.*/
#include <libaudioverse/private_all.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

struct fileinfo {
	float delta;
	float **sample_array;
	unsigned int start;
	float offset;
	unsigned int channels, frames;
	float graphSr, fileSr;
};

Lav_PUBLIC_FUNCTION LavError fileNodeProcessor(LavObject* obj);

LavPropertyTableEntry filePropertyTable[] = {
	Lav_FILE_PITCH_BEND, Lav_PROPERTYTYPE_FLOAT, "pitch_bend", {.fval = 1.0f},
};

void file_close(void* h) {
	sf_close(h);
}

Lav_PUBLIC_FUNCTION LavError Lav_createFileNode(LavObject *graph, const char* path, LavObject** destination) {
	STANDARD_PREAMBLE;
	CHECK_NOT_NULL(graph);
	CHECK_NOT_NULL(path);
	CHECK_NOT_NULL(destination);
	LavNode* node;
	//the first thing we do is open the file. If this fails, we do nothing more.
	SF_INFO info;
	SNDFILE *handle = sf_open(path, SFM_READ, &info);
	if(handle == NULL) SAFERETURN(Lav_ERROR_FILE);

	//we can associate with the memory manager and have the handle automatically close for us.
	mmanagerAssociatePointer(localMemoryManager, handle, file_close);

	sf_count_t fileBufferLength = info.channels*info.frames;
	float* fileBuffer = mmanagerMalloc(localMemoryManager, (unsigned int)((fileBufferLength+info.channels)*sizeof(float)));
	if(fileBuffer == NULL) {
		SAFERETURN(Lav_ERROR_MEMORY);
	}

	//this is the only other file-sensitive thing in the function: read everything in, and error if we can't.
	sf_count_t readSoFar = 0, readThisTime = 0;
	do {
		readThisTime = sf_readf_float(handle, fileBuffer+readSoFar*info.channels, fileBufferLength/info.channels-readSoFar);
		readSoFar += readThisTime;
	} while(readThisTime > 0);
	ERROR_IF_TRUE(readSoFar != fileBufferLength/info.channels, Lav_ERROR_FILE);
	unsigned int sr = (unsigned int)info.samplerate, channels = (unsigned int)info.channels, frames = (unsigned int)info.frames; //for sanity, and suppresses some unnecessary warnings.

	float** uninterleavedSamples = NULL;
	uninterleavedSamples = uninterleaveSamplesFast(channels, frames, fileBuffer);
	if(uninterleavedSamples == NULL) SAFERETURN(Lav_ERROR_MEMORY);

	struct fileinfo *f = calloc(1, sizeof(struct fileinfo));
	ERROR_IF_TRUE(f == NULL, Lav_ERROR_MEMORY);
	f->fileSr = (float)sr;
	f->graphSr = ((LavGraph*)graph)->sr;
	f->channels = channels;
	f->frames = frames;
	f->sample_array = uninterleavedSamples;
	f->delta = sr/((LavGraph*)graph)->sr;

	LavError err = Lav_createNode(0, channels,
sizeof(filePropertyTable)/sizeof(filePropertyTable[0]), filePropertyTable,
Lav_NODETYPE_FILE, graph, (LavObject**)&node);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	node->data = f;
	((LavObject*)node)->process = fileNodeProcessor;
	*destination = (LavObject*)node;
	SAFERETURN(Lav_ERROR_NONE);
	STANDARD_CLEANUP_BLOCK;
}

Lav_PUBLIC_FUNCTION LavError fileNodeProcessor(LavObject* obj) {
	const LavNode* node = (LavNode*)obj;
	struct fileinfo *data = node->data;
	float pitch_bend = 1.0f;
	Lav_getFloatProperty((LavObject*)node, Lav_FILE_PITCH_BEND, &pitch_bend);
	for(unsigned int i = 0; i < obj->block_size; i++) {
		if(data->start >= data->frames) {
			for(unsigned int j = 0; j < obj->num_outputs; j++) obj->outputs[j][i] = 0.0f;
			continue;
		}

		unsigned int samp1 = data->start;
		unsigned int samp2 = data->start+1;
		float weight1 = 1-data->offset;
		float weight2 = data->offset;
		for(unsigned int j = 0; j < obj->num_outputs; j++) {
			float sample = data->sample_array[j][samp1]*weight1+data->sample_array[j][samp2]*weight2;
			obj->outputs[j][i] = sample;
		}
		data->offset += data->delta*pitch_bend;
		while(data->offset >= 1) {
			data->start += 1;
			data->offset-= 1;
		}
	}
	return Lav_ERROR_NONE;
}
