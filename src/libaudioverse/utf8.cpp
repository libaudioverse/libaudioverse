/**Copyright (C) Austin Hicks, 2014-2016
This file is part of Libaudioverse, a library for realtime audio applications.
This code is dual-licensed.  It is released under the terms of the Mozilla Public License version 2.0 or the Gnu General Public License version 3 or later.
You may use this code under the terms of either license at your option.
A copy of both licenses may be found in license.gpl and license.mpl at the root of this repository.
If these files are unavailable to you, see either http://www.gnu.org/licenses/ (GPL V3 or later) or https://www.mozilla.org/en-US/MPL/2.0/ (MPL 2.0).*/

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