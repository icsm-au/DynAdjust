//============================================================================
// Name         : dnaioaml.hpp
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
// Description  : DynAdjust associated measurement file io operations
//============================================================================

#ifndef DNAIOAML_H_
#define DNAIOAML_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/io/dnaiobms.hpp>

namespace dynadjust {
namespace iostreams {

class dna_io_aml : public dna_io_base
{
public:
	dna_io_aml(void) {};
	dna_io_aml(const dna_io_aml& aml) : dna_io_base(aml) {};
	virtual ~dna_io_aml(void) {};

	dna_io_aml& operator=(const dna_io_aml& rhs);

	void load_aml_file(const std::string& aml_filename, v_aml_pair* vbinary_aml, pvmsr_t bmsRecords);
	void write_aml_file(const std::string& aml_filename, pvUINT32 vbinary_aml);
	void write_aml_file_txt(const std::string& bms_filename, const std::string& aml_filename, pvUINT32 vbinary_aml, const pvASLPtr vAssocStnList, vdnaStnPtr* vStations);

	void create_msr_to_stn_tally(const pvASLPtr vAssocStnList, v_aml_pair& vAssocMsrList, 
		vmsrtally& stnmsrTally, vmsr_t& bmsBinaryRecords);

	void write_msr_to_stn(std::ostream &os, pvstn_t bstBinaryRecords, 
		pvUINT32 vStationList, vmsrtally& v_stnmsrTally, MsrTally* parsemsrTally);
	
protected:

};

}	// namespace measurements
}	// namespace dynadjust

#endif
