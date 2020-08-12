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

void dna_io_adj::print_msr_to_stn_header(ostream& os)
{
	os << endl << "Measurements to Station " << endl;
	os << "------------------------------------------" << endl << endl;

	os << setw(STATION) << left << "Station" <<
		setw(NUMERIC_WIDTH) << right <<	"A" <<
		setw(NUMERIC_WIDTH) << right <<	"B" <<
		setw(NUMERIC_WIDTH) << right <<	"C" <<
		setw(NUMERIC_WIDTH) << right <<	"D" <<
		setw(NUMERIC_WIDTH) << right <<	"E" <<
		setw(NUMERIC_WIDTH) << right << "G" <<
		setw(NUMERIC_WIDTH) << right <<	"H" <<
		setw(NUMERIC_WIDTH) << right <<	"I" <<
		setw(NUMERIC_WIDTH) << right <<	"J" <<
		setw(NUMERIC_WIDTH) << right <<	"K" <<
		setw(NUMERIC_WIDTH) << right <<	"L" <<
		setw(NUMERIC_WIDTH) << right <<	"M" <<
		setw(NUMERIC_WIDTH) << right <<	"P" <<
		setw(NUMERIC_WIDTH) << right <<	"Q" <<
		setw(NUMERIC_WIDTH) << right <<	"R" <<
		setw(NUMERIC_WIDTH) << right <<	"S" <<
		setw(NUMERIC_WIDTH) << right <<	"V" <<
		setw(NUMERIC_WIDTH) << right <<	"X" <<
		setw(NUMERIC_WIDTH) << right <<	"Y" <<
		setw(NUMERIC_WIDTH) << right <<	"Z" <<
		// Total
		setw(STAT) << right <<	"Total" <<
		endl;

	UINT32 i, j(STATION + (NUMERIC_WIDTH * 20) + STAT);
	for (i=0; i<j; ++i)
		os << "-";

	os << endl;

	
}


void dna_io_adj::print_adj_stn_header(ostream& os)
{
	os << endl << "Adjusted Coordinates" << endl <<
		"------------------------------------------" << endl << endl;
}

void dna_io_adj::print_adj_stn_block_header(ostream& os, const UINT32& block)
{
	os << endl << "Adjusted Coordinates (Block " << block + 1 << ")" << endl <<
		"------------------------------------------" << endl << endl;
}

void dna_io_adj::print_stn_info_col_header(ostream& os, 
	const string& stn_coord_types, const UINT16& printStationCorrections)
{	
	os << setw(STATION) << left << "Station" << setw(CONSTRAINT) << left << "Const";

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
			os << right << setw(width) << CDnaStation::CoordinateName(c);
	}	
	
	os << setw(PAD2) << " " << 
		right << setw(STDDEV) << "SD(e)" << 
		right << setw(STDDEV) << "SD(n)" << 
		right << setw(STDDEV) << "SD(up)";

	j += PAD2+STDDEV+STDDEV+STDDEV+PAD2+COMMENT;

	if (printStationCorrections)
	{
		os << setw(PAD2) << " " << 
			right << setw(HEIGHT) << "Corr(e)" << 
			right << setw(HEIGHT) << "Corr(n)" << 
			right << setw(HEIGHT) << "Corr(up)";

		j += PAD2+HEIGHT+HEIGHT+HEIGHT;
	}

	os << setw(PAD2) << " " << left << "Description" << endl;

	UINT32 i;
	for (i=0; i<j; ++i)
		os << "-";
	os << endl;
}

} // dnaiostreams
} // dynadjust

