//============================================================================
// Name         : dnaiobase.hpp
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
// Description  : DynAdjust file io operations
//============================================================================

#ifndef DNAIOBASE_H_
#define DNAIOBASE_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <fstream>
#include <string>
#include <iomanip>
#include <include/config/dnatypes.hpp>

#define __FILE_VERSION__ "1.0"

using namespace std;

namespace dynadjust {
namespace iostreams {


// File Info data: 
// Version identifier (10 character string), e.g. "VERSION   "
// Version value (10 character string), e.g. "       0.1"
// Created identifier (10 character string), e.g. "CREATED ON"
// Created value (10 character string), e.g. "2012-11-27"
// Application identifier (10 character string), e.g. "CREATED BY"
// Application value (10 character string), e.g. "    GEOMAP" or " DYNADJUST"
//
// For example, the beginning of the binary stream would have:
//  "VERSION          0.1CREATED ON2012-11-27CREATED BY DYNADJUST"
//  OR
//  "VERSION          0.1CREATED ON2013-03-01CREATED BY   THE_APP"

const UINT16 identifier_field_width(10);
const char* const version_header     = "VERSION   ";	// 10 characters
const char* const create_date_header = "CREATED ON";	// 10 characters
const char* const create_by_header   = "CREATED BY";	// 10 characters

class dna_io_base
{
public:
	dna_io_base(void);
	dna_io_base(const dna_io_base&);
	virtual ~dna_io_base(void);

	dna_io_base& operator=(const dna_io_base& rhs);

	inline string getVersion() const { return m_strVersion; }
	inline void setVersion(const string& version) { m_strVersion = version; }

	void writeFileInfo(std::ofstream& file_stream);
	void readFileInfo(std::ifstream& file_stream);

	void writeFileMetadata(std::ofstream& file_stream, binary_file_meta_t& file_meta);
	void readFileMetadata(std::ifstream& file_stream, binary_file_meta_t& file_meta);

protected:

	void writeVersion(std::ofstream& file_stream);
	void readVersion(std::ifstream& file_stream);
	void writeDate(std::ofstream& file_stream);
	void readDate(std::ifstream& file_stream);
	void writeApp(std::ofstream& file_stream);
	void readApp(std::ifstream& file_stream);

	string	m_strVersion;
	string	m_strDate;
	string	m_strApp;
};

}	// namespace measurements
}	// namespace dynadjust

#endif
