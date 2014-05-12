#include "libaudioverse.h"
Lav_PUBLIC_FUNCTION LavError createAudioOutputThread(LavGraph *graph, unsigned int blockSize, unsigned int mixAhead, void **destination);
Lav_PUBLIC_FUNCTION void stopAudioOutputThread(void* thread);