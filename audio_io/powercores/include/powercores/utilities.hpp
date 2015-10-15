/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#pragma once
#include <thread>
#include <system_error>
#include <utility>
#include <functional>

namespace powercores {

/**If using threads directly, it is required that one deal with EAGAIN.
This function wraps the std::thread constructor and automatically retries.
If any other error besides EAGAIN (std::errc::resource_unavailable_try_again) occurs, the exception is rethrown.*/
template<typename... ParamsT>
std::thread  safeStartThread(ParamsT&&... params) {
	bool retry = false;
	std::thread retval;
	do {
		try {
			retval = std::thread(params...);
			retry = false;
		}
		catch(std::system_error &e) {
			//This next line is (hopefully) a workaround for a ctritical bug in VC++2013.
			//See: https://connect.microsoft.com/VisualStudio/feedback/details/1053790
			if(e.code().value() == std::make_error_code(std::errc::resource_unavailable_try_again).value()) retry = true;
			else throw;
		}
	} while(retry);
	return retval;
}

/**Get a unique identifier for a thread.
This is guaranteed to be unique for up to the size limit of long long threads, and does not reuse identifiers.
std::thread::id does reuse identifiers, which causes problems for some applications.*/
long long getThreadId();

//Internal helper for the following function, do not use.
void atThreadExitImpl(std::function<void(void)> what);

/**Run a callable at thread exit.
This has all of the restrictions of a destructor.  Arguments and the callable object are copied.  Order of calls is not guaranteed.*/
template<typename CallableT, typename... ArgsT>
void atThreadExit(CallableT&& callable, ArgsT&&... args) {
	atThreadExitImpl([callable, args...]() {callable(args...);});
}

}