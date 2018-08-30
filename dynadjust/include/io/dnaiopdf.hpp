//============================================================================
// Name         : dnaiopdf.hpp
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
// Description  : DynAdjust pdf file io operations
//============================================================================

#ifndef DNAIOPDF_H_
#define DNAIOPDF_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#if defined(_WIN32) || defined(__WIN32__)
	#include <windows.h>
	#include <winbase.h>
	#include <ShellAPI.h>		// for FindExecutable
#elif defined(__linux) || defined(sun) || defined(__unix__)
	#include <unistd.h>
#endif

#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include <include/config/dnatypes.hpp>
#include <include/exception/dnaexception.hpp>

#include <include/io/dnaiobase.hpp>

namespace dynadjust {
namespace iostreams {

class dna_io_pdf : public dna_io_base
{
public:
	dna_io_pdf(void) {};
	dna_io_pdf(const dna_io_pdf&) {};
	virtual ~dna_io_pdf(void) {};

	dna_io_pdf& operator=(const dna_io_pdf& rhs);

	string form_pdf_action_command_string(const string& pdf_filename, const string& ddename, const string& action);
	string form_pdf_close_string(const string& pdf_filename, const string& ddename);
	string form_pdf_open_string(const string& pdf_filename, const string& ddename);

protected:

};

}	// namespace measurements
}	// namespace dynadjust

#endif
