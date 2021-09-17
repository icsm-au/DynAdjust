//============================================================================
// Name         : dnaiofrx.cpp
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
// Description  : Reference frame substitutions file
//============================================================================

#include <include/io/dnaiofrx.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust {
namespace iostreams {


void dna_io_frx::load_frx_file(const string& frx_filename, v_frame_substitutions& frame_subs)
{
	// Frame substitutions are as follows.
	//
	// frame       epsg  substitute  epsg  alignment       from-epoch      to-epoch        description           X       Y       Z       PPM     R1      R2      R3
    // ----------------------------------------------------------------------------------------------------------------------------------------------------------------
    // wgs84       8888  itrf1990    4912  01.01.1984      01.01.1987      01.01.1994      wgs84 (transit)       0.060   -0.517  -0.223  -0.011  0.0183  -0.0003 0.0070
    // wgs84       9053  itrf1991    4913  01.01.1991      02.01.1994      28.09.1996      wgs84 (G730)      
    //


	std::ifstream frx_file;
	stringstream ss;
	ss << "load_frx_file(): An error was encountered when opening " << frx_filename << "." << endl;
	
	try {
		// open ascii frame substitutions file.  Throws runtime_error on failure.
		file_opener(frx_file, frx_filename, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "load_frx_file(): An error was encountered when reading from " << frx_filename << "." << endl;

	string frx_record;
	
    frame_substitutions frx;
    boost::gregorian::date today(day_clock::local_day());

	try {

		frame_subs.clear();

		while (!frx_file.eof())			// while EOF not found
		{
			// get the plate identifier
			getline(frx_file, frx_record);

			// blank or whitespace?
			if (trimstr(frx_record).empty())
				continue;
			
			// Ignore lines with comments
			if (frx_record.compare(0, 1, "#") == 0)
				continue;

            // Ignore lines with comments
			if (frx_record.compare(0, 3, "---") == 0)
				continue;

            // frame name
            frx.frame_name = trimstr(frx_record.substr(0, 12));

            // frame epsg
            frx.frame_epsg = val_uint<UINT32, string>(trimstr(frx_record.substr(12, 6)));

            // substitute
            frx.substitute_name = trimstr(frx_record.substr(18, 12));

            // substitute epsg
            frx.substitute_epsg = val_uint<UINT32, string>(trimstr(frx_record.substr(30, 6)));

            // alignment date
            frx.alignment_epoch = dateFromString<date>(trimstr(frx_record.substr(36, 16)));

            // from date
            frx.from_epoch = dateFromString<date>(trimstr(frx_record.substr(52, 16)));

            // to date
            if (trimstr(frx_record.substr(68, 16)).empty())
			    frx.to_epoch = day_clock::local_day() + years(100);
			else
                frx.to_epoch = dateFromString<date>(trimstr(frx_record.substr(68, 16)));
            
            // description
            if (frx_record.length() > 84)
                frx.frame_desc = trimstr(frx_record.substr(84, 22));
            
            if (frx_record.length() < 106)
                continue;

            // parameters
            frx.parameters_[0] = DoubleFromString<double>(trimstr(frx_record.substr(106, 10)));		
            frx.parameters_[1] = DoubleFromString<double>(trimstr(frx_record.substr(116, 10)));		
            frx.parameters_[2] = DoubleFromString<double>(trimstr(frx_record.substr(126, 10)));		
            frx.parameters_[3] = DoubleFromString<double>(trimstr(frx_record.substr(136, 10)));		
            frx.parameters_[4] = DoubleFromString<double>(trimstr(frx_record.substr(146, 10)));		
            frx.parameters_[5] = DoubleFromString<double>(trimstr(frx_record.substr(156, 10)));		
            frx.parameters_[6] = DoubleFromString<double>(trimstr(frx_record.substr(166, 10)));		
            
			frame_subs.push_back(frx);
		}

		frx_file.close();
	}
	catch (const ios_base::failure& f) {
		if (frx_file.eof())
		{
			frx_file.close();
			return;
		}
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		if (frx_file.eof())
		{
			frx_file.close();
			return;
		}
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}	

	return;
}


} // dnaiostreams
} // dynadjust
