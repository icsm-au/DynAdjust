//============================================================================
// Name         : dnaioadj.hpp
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
// Description  : DynAdjust adjustment output file io operations
//============================================================================

#ifndef DNAIOADJ_H_
#define DNAIOADJ_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>

namespace dynadjust {
namespace iostreams {

class dna_io_adj : public dna_io_base
{
public:
	dna_io_adj(void) {};
	dna_io_adj(const dna_io_adj&) {};
	virtual ~dna_io_adj(void) {};

	dna_io_adj& operator=(const dna_io_adj& rhs);

	void print_stn_info_col_header(ostream& os, const string& stn_coord_types, const UINT16& printStationCorrections=false);
	void print_adj_stn_header(ostream& os);
	void print_adj_stn_block_header(ostream& os, const UINT32& block);

protected:

};

}	// namespace iostreams
}	// namespace dynadjust

#endif
