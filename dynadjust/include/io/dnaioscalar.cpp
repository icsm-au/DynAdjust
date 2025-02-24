//============================================================================
// Name         : dnaioscalar.cpp
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

#include <include/io/dnaioscalar.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

namespace dynadjust { 
namespace iostreams {

void dna_io_scalar::load_scalar_file(const std::string& scalar_filename, pvscl_t bslScaling)
{
	std::ifstream scalar_file;
	std::stringstream ss;
	ss << "load_scalar_file(): An error was encountered when opening " << scalar_filename << "." << std::endl;

	// The contents of the scalar file is as follows:
	//
	// --------------------------------------------------------------------------------
	// GNSS BASELINE VARIANCE MATRIX SCALAR FILE
	// 
	// Scalar  count                      3 
	// Station name width                 20
	// --------------------------------------------------------------------------------
	// 
	// SCALARS
	// 
	// Station 1            Station 2               v-scale    p-scale    l-scale    h-scale
	// -------------------- -------------------- ---------- ---------- ---------- ----------
	// 409601230            409601240                 100.0          1          1          1
	// 409601230            409601250                    10          2          2          5
	// 409601230            409601260                     1          3          3         10
	
	//
	try {
		// Load scalar file.  Throws runtime_error on failure.
		file_opener(scalar_file, scalar_filename, 
			std::ios::in, ascii, true);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_scalar_file(): An error was encountered when reading from " << scalar_filename << "." << std::endl;

	bslScaling->clear();

	scalar_t bslScalars;
	std::string sBuf(""), tmp;
	UINT32 stationWidth(STATION), scalarCount(100);
	UINT32 line(0), position(0);
	
	try {
		
		// continue until "SCALARS" is found
		do 
		{
			line++;
			getline(scalar_file, sBuf);

			if (boost::iequals(trimstr(sBuf), "SCALARS"))
				break;

			if (boost::iequals(trimstr(sBuf.substr(0, 16)), "Baseline count"))
			{
				scalarCount = boost::lexical_cast<UINT16, std::string>(trimstr(sBuf.substr(PRINT_VAR_PAD)));
				continue;
			}

			if (boost::iequals(trimstr(sBuf.substr(0, 18)), "Station name width"))
			{
				stationWidth = boost::lexical_cast<UINT16, std::string>(trimstr(sBuf.substr(PRINT_VAR_PAD)));
				continue;
			}

		}
		while (!boost::iequals(trimstr(sBuf), "SCALARS"));
		
		bslScaling->reserve(scalarCount);
		
		// Okay, now get the data
		while (!scalar_file.eof())
		{
			getline(scalar_file, sBuf);

			// blank or whitespace?
			if (trimstr(sBuf).empty())			
				continue;

			if (trimstr(sBuf).length() < stationWidth+1)
				continue;

			if (boost::iequals(trimstr(sBuf.substr(0, 20)), "Station 1"))
				continue;

			if (boost::iequals(trimstr(sBuf.substr(0, 3)), "---"))
				continue;

			// Ignore lines with blank station name
			if (trimstr(sBuf.substr(0, stationWidth)).empty())			
				continue;

			// Ignore lines with blank substitute name
			if (trimstr(sBuf.substr(stationWidth, stationWidth)).empty())			
				continue;

			// initialise
			position = 0;
			bslScalars.station1 = "";
			bslScalars.station2 = "";
			bslScalars.v_scale = bslScalars.p_scale = bslScalars.l_scale = bslScalars.h_scale = 1.0;

			bslScalars.station1 = trimstr(sBuf.substr(0, stationWidth));
			position += stationWidth;
			bslScalars.station2 = trimstr(sBuf.substr(stationWidth, stationWidth));
			position += stationWidth;
			
			// v-scale present?
			if (sBuf.length() > position)
			{
				if ((tmp = trimstr(sBuf.substr(position, 10))).length() > 0)
					bslScalars.v_scale = boost::lexical_cast<double, std::string>(tmp);
			}
			else
				continue;

			position += 10;
			
			// p-scale present?
			if (sBuf.length() > position)
			{
				if ((tmp = trimstr(sBuf.substr(position, 10))).length() > 0)
					bslScalars.p_scale = boost::lexical_cast<double, std::string>(tmp);
			}
			else
				continue;

			position += 10;
			
			// l-scale present?
			if (sBuf.length() > position)
			{
				if ((tmp = trimstr(sBuf.substr(position, 10))).length() > 0)
					bslScalars.l_scale = boost::lexical_cast<double, std::string>(tmp);
			}
			else
				continue;

			position += 10;
			
			// h-scale present?
			if (sBuf.length() > position)
			{
				if ((tmp = trimstr(sBuf.substr(position, 10))).length() > 0)
					bslScalars.h_scale = boost::lexical_cast<double, std::string>(tmp);
			}
			else
				continue;
			
			bslScaling->push_back(bslScalars);
		}
	}
	catch (const std::ios_base::failure& f) {
		if (scalar_file.eof())
		{
			scalar_file.close();
			return;
		}
		ss << f.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (const std::runtime_error& e) {
		if (scalar_file.eof())
		{
			scalar_file.close();
			return;
		}
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		if (scalar_file.eof())
		{
			scalar_file.close();
			return;
		}
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}

	scalar_file.close();
}

} // dnaiostreams
} // dynadjust

