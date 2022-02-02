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

	string frx_record, version, type, param;
	
    frame_substitutions frx;
    boost::gregorian::date today(day_clock::local_day());

	// read header
	getline(frx_file, frx_record);
	frx_record = trimstr(frx_record);

	// Attempt to get the file's version
	try {
		if (iequals("!#=DNA", frx_record.substr(0, 6)))
			version = trimstr(frx_record.substr(6, 6));

		// Attempt to get the file's type
		try {
			type = trimstr(frx_record.substr(12, 3));
		}
		catch (const runtime_error& e) {
			stringstream ssError;
			ssError << "- Error: File type has not been provided in the header" << endl <<
				frx_record << endl << e.what() << endl;
			throw boost::enable_current_exception(runtime_error(ssError.str()));
		}

		// Station file
		if (!iequals(type, "frx"))
		{
			stringstream ssError;
			ssError << "- Error: A FRX file type was expected in the header" << endl <<
				frx_record << endl;
			throw boost::enable_current_exception(runtime_error(ssError.str()));
		}
	}
	catch (const runtime_error& e) {
		throw boost::enable_current_exception(runtime_error(e.what()));
	}

	double parameters[7];

	try {

		frame_subs.clear();

		while (!frx_file.eof())			// while EOF not found
		{
			// get the plate identifier
			getline(frx_file, frx_record);
			frx_record = trimstr(frx_record);

			// blank or whitespace?
			if (trimstr(frx_record).empty())
				continue;
			
			// Ignore lines with comments
			if (frx_record.compare(0, 1, "#") == 0)
				continue;

            // Ignore lines with comments
			if (frx_record.compare(0, 3, "---") == 0)
				continue;

			frx.initialise();

            // frame name
            frx.frame_name = trimstr(frx_record.substr(0, 12));
			str_toupper<int>(frx.frame_name);

            // frame epsg
            frx.frame_epsg = val_uint<UINT32, string>(trimstr(frx_record.substr(12, 6)));

            // substitute
            frx.substitute_name = trimstr(frx_record.substr(18, 12));
			str_toupper<int>(frx.substitute_name);

            // substitute epsg
            frx.substitute_epsg = val_uint<UINT32, string>(trimstr(frx_record.substr(30, 6)));

            // alignment date
            frx.alignment_epoch = dateFromString<date>(trimstr(frx_record.substr(36, 16)));

            // from date
            frx.from_epoch = dateFromString<date>(trimstr(frx_record.substr(52, 16)));

            // to date
            if (trimstr(frx_record.substr(68, 16)).empty())
			    // No date supplied?
				// Set a date 100 years into the future
				frx.to_epoch = day_clock::local_day() + years(100);
			else
                frx.to_epoch = dateFromString<date>(trimstr(frx_record.substr(68, 16)));
            
            // description
            if (frx_record.length() > 84)
                frx.frame_desc = trimstr(frx_record.substr(84, 22));
            
			if (frx_record.length() < 106)
			{
				// no parameters, just direct substitution
				frame_subs.push_back(frx);
				continue;
			}

			// initialise and extract parameters, if any
			memset(parameters, '\0', sizeof(parameters));
			
			// X
			if (frx_record.length() > 106)
			{
				param = trimstr(frx_record.substr(106, 10));
				if (!param.empty())
					parameters[0] = DoubleFromString<double>(param);
			}

			// Y
			if (frx_record.length() > 116)
			{
				param = trimstr(frx_record.substr(116, 10));
				if (!param.empty())
					parameters[1] = DoubleFromString<double>(param);
			}

			// Z
			if (frx_record.length() > 126)
			{
				param = trimstr(frx_record.substr(126, 10));
				if (!param.empty())
					parameters[2] = DoubleFromString<double>(param);
			}

			// PPM
			if (frx_record.length() > 136)
			{
				param = trimstr(frx_record.substr(136, 10));
				if (!param.empty())
					parameters[3] = DoubleFromString<double>(param);
			}

			// R1
			if (frx_record.length() > 146)
			{
				param = trimstr(frx_record.substr(146, 10));
				if (!param.empty())
					parameters[4] = DoubleFromString<double>(param);
			}

			// R2
			if (frx_record.length() > 156)
			{
				param = trimstr(frx_record.substr(156, 10));
				if (!param.empty())
					parameters[5] = DoubleFromString<double>(param);
			}

			// R3
			if (frx_record.length() > 166)
			{
				param = trimstr(frx_record.substr(166, 10));
				if (!param.empty())
					parameters[6] = DoubleFromString<double>(param);
			}

			memcpy(frx.parameters_, parameters, sizeof(frx.parameters_));
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
