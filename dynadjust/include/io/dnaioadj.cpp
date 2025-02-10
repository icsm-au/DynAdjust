//============================================================================
// Name         : dnaioadj.cpp
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

#include <include/io/dnaioadj.hpp>
#include <include/config/dnaconsts-iostream.hpp>
#include <include/measurement_types/dnastation.hpp>

using namespace dynadjust::measurements;

namespace dynadjust { 
namespace iostreams {

void dna_io_adj::print_adj_stn_header(std::ostream& os)
{
	os << std::endl << "Adjusted Coordinates" << std::endl <<
		"------------------------------------------" << std::endl << std::endl;
}

void dna_io_adj::print_adj_stn_block_header(std::ostream& os, const UINT32& block)
{
	os << std::endl << "Adjusted Coordinates (Block " << block + 1 << ")" << std::endl <<
		"------------------------------------------" << std::endl << std::endl;
}

void dna_io_adj::print_stn_info_col_header(std::ostream& os, 
	const std::string& stn_coord_types, const UINT16& printStationCorrections)
{	
	os << std::setw(STATION) << std::left << "Station" << std::setw(CONSTRAINT) << std::left << "Const";

	_it_str_const it_s;
	UINT32 width(0);

	UINT32 j(STATION+CONSTRAINT);
	bool validType(true);
	
	for (it_s=stn_coord_types.begin(); it_s!=stn_coord_types.end(); ++it_s)
	{
		char c = it_s[0];

		validType = true;

		switch (c)
		{
		case 'P':
		case 'E':
			width = LAT_EAST;
			j += LAT_EAST;
			break;
		case 'L':
		case 'N':
			width = LON_NORTH;
			j += LON_NORTH;
			break;
		case 'H':
		case 'h':
			width = HEIGHT;
			j += HEIGHT;
			break;
		case 'z':
			width = ZONE;
			j += ZONE;
			break;
		case 'X':
		case 'Y':
		case 'Z':
			width = XYZ;
			j += XYZ;
			break;
		default:
			validType = false;
		}

		if (validType)
			os << std::right << std::setw(width) << CDnaStation::CoordinateName(c);
	}	
	
	os << std::setw(PAD2) << " " << 
		std::right << std::setw(STDDEV) << "SD(e)" << 
		std::right << std::setw(STDDEV) << "SD(n)" << 
		std::right << std::setw(STDDEV) << "SD(up)";

	j += PAD2+STDDEV+STDDEV+STDDEV+PAD2+COMMENT;

	if (printStationCorrections)
	{
		os << std::setw(PAD2) << " " << 
			std::right << std::setw(HEIGHT) << "Corr(e)" << 
			std::right << std::setw(HEIGHT) << "Corr(n)" << 
			std::right << std::setw(HEIGHT) << "Corr(up)";

		j += PAD2+HEIGHT+HEIGHT+HEIGHT;
	}

	os << std::setw(PAD2) << " " << std::left << "Description" << std::endl;

	UINT32 i;
	for (i=0; i<j; ++i)
		os << "-";
	os << std::endl;
}

} // dnaiostreams
} // dynadjust

