//============================================================================
// Name         : dnaiotbu.cpp
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
// Description  : Type B uncertainty file io and helps
//============================================================================

#include <include/io/dnaiotbu.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust {
namespace iostreams {

void dna_io_tbu::read_tbu_header(std::ifstream* ptr, string& version, INPUT_DATA_TYPE& idt)
{
	string sBuf;
	getline((*ptr), sBuf);
	sBuf = trimstr(sBuf);

	// Set the default version
	version = "1.00";

	// Attempt to get the file's version
	try {
		if (iequals("!#=DNA", sBuf.substr(0, 6)))
			version = trimstr(sBuf.substr(6, 6));
	}
	catch (const runtime_error& e) {
		throw boost::enable_current_exception(runtime_error(e.what()));
	}

	string type;
	// Attempt to get the file's type
	try {
		type = trimstr(sBuf.substr(12, 3));
	}
	catch (const runtime_error& e) {
		stringstream ssError;
		ssError << "- Error: File type has not been provided in the header" << endl <<
			sBuf << endl << e.what() << endl;
		throw boost::enable_current_exception(runtime_error(ssError.str()));
	}

	// Station file
	if (iequals(type, "tbu"))
		idt = tbu_data;
	else
	{
		idt = unknown;
		stringstream ssError;
		ssError << "The supplied filetype '" << type << "' is not recognised" << endl;
		throw boost::enable_current_exception(runtime_error(ssError.str()));
	}
}
	

void dna_io_tbu::load_tbu_file(const string& tbu_filename, v_type_b_uncertainty& type_b_uncertainties)
{
	// Type B uncertainty file structure is as follows.
	// Note - uncertainties are in metres, in the local reference frame (e,n,u)
	// and are given at 1 sigma (68%). 
	//
	//   station id (20 chars)  east uncertainty (13) north uncertainty (13) up uncertainty (13)
	//      ...
	//   EOF

	std::ifstream tbu_file;
	stringstream ss;
	ss << "load_tbu_file(): An error was encountered when opening " << tbu_filename << "." << endl;
	
	INPUT_DATA_TYPE idt;
	string version;

	try {
		// open ascii plate boundaries file.  Throws runtime_error on failure.
		file_opener(tbu_file, tbu_filename, ios::in, ascii, true);

		// read header information
		read_tbu_header(&tbu_file, version, idt);

	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "load_tbu_file(): An error was encountered when reading from " << tbu_filename << "." << endl;

	string record, stn;
	vstring typeb_values;
	const UINT16 STDDEVn(STATION + STDDEV);
	const UINT16 STDDEVu(STATION + STDDEV + STDDEV);

	type_b_uncertainty typeBUncertainties;

	type_b_uncertainties.clear();

	try {

		typeb_values.resize(3);

		while (!tbu_file.eof())			// while EOF not found
		{
			typeb_values.at(0) = "";
			typeb_values.at(1) = "";
			typeb_values.at(2) = "";

			// get the plate identifier
			getline(tbu_file, record);

			// blank or whitespace?
			if (trimstr(record).empty())
				continue;

			// Ignore lines with comments
			if ((record.compare(0, 1, "*") == 0))
				continue;

			stn = trimstr(record.substr(0, STATION));
			typeBUncertainties.set_station_id_str(stn);
			
			// east uncertainty (may be blank)
			if (record.length() > STDDEVn)
				typeb_values.at(0) = trimstr(record.substr(STATION, STDDEV));
			else
				typeb_values.at(0) = trimstr(record.substr(STATION));

			// north uncertainty (may be blank)
			if (record.length() > STDDEVu)
				typeb_values.at(1) = trimstr(record.substr(STDDEVn, STDDEV));
			else
				typeb_values.at(1) = trimstr(record.substr(STDDEVn));
			
			// up uncertainty (may be blank)
			typeb_values.at(2) = "";
			if (record.length() > STDDEVu)
				typeb_values.at(2) = trimstr(record.substr(STDDEVu));

			// assign values
			typeBUncertainties.set_typeb_values_all(typeb_values);

			// add to the list
			type_b_uncertainties.push_back(typeBUncertainties);

		}

		tbu_file.close();
	}
	catch (const ios_base::failure& f) {
		if (tbu_file.eof())
		{
			tbu_file.close();
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
		if (tbu_file.eof())
		{
			tbu_file.close();
			return;
		}
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}	

	return;
}



} // dnaiostreams
} // dynadjust

