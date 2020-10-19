//============================================================================
// Name         : dnaioasl.cpp
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
// Description  : DynAdjust associated station file io operations
//============================================================================

#include <include/io/dnaioasl.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnatemplatestnmsrfuncs.hpp>
#include <include/functions/dnatemplatefuncs.hpp>

namespace dynadjust { 
namespace iostreams {

// open the asl file. the asl file is required to ensure invalid stations are
// excluded from the adjustment. invalid stations are those with no measurements.
UINT32 dna_io_asl::load_asl_file(const string& asl_filename, vASL* vbinary_asl, vUINT32* vfree_stn)
{	
	std::ifstream asl_file;
	stringstream ss;
	ss << "load_asl_file(): An error was encountered when opening " << asl_filename << "." << endl;

	try {
		// open stations asl file.  Throws runtime_error on failure.
		file_opener(asl_file, asl_filename, ios::in | ios::binary, binary, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_asl_file(): An error was encountered when reading from " << asl_filename << "." << endl;

	UINT32 stnCount;
	
	try {
		// read the file information
		readFileInfo(asl_file);
		
		// Get number of records and resize AML vector
		asl_file.read(reinterpret_cast<char *>(&stnCount), sizeof(UINT32));
		
		// initialise free station list
		initialiseIncrementingIntegerVector(vfree_stn, stnCount);
		
		// initialise assocaiated station list
		vbinary_asl->clear();
		vbinary_asl->resize(stnCount);
		
		_it_vasl _it_asl;

		for (_it_asl=vbinary_asl->begin();
			_it_asl!=vbinary_asl->end();
			++_it_asl)
		{
			asl_file >> &(*_it_asl);
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

	asl_file.close();

	return stnCount;
}

void dna_io_asl::write_asl_file(const string& asl_filename, pvASLPtr vbinary_asl) 
{	
	std::ofstream asl_file;
	stringstream ss;
	ss << "write_asl_file(): An error was encountered when opening " << asl_filename << "." << endl;

	try {
		// create binary asl file.  Throws runtime_error on failure.
		file_opener(asl_file, asl_filename,
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
	ss << "write_asl_file(): An error was encountered when writing to " << asl_filename << "." << endl;

	// Calculate number of station records and write to binary file
	UINT32 aslCount = static_cast<UINT32>(vbinary_asl->size());
	vASLPtr::const_iterator _it_asl(vbinary_asl->begin());
	
	try {
		// write the file information 
		writeFileInfo(asl_file);
		
		// write the data
		asl_file.write(reinterpret_cast<char *>(&aslCount), sizeof(UINT32));
		for (_it_asl=vbinary_asl->begin(); _it_asl!=vbinary_asl->end(); ++_it_asl)
			if (_it_asl->get())
				asl_file << _it_asl->get();			// see dnastation.hpp (region CAStationList stream handlers)
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
	
	asl_file.close();
}

// prints ASL entries sorted according to measurement count, which is useful when determining 
// the starting station to begin network segmentation.
void dna_io_asl::write_asl_file_txt(const string& asl_filename, pvASLPtr vbinary_asl, vdnaStnPtr* vStations) 
{	
	std::ofstream asl_file;
	stringstream ss;
	ss << "write_asl_file_txt(): An error was encountered when opening " << asl_filename << "." << endl;

	try {
		// create binary asl file.  Throws runtime_error on failure.
		file_opener(asl_file, asl_filename);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "write_asl_file_txt(): An error was encountered when writing to " << asl_filename << "." << endl;

	// Calculate number of station records and write to binary file
	size_t aslCount = vbinary_asl->size();
	
	// Pump to text file
	stringstream ss_asl;
	ss_asl << aslCount << " stations";
	asl_file << left << setw(STATION) << ss_asl.str();
	asl_file << setw(HEADER_20) << right << "No. connected msrs";
	asl_file << setw(STATION) << right << "AML index";
	asl_file << setw(STATION) << right << "Unused?" << endl;

	// Create an incrementing list and sort on number of measurements to each station
	vUINT32 aslPtrs(vbinary_asl->size());
	initialiseIncrementingIntegerVector<UINT32>(aslPtrs, static_cast<UINT32>(vbinary_asl->size()));

	// Sort on measurement count
	CompareMeasCount2<ASLPtr, UINT32> msrcountCompareFunc(vbinary_asl);
	sort(aslPtrs.begin(), aslPtrs.end(), msrcountCompareFunc);

	vbinary_asl->at(0).get()->GetAMLStnIndex();

	try {

		it_vUINT32 _it_asl;
		vASLPtr::iterator _it_pasl;
		
		for (_it_asl=aslPtrs.begin(); _it_asl!=aslPtrs.end(); ++_it_asl)
		{
			if (!vbinary_asl->at(*_it_asl))	// unused station
				continue;

			_it_pasl = vbinary_asl->begin() + *_it_asl;
			
			// Name and measurement count
			asl_file << setw(STATION) << left << 
				vStations->at(*_it_asl)->GetName() << 
				setw(HEADER_20) << right << _it_pasl->get()->GetAssocMsrCount();
			// Aml station index
			if (_it_pasl->get()->GetAssocMsrCount() == 0)
				asl_file << setw(STATION) << "-";
			else
				asl_file << setw(STATION) << right << _it_pasl->get()->GetAMLStnIndex();
			// Valid station?
			asl_file << setw(STATION) << right << (_it_pasl->get()->IsValid() ? " " : "*") << endl;
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
	
	asl_file.close();
}

} // dnaiostreams
} // dynadjust
