//============================================================================
// Name         : dnaiobst.cpp
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
// Description  : DynAdjust binary station file io operations
//============================================================================

#include <include/io/dnaiobst.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust { 
namespace iostreams {

UINT16 dna_io_bst::create_stn_input_file_meta(vifm_t& vinput_file_meta, input_file_meta_t** input_file_meta)
{
	UINT16 stn_file_count(0);
	stringstream ss;
	ss << "create_stn_input_file_meta(): An error was encountered when creating " << endl <<
		"  the binary station file metadata." << endl;

	// Determine how many station files were supplied
	try {
		for_each(vinput_file_meta.begin(), vinput_file_meta.end(),
			[&stn_file_count] (input_file_meta_t& ifm) { 
				if (ifm.datatype == stn_data || ifm.datatype == stn_msr_data)
					stn_file_count++;
		});

		if (*input_file_meta != NULL)
			delete [] *input_file_meta;
	
		(*input_file_meta) = new input_file_meta_t[stn_file_count];

		stn_file_count = 0;
		for_each(vinput_file_meta.begin(), vinput_file_meta.end(),
			[&stn_file_count, &input_file_meta] (input_file_meta_t& ifm) { 
				if (ifm.datatype == stn_data || ifm.datatype == stn_msr_data)
				{
					(*input_file_meta)[stn_file_count] = ifm;
					stn_file_count++;
				}
		});
	
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	return stn_file_count;
}

void dna_io_bst::load_bst_file_meta(const string& bst_filename, binary_file_meta_t& bst_meta) 
{
	std::ifstream bst_file;
	stringstream ss;
	ss << "load_bst_file(): An error was encountered when opening " << bst_filename << "." << endl;

	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(bst_file, bst_filename, ios::in | ios::binary, binary, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "load_bst_file(): An error was encountered when reading from " << bst_filename << "." << endl;

	try {
		// read the file information
		readFileInfo(bst_file);
		
		// read the metadata
		readFileMetadata(bst_file, bst_meta);
	}
	catch (const ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	bst_file.close();
}
	

UINT32 dna_io_bst::load_bst_file(const string& bst_filename, pvstn_t vbinary_stn, binary_file_meta_t& bst_meta) 
{	
	std::ifstream bst_file;
	stringstream ss;
	ss << "load_bst_file(): An error was encountered when opening " << bst_filename << "." << endl;

	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(bst_file, bst_filename, ios::in | ios::binary, binary, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	station_t stationRecord;
	UINT32 stn;

	ss.str("");
	ss << "load_bst_file(): An error was encountered when reading from " << bst_filename << "." << endl;

	try {
		// read the file information
		readFileInfo(bst_file);
		
		// read the metadata
		readFileMetadata(bst_file, bst_meta);
		
		// read the bms data
		vbinary_stn->reserve(bst_meta.binCount);
		for (stn=0; stn<bst_meta.binCount; stn++)
		{
			bst_file.read(reinterpret_cast<char *>(&stationRecord), sizeof(station_t));
			vbinary_stn->push_back(stationRecord);
		}
	}
	catch (const ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	bst_file.close();

	return bst_meta.binCount;
}

void dna_io_bst::write_bst_file(const string& bst_filename, pvstn_t vbinary_stn, binary_file_meta_t& bst_meta)
{
	std::ofstream bst_file;
	stringstream ss;
	ss << "write_bst_file(): An error was encountered when opening " << bst_filename << "." << endl;

	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(bst_file, bst_filename,
			ios::out | ios::binary, binary);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "write_bst_file(): An error was encountered when writing to " << bst_filename << "." << endl;

	try {
		// write version
		writeFileInfo(bst_file);
		
		// write the metadata
		writeFileMetadata(bst_file, bst_meta);
		
		// write the bst data
		it_vstn_t _it_stn(vbinary_stn->begin());		
		for (; _it_stn!=vbinary_stn->end(); ++_it_stn)
			bst_file.write(reinterpret_cast<char *>(&(*_it_stn)), sizeof(station_t));
	}
	catch (const ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	bst_file.close();
}

bool dna_io_bst::write_bst_file(const string& bst_filename, vdnaStnPtr* vStations, pvstring vUnusedStns, binary_file_meta_t& bst_meta, bool flagUnused)
{
	std::ofstream bst_file;
	stringstream ss;
	ss << "write_bst_file(): An error was encountered when opening " << bst_filename << "." << endl;

	try {
		// open binary stations file.  Throws runtime_error on failure.
		file_opener(bst_file, bst_filename,
			ios::out | ios::binary, binary);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "write_bst_file(): An error was encountered when writing to " << bst_filename << "." << endl;

	_it_vdnastnptr _it_stn;
	
	try {
		// write version
		writeFileInfo(bst_file);		
		
		// write the metadata
		writeFileMetadata(bst_file, bst_meta);
		
		// write the bst data
		if (flagUnused)
		{
			sort(vUnusedStns->begin(), vUnusedStns->end());
			for (_it_stn=vStations->begin(); _it_stn!=vStations->end(); _it_stn++)
			{
				if (binary_search(vUnusedStns->begin(), vUnusedStns->end(), _it_stn->get()->GetName()))
				{
					_it_stn->get()->SetStationUnused();
					_it_stn->get()->WriteBinaryStn(&bst_file, TRUE);
				}
				else
					_it_stn->get()->WriteBinaryStn(&bst_file);
			}
		}
		else
			for (_it_stn=vStations->begin(); _it_stn!=vStations->end(); _it_stn++)
				_it_stn->get()->WriteBinaryStn(&bst_file);
	}
	catch (const ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	bst_file.close();
	return true;
}


} // dnaiostreams
} // dynadjust

