//============================================================================
// Name         : dnaioscalar.hpp
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
// Description  : DynAdjust GNSS variance matrix scalar file handling
//============================================================================

#ifndef DNAIOSCALAR_H_
#define DNAIOSCALAR_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>
#include <include/measurement_types/dnameasurement.hpp>

using namespace dynadjust::measurements;

namespace dynadjust {
namespace iostreams {

class dna_io_scalar : public dna_io_base
{
public:
	dna_io_scalar(void) {};
	dna_io_scalar(const dna_io_scalar& scl) : dna_io_base(scl) {};
	virtual ~dna_io_scalar(void) {};

	dna_io_scalar& operator=(const dna_io_scalar& rhs);

	void load_scalar_file(const string& map_filename, pvscl_t bslScaling);

protected:

};

}	// namespace iostreams
}	// namespace dynadjust

#endif
