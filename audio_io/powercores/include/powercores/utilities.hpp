/**This file is part of powercores, released under the terms of the Unlicense.
See LICENSE in the root of the powercores repository for details.*/
#pragma once
#include <thread>
#include <system_error>
#include <utility>

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

}