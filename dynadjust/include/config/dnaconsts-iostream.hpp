//============================================================================
// Name         : dnaconsts-iostream.hpp
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
// Description  : DynAdjust input/output file constants header file
//============================================================================

#ifndef DNACONSTS_IOSTREAM_HPP
#define DNACONSTS_IOSTREAM_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnatypes.hpp>

const char* const CORRELATION_MATRIX = "CORR";
const char* const COVARIANCE_MATRIX = "COVA";
const char* const INFORMATION_MATRIX = "INFO";
const char* const ENDSNX = "%ENDSNX";

// 80 character line
const UINT32 OUTPUTLINELENGTH(80);
const char* const OUTPUTLINE = "--------------------------------------------------------------------------------";

const UINT32 PROGRAM_OPTIONS_LINE_LENGTH(100);
const UINT32 PRINT_LINE_LENGTH(500);

const UINT32 PRINT_VAR_PAD(35);
const UINT32 PRINT_VAL_PAD(45);
const UINT32 NUMERIC_WIDTH(8);
const UINT32 PASS_FAIL(16);
const UINT32 CHISQRLIMITS(OUTPUTLINELENGTH - PRINT_VAR_PAD - PASS_FAIL);

const UINT16 HEADER_18(18);
const UINT16 HEADER_20(20);
const UINT16 HEADER_25(25);
const UINT16 HEADER_32(32);

const UINT16 BLOCK(14);
const UINT16 NETID(14);
const UINT16 INNER(16);
const UINT16 JUNCT(16);
const UINT16 MEASR(16);
const UINT16 TOTAL(16);
const UINT16 STATION(20);
const UINT16 MSR(19);
const UINT16 OUTLIER(12);
const UINT16 PACORR(14);
const UINT16 CORR(12);
const UINT16 PREC(13);
const UINT16 REL(12);
const UINT16 STAT(11);
const UINT16 PAD2(2);
const UINT16 PAD3(3);
const UINT16 PAD(5);

// output station coordinates
const UINT16 CONSTRAINT(5);
const UINT16 LAT_EAST(14);
const UINT16 LON_NORTH(15);
const UINT16 HEIGHT(11);
const UINT16 ZONE(8);
const UINT16 XYZ(15);
const UINT16 STDDEV(10);
const UINT16 PAD5(5);
const UINT16 COMMENT(56);

const int MAX_RECORD_LENGTH = 600;
const int NULL_PAD = 4;
const int INT_BUF = 4;
const int IDENT_BUF = 8;
const int HEADER_RECORD = 100;
const int DATA_RECORD = 100;
const int OVERVIEW_RECS = 8;

#endif  // DNAOPTIONS_HPP
