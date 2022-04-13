//============================================================================
// Name         : dnaiobms.hpp
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
// Description  : DynAdjust binary measurement file io operations
//============================================================================

#ifndef DNAIOBMS_H_
#define DNAIOBMS_H_

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

class dna_io_bms : public dna_io_base
{
public:
	dna_io_bms(void) {};
	dna_io_bms(const dna_io_bms& bms) : dna_io_base(bms) {};
	virtual ~dna_io_bms(void) {};

	dna_io_bms& operator=(const dna_io_bms& rhs);

	UINT16 create_msr_input_file_meta(vifm_t& vinput_file_meta, input_file_meta_t** input_file_meta);
	void load_bms_file_meta(const string& bms_filename, binary_file_meta_t& bms_meta);
	UINT32 load_bms_file(const string& bms_filename, pvmsr_t vbinary_msr, binary_file_meta_t& bms_meta);
	void write_bms_file(const string& bms_filename, pvmsr_t vbinary_msr, binary_file_meta_t& bms_meta);
	void write_bms_file(const string& bms_filename, vdnaMsrPtr* vMeasurements, binary_file_meta_t& bms_meta);

protected:

};

}	// namespace measurements
}	// namespace dynadjust

#endif
