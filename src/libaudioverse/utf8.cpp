/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/

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