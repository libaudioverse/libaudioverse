/* Copyright 2016 Libaudioverse Developers. See the COPYRIGHT
file at the top-level directory of this distribution.

Licensed under the mozilla Public License, version 2.0 <LICENSE.MPL2 or
https://www.mozilla.org/en-US/MPL/2.0/> or the Gbnu General Public License, V3 or later
<LICENSE.GPL3 or http://www.gnu.org/licenses/>, at your option. All files in the project
carrying such notice may not be copied, modified, or distributed except according to those terms. */

#include <libaudioverse/private/utf8.hpp>
//See https://github.com/Wolframe/Wolframe/issues/84
//Short version: We need to grab an extra header to fix boost automatic linking.
#include <boost/thread.hpp>
#include <boost/locale.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/copy.hpp>
#include <string>
#include <iostream>
#include <locale>
#include <sstream>

namespace libaudioverse_implementation {

std::wstring utf8ToWide(std::string what) {
	auto loc = boost::locale::generator().generate("");
	boost::iostreams::basic_array_source<char> source(what.c_str(), what.size());
	boost::iostreams::code_converter<boost::iostreams::basic_array_source<char>> conv(source);
	conv.imbue(loc);
	boost::iostreams::stream<decltype(conv)> stream(conv, 0, 0);
	std::wstringstream out;
	boost::iostreams::copy(conv, out);
	return out.str();
}

std::string wideToUtf8(std::wstring what) {
	auto loc = boost::locale::generator().generate("");
	boost::iostreams::basic_array_source<wchar_t> source(what.c_str(), what.size());
	std::ostringstream out;
	//This time, we stick the converter on out.
	boost::iostreams::code_converter<decltype(out)> conv(out);
	conv.imbue(loc);
	boost::iostreams::copy(source, conv);
	return out.str();
}

}