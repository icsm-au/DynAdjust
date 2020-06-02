//============================================================================
// Name         : dnaprocessfuncs.hpp
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
// Description  : Common process and fork functions
//============================================================================

#ifndef DNAPROCESSFUNCS_HPP_
#define DNAPROCESSFUNCS_HPP_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

// The following must come after <boost/filesystem.hpp> otherwise the following compilation error occurs:
//		U:/Boost/include/boost_1_44_0/boost/filesystem/v2/operations.hpp:1191: 
//		error: 'boost::filesystem2::create_hard_link' has not been declared
// Why?  I have no idea!
#if defined(_WIN32) || defined(__WIN32__)
	#include <windows.h>			// for CreateProcess(...) called by run_command(...)
#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
	#include <unistd.h>
	#include <sys/wait.h>
#endif

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>

using namespace std;

bool run_command(const string& exec_path_name, bool validateReturnCode=true);

#endif // DNAPROCESSFUNCS_HPP_
