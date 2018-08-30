//============================================================================
// Name         : dnaconsts-interface.hpp
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
// Description  : DynAdjust user interface constants include file
//============================================================================

#ifndef DNACONSTS_INTERFACE_HPP
#define DNACONSTS_INTERFACE_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnatypes.hpp>

const UINT16 PROGRESS_BLOCK = 10;
const UINT16 PROGRESS_ADJ_BLOCK_12 = 12;
const UINT16 PROGRESS_ADJ_BLOCK_14 = 14;
const UINT16 PROGRESS_ADJ_BLOCK_28 = 28;

const UINT16 PROGRESS_PERCENT_04 = 4;
const UINT16 PROGRESS_PERCENT_20 = 20;
const UINT16 PROGRESS_PERCENT_29 = 29;

const UINT16 PROGRESS_PAD_30 = PROGRESS_BLOCK + PROGRESS_PERCENT_20;
const UINT16 PROGRESS_PAD_39 = PROGRESS_BLOCK + PROGRESS_PERCENT_29;

const char* const PROGRESS_BACKSPACE_04 = { "\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_12 = { "\b\b\b\b\b\b\b\b\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_14 = { "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_24 = { "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_28 = { "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_37 = { "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" };
const char* const PROGRESS_BACKSPACE_39 = { "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" };

#endif  // DNACONSTS_HPP
