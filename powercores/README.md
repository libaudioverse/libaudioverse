Powercores
==========

Higher level multithreading for C++.  This library uses C++11 features wherever possible.

Note: This is definitely a work in progress and will develop alongside [Libaudioverse](http://github.com/camlorn/libaudioverse).  unlike Libaudioverse, it is released under the unlicense, so you can use it or modify it for any reason and any purpose.  See the LICENSE file for specifics.

the purpose of this library is to provide some higher level primitives needed for [Libaudioverse](http://github.com/camlorn/libaudioverse).  While C++11 provides some lower-level features, the higher level abstractions are missing.  This library aims to fill that gap.

Current features:

- Function to safely start a thread.

-  Threadsafe Multiproducer multi-consumer queue.

- Thread pool, including support for waiting on results of a job (using `std::future`) and submitting barriers.

