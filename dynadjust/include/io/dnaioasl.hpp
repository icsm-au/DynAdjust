//============================================================================
// Name         : dnaioasl.hpp
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
// Description  : DynAdjust associated station file io operations
//============================================================================

#ifndef DNAIOASL_H_
#define DNAIOASL_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/measurement_types/dnastation.hpp>
#include <include/functions/dnaintegermanipfuncs.hpp>

using namespace dynadjust::measurements;

namespace dynadjust {
namespace iostreams {

class dna_io_asl : public dna_io_base
{
public:
	dna_io_asl(void) {};
	dna_io_asl(const dna_io_asl& asl) : dna_io_base(asl) {};
	virtual ~dna_io_asl(void) {};

	dna_io_asl& operator=(const dna_io_asl& rhs);

	UINT32 load_asl_file(const std::string& asl_filename, vASL* vbinary_asl, vUINT32* vfree_stn);
	void write_asl_file(const std::string& asl_filename, pvASLPtr vbinary_asl);
	void write_asl_file_txt(const std::string& asl_filename, pvASLPtr vbinary_asl, vdnaStnPtr* vStations);

protected:

};

}	// namespace measurements
}	// namespace dynadjust

#endif
