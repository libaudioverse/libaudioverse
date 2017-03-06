#include <speex_resampler_cpp.hpp>
#include "speex_resampler.h"

namespace speex_resampler_cpp {

const char* MemoryAllocationError::what() const noexcept {
	return "Failure to allocate memory.";
}

SpeexError::SpeexError(int c): code(c) {
}

const char* SpeexError::what() const noexcept {
	return speex_resampler_strerror(code);
}

}