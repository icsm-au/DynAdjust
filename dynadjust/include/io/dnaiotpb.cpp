//============================================================================
// Name         : dnaiotpb.cpp
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
// Description  : PB2002 Tectonic Plate Boundary file io operations
//============================================================================

#include <include/io/dnaiotpb.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust {
namespace iostreams {
	
void dna_io_tpb::load_tpb_file(const string& tpb_filename, v_string_v_doubledouble_pair& global_plates)
{
	// From the standard:
	// PB2002_plates.dig.For some applications it is necessary to
	// represent plates by closed outlines.They include computing the
	// areas of plates, determining which plate a given point lies
	// within, and mapping plates as regions of contrasting color.For
	// such applications, file PB2002_plates.dig is provided.It
	// contains 52 segments, each titled with the two - letter identifier
	// of a plate.Each segment is a closed curve outlining that plate
	// in the counterclockwise direction(as seen from outside the
	// Earth).The last point in the segment is identical to the first
	// point.Because each plate boundary necessarily appears twice in
	// this file, it is about twice as large as the first.
	//
	// Tectonic plate boundary file structure is as follows.
	// Note - coordinates are in degrees and are given in a counterclockwise  
	// direction.  Since reftran uses the boost geometry package to interpolate
	// which plate a point lies in, the polygon node coordinates will need
	// to be reversed and converted to decimal degrees.
	//
	//   <plate identifier>
	//   <long,lat>
	//   <long,lat>
	//      ...
	//   <long,lat>
	//   *** end of line segment ***
	//   <plate identifier>
	//   <long,lat>
	//   <long,lat>
	//      ...
	//   <long,lat>
	//   *** end of line segment ***

	std::ifstream tpb_file;
	stringstream ss;
	ss << "load_tpb_file(): An error was encountered when opening " << tpb_filename << "." << endl;
	
	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(tpb_file, tpb_filename, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "load_tpb_file(): An error was encountered when reading from " << tpb_filename << "." << endl;

	v_doubledouble_pair plate_nodes;
	string plate_name, node_coordinates;
	vstring coordinates;

	try {

		global_plates.clear();

		while (!tpb_file.eof())			// while EOF not found
		{
			// get the plate identifier
			getline(tpb_file, plate_name);

			// blank or whitespace?
			if (trimstr(plate_name).empty())
				continue;
			
			str_toupper<int>(plate_name);

			// get the next set of plate coordinates
			getline(tpb_file, node_coordinates);

			while (!iequals(node_coordinates.substr(0, 3), "***"))
			{
				// Extract coordinates from comma delimited string
				SplitDelimitedString<string>(
					node_coordinates,				// the comma delimited string
					string(","),				// the delimiter
					&coordinates);			// the respective values

				plate_nodes.push_back(doubledouble_pair(
					DoubleFromString<double>(coordinates.at(0)),
					DoubleFromString<double>(coordinates.at(1))));

				// get the plate identifier
				getline(tpb_file, node_coordinates);
			}

			global_plates.push_back(string_v_doubledouble_pair(
				plate_name, plate_nodes));

			plate_nodes.clear();
		}

		tpb_file.close();
	}
	catch (const ios_base::failure& f) {
		if (tpb_file.eof())
		{
			tpb_file.close();
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
		if (tpb_file.eof())
		{
			tpb_file.close();
			return;
		}
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}	

	return;
}

void dna_io_tpb::load_tpp_file(const string& tpp_filename, v_plate_motion_eulers& plate_pole_parameters)
{
	// Tectonic pole parameters file structure is as follows.
	// Note - coordinates are in degrees.
	//
	//   <plate identifier> <latitude> <longitue> <pole rotation rate> <pole parameters author>
	//

	std::ifstream tpp_file;
	stringstream ss;
	ss << "load_tpp_file(): An error was encountered when opening " << tpp_filename << "." << endl;

	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(tpp_file, tpp_filename, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "load_tpp_file(): An error was encountered when reading from " << tpp_filename << "." << endl;

	plate_motion_euler pmm;
	string record;

	try {

		plate_pole_parameters.clear();
	
		while (!tpp_file.eof())			// while EOF not found
		{
			// get the plate parameters record
			getline(tpp_file, record);

			// blank or whitespace?
			if (trimstr(record).empty())
				continue;

			// Ignore lines with comments
			if (record.compare(0, 1, "*") == 0)
				continue;

			// Extract plate name
			pmm.plate_name = record.substr(0, 2);

			// Extract coordinates from comma delimited string
			pmm.pole_latitude = DoubleFromString<double>(trimstr(record.substr(3, 10)));
			pmm.pole_longitude = DoubleFromString<double>(trimstr(record.substr(12, 20)));
			
			// extract pole rotation rate
			pmm.pole_rotation_rate = DoubleFromString<double>(trimstr(record.substr(22, 29)));
	
			// extract plate motion model author
			pmm.pole_param_author = record.substr(32);
			
			plate_pole_parameters.push_back(pmm);			
		}

		tpp_file.close();
	}
	catch (const ios_base::failure& f) {
		if (tpp_file.eof())
		{
			tpp_file.close();
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
		if (tpp_file.eof())
		{
			tpp_file.close();
			return;
		}
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	return;
}

bool dna_io_tpb::validate_plate_files(v_string_v_doubledouble_pair& global_plates,
	v_plate_motion_eulers& plate_pole_parameters, string& message)
{
	if (plate_pole_parameters.size() != global_plates.size())
	{
		message = "There number of plates in the plate boundaries file and the pole parameters file are not equal.";
		return false;
	}

	//for (it_plate_motion_euler i= plate_pole_parameters.begin(), it_v_string_v_doubledouble_pair j = global_plates.begin();
	//	i!=plate_pole_parameters.end(), j!=global_plates.end(); 
	//	++i, ++j)
	
	it_v_string_v_doubledouble_pair j = global_plates.begin();
	
	for (it_plate_motion_euler i = plate_pole_parameters.begin();
		i != plate_pole_parameters.end(),
		j != global_plates.end();
		++i, ++j)
	{
		if (!boost::equals(i->plate_name, j->first))
		{
			message = "There is a mismatch between the plates in the plate boundaries file and the pole parameters file.";
			return false;
		}
	}

	return true;
}

} // dnaiostreams
} // dynadjust

