//============================================================================
// Name         : dnaiostreamfuncs.hpp
// Author       : Roger Fraser
// Contributors :
// Version      : 1.00
// Copyright    : Copyright 2017 Geoscience Australia
//
//                Licensed under the Apache License, Version 2.0 (the "License");
//                you may not use this file except in compliance with the License.
//                You may obtain a copy of the License at
//               
//                http ://www.apache.org/licenses/LICENSE-2.0
//               
//                Unless required by applicable law or agreed to in writing, software
//                distributed under the License is distributed on an "AS IS" BASIS,
//                WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//                See the License for the specific language governing permissions and
//                limitations under the License.
//
// Description  : Common file stream Functions
//============================================================================

#ifndef DNAIOSTREAMFUNCS_HPP_
#define DNAIOSTREAMFUNCS_HPP_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <iostream>
#include <string>

#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception_ptr.hpp>

#include <include/config/dnaversion-stream.hpp>
#include <include/config/dnaconsts-iostream.hpp>
#include <include/config/dnaversion.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::timer;
using namespace boost::posix_time;
using namespace boost::gregorian;


//template<class T>
//std::ostream &operator<<(std::ostream &output, T const &input) {
//    T::size_type size = input.size();
//
//    output << size << "\n";
//    std::copy(input.begin(), input.end(), 
//         std::ostream_iterator<T::value_type>(output, "\n"));
//
//    return output;
//}
//
//template<class T>
//std::istream &operator>>(std::istream &input, T &output) {
//    T::size_type size, i;
//
//    input >> size;
//    output.resize(size);
//    std::copy_n(
//        std::istream_iterator<t::value_type>(input),
//        size,
//        output.begin());
//
//    return input;
//}


template <typename T>
void file_opener(
	T* stream,
	const string& str, 
	ios_base::openmode mode=ios_base::out,	// default: output
	const iosMode type=ascii,				// default: ascii
	bool fileMustExist=false)				// default: no need for file to exist
{
	file_opener(*stream, str, mode, type, fileMustExist);
}

template <typename T>
void file_opener(
	T& stream,
	const string& str, 
	ios_base::openmode mode=ios_base::out,	// default: output
	const iosMode type=ascii,				// default: ascii
	bool fileMustExist=false)				// default: no need for file to exist
{
	try {
		stream.open(str.c_str(), mode);
		stream.exceptions (/*ios_base::eofbit | */ios_base::badbit | ios_base::failbit);
		stream.iword(0) = type;
	}
	catch (const ios_base::failure& f) {
		stringstream ss;
		if (fileMustExist && !boost::filesystem::exists(str.c_str()))
			ss << "file_opener(): Can't find " << str << ".";
		else
			ss << "file_opener(): An error was encountered when opening " << 
				str << ". \n  Check that the file is not already opened.";
		ss << endl << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	if (!stream.good()) {
		stringstream ss;
		ss << "file_opener(): An error was encountered when opening " << 
			str << ". \n  Check that the file is not already opened.";
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
}

template <typename T>
void print_file_header(
	T& stream,
	const string& header)
{
	// Print formatted header
	stream << OUTPUTLINE << endl;
	stream << header << endl << endl;

	// version
	output_version(stream, true);
	stream << endl;

	// build
	output_build(stream, true);
	stream << endl;

	// File creation time
	stream << setw(PRINT_VAR_PAD) << left << "File created:";
	ostringstream datetime_ss;
	time_facet* p_time_output = new time_facet;
	locale special_locale (locale(""), p_time_output);
	// special_locale takes ownership of the p_time_output facet
	datetime_ss.imbue (special_locale);
	(*p_time_output).format("%A, %d %B %Y, %X");
	datetime_ss << second_clock::local_time();
	stream << datetime_ss.str().c_str() << endl;
}


#endif // DNAIOSTREAMFUNCS_HPP_
