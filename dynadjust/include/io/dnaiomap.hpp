//============================================================================
// Name         : dnaiomap.hpp
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
// Description  : DynAdjust station map file io operations
//============================================================================

#ifndef DNAIOMAP_H_
#define DNAIOMAP_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>

namespace dynadjust {
namespace iostreams {

class dna_io_map : public dna_io_base
{
public:
	dna_io_map(void) {};
	dna_io_map(const dna_io_map& map) : dna_io_base(map) {};
	virtual ~dna_io_map(void) {};

	dna_io_map& operator=(const dna_io_map& rhs);

	void load_map_file(const string& map_filename, pv_string_uint32_pair stnsMap);
	void write_map_file(const string& map_filename, pv_string_uint32_pair stnsMap);
	void write_map_file_txt(const string& map_filename, pv_string_uint32_pair stnsMap);

	//void load_renaming_file(const string& map_filename, pv_string_string_pair stnRenaming);
	
protected:

};

}	// namespace measurements
}	// namespace dynadjust

#endif
