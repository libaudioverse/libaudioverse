#speex_resampler_cpp#

[![Windows Build status](https://ci.appveyor.com/api/projects/status/p578wmxkon1exhym?svg=true)](https://ci.appveyor.com/project/camlorn/speex-resampler-cpp)
[![Linux Build Status](https://travis-ci.org/camlorn/speex_resampler_cpp.svg?branch=master)](https://travis-ci.org/camlorn/speex_resampler_cpp)

This is a wrapper over the Speex resampler, hard-coded to expect and output floating point audio.
In addition, a function exists to resample static buffers of audio data.

This library should be self-explanatory via reading it.
The build process is the standard CMake build process:

```
mkdir build
cd build
cmake ..
make
```

The final command depends on your platform.

No tests are included here; this is an integral part of [libaudioverse](http://github.com/camlorn/libaudioverse) and [audio_io](http://github.com/camlorn/audio_io).
It was extracted from these projects to avoid code duplication and because it might be useful to others.

Code here is available under the unlicense.
Note that the license applying to the speex resampler still applies to the speex resampler.
It is included here as `license.speex` and left intact in any files takenf from that project.
