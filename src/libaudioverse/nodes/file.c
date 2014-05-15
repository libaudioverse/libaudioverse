/**Reads an entire file with Libsndfile.  At the moment, this file must be specified at construction time.*/
#include <libaudioverse/private_all.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>

struct fileinfo {
	SF_INFO info;
	float table_delta;
	LavTable **tables;
	float current_index;
};

Lav_PUBLIC_FUNCTION Lav_createFileNode(LavGraph *graph, const char* path, LavNode** destination) {
	WILL_RETURN(LavError);
	LOCK(graph->mutex);
	LavNode* retval;
	//we need some information first.  Namely, the number of channels in the file.
	struct fileinfo f= {0};
	SNDFILE *handle = sf_open(path, SFM_READ, &f.info);
	ERROR_IF_TRUE(handle == NULL, Lav_ERROR_FILE);
	unsigned int outputs = f.info.channels;
	LavError err = Lav_createNode(0, outputs, 0, Lav_NODETYPE_FILE, graph, &retval);
	ERROR_IF_TRUE(err != Lav_ERROR_NONE, err);
	retval->data = calloc(1, sizeof(f));
	ERROR_IF_TRUE(retval->data == NULL, Lav_ERROR_MEMORY);
	memcpy(retval->data, &f, sizeof(f)); //we can't forget this.

	//Now, we read the file until the end, filling a buffer; then we copy the buffer into the table and free it.
	//Todo: incredibly memory ineffficient, using at least 3 times the size of the file until the frees. Optimize, optimize, optimize.
	float* buffer = calloc((size_t)(f.info.channels*f.info.frames), sizeof(float));
	ERROR_IF_TRUE(buffer == NULL, Lav_ERROR_MEMORY);
	sf_count_t read = 0, readThisTime = 0;
	do {
		readThisTime = sf_readf_float(handle, buffer+(read*f.info.channels), f.info.frames-read);
		read += readThisTime;
	} while(readThisTime > 0);
	if(read != f.info.frames) {
		sf_close(handle);
		RETURN(Lav_ERROR_FILE);
	}

//we have to make a table for each channel, and then fill them individually.  This is, basically, horrible.
float** temp_buffers = calloc(f.info.channels, sizeof(float*));
	LavTable **tables = calloc(f.info.channels, sizeof(LavTable*));
	if(tables == NULL || temp_buffers == NULL) {
		if(tables) free(tables);
		if(temp_buffers) free(temp_buffers);
		sf_close(handle);
		RETURN(Lav_ERROR_MEMORY);
	}

	for(int offset = 0; offset < f.info.channels; ++offset) {
		err = Lav_ERROR_NONE;
		temp_buffers[offset] = calloc((size_t)f.info.frames, sizeof(float));
		if(temp_buffers[offset] == NULL) err = Lav_ERROR_MEMORY;
		if(err != Lav_ERROR_NONE) break;
		err = Lav_createTable(&tables[offset]);
		if(err != Lav_ERROR_NONE) break;
		for(unsigned int i = 0; i < f.info.frames; ++i) {
			*(temp_buffers[offset]+i) = buffer[offset+i*f.info.frames];
		}
		err = Lav_tableSetSamples(tables[offset], (unsigned int)f.info.frames, temp_buffers[offset]);
		if(err != Lav_ERROR_NONE) break;
	}
	if(err != Lav_ERROR_NONE) {
		sf_close(handle);
		free(temp_buffers);
		free(tables);
		RETURN(err);
	}

	((struct fileinfo*)(retval->data))->tables = tables;
	for(unsigned int i = 0; i < f.info.channels; i++) free(temp_buffers[i]);
	sf_close(handle);
	((struct fileinfo*)retval->data)->table_delta = (float)graph->sr/f.info.samplerate;
	STANDARD_CLEANUP_BLOCK(graph->mutex);
}

Lav_PUBLIC_FUNCTION LavError fileProcessor(LavNode* node, unsigned int count) {
	return Lav_ERROR_NONE;
}
