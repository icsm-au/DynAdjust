//============================================================================
// Name         : dnastringfuncs.hpp
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
// Description  : Common String Functions
//============================================================================

#ifndef DNASTRINGFUNCS_HPP_
#define DNASTRINGFUNCS_HPP_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <stdarg.h>
#include <sstream>
#include <iomanip>

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>

using namespace std;

// Returns the number of fields with valid data
int GetFields(char *line, char delim, bool multiple_delim_as_one, const char *fmt, ...);
void fileproc_help_header(string* msg);
void dynaml_header(ostream& os, const string& fileType, const string& referenceFrame, const string& epoch);
void dynaml_footer(ostream& os);
void dynaml_comment(ostream& os, const string& comment);
void dna_header(ostream& os, const string& fileVersion, const string& fileType, const string& reference_frame, const string& epoch_version, const size_t& count);
void dna_comment(ostream& os, const string& comment);
void dnaproj_header(ostream& os, const string& comment);
void dnaproj_comment(ostream& os, const string& comment);

string snx_softwarehardware_text();

#endif // DNASTRINGFUNCS_HPP_