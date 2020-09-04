//============================================================================
// Name         : dnaiodna.cpp
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
// Description  : DNA file io helper
//============================================================================

#include <include/io/dnaiodna.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/measurement_types/dnameasurement_types.hpp>

using namespace dynadjust::measurements;
using namespace dynadjust::epsg;

namespace dynadjust { 
namespace iostreams {

void dna_io_dna::write_dna_files(vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, 
			const string& stnfilename, const string& msrfilename, 
			const project_settings& p, const CDnaDatum& datum, 
			const CDnaProjection& projection, bool flagUnused,
			const string& stn_comment, const string& msr_comment)
{

	write_stn_file(vStations, stnfilename, p, datum, projection, flagUnused, stn_comment);
	write_msr_file(vMeasurements, msrfilename, p, datum, msr_comment);
}

void dna_io_dna::write_dna_files(pvstn_t vbinary_stn, pvmsr_t vbinary_msr, 
			const string& stnfilename, const string& msrfilename, 
			const project_settings& p, const CDnaDatum& datum, 
			const CDnaProjection& projection, bool flagUnused,
			const string& stn_comment, const string& msr_comment)
{
	write_stn_file(vbinary_stn, stnfilename, p, datum, projection, flagUnused, stn_comment);
	write_msr_file(*vbinary_stn, vbinary_msr, msrfilename, p, datum, msr_comment);
}
	
void dna_io_dna::create_file_stn(std::ofstream* ptr, const string& filename)
{
	determineDNASTNFieldParameters<UINT16>("3.01", dsl_, dsw_);
	create_file_pointer(ptr, filename);
}
	
void dna_io_dna::create_file_msr(std::ofstream* ptr, const string& filename)
{
	determineDNAMSRFieldParameters<UINT16>("3.01", dml_, dmw_);
	create_file_pointer(ptr, filename);
}

void dna_io_dna::prepare_sort_list(const UINT32 count)
{
	vStationList_.resize(count);

	// initialise vector with 0,1,2,...,n-2,n-1,n
	initialiseIncrementingIntegerVector<UINT32>(vStationList_, count);
}
	
void dna_io_dna::create_file_pointer(std::ofstream* ptr, const string& filename)
{
	try {
		// Create file pointer to DNA file. 
		file_opener(*ptr, filename);
	}
	catch (const runtime_error& e) {
		throw boost::enable_current_exception(runtime_error(e.what()));
	}
}
	
void dna_io_dna::open_file_pointer(std::ifstream* ptr, const string& filename)
{
	try {
		// Open DNA file.
		file_opener(*ptr, filename, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		throw boost::enable_current_exception(runtime_error(e.what()));
	}
}

void dna_io_dna::write_stn_header(std::ofstream* ptr, vdnaStnPtr* vStations, 
			const project_settings& p, const CDnaDatum& datum, bool flagUnused,
			const string& comment)
{
	// print stations
	// Has the user specified --flag-unused-stations, in which case, do not
	// print stations marked unused?
	size_t count(0);
	if (flagUnused) 
	{
		for_each(vStations->begin(), vStations->end(),
			[&count](const dnaStnPtr& stn) {
				if (stn.get()->IsNotUnused())
					count++;
		});
	}
	else
		count = vStations->size();

	// Write version line
	dna_header(*ptr, "3.01", "STN", datum.GetName(), datum.GetEpoch_s(), count);

	// Write header comment line, e.g. "Coordinates exported from reftran."
	dna_comment(*ptr, comment);
}
	

void dna_io_dna::write_stn_header(std::ofstream* ptr, pvstn_t vbinary_stn, 
			const project_settings& p, const CDnaDatum& datum, bool flagUnused,
			const string& comment)
{
	// print stations
	// Has the user specified --flag-unused-stations, in which case, do not
	// print stations marked unused?
	size_t count(0);
	if (flagUnused) 
	{
		for_each(vbinary_stn->begin(), vbinary_stn->end(),
			[&count](const station_t& stn) {
				if (stn.unusedStation == 0)
					count++;
		});
	}
	else
		count = vbinary_stn->size();

	// Write version line
	dna_header(*ptr, "3.01", "STN", datum.GetName(), datum.GetEpoch_s(), count);

	// Write header comment line, e.g. "Coordinates exported from reftran."
	dna_comment(*ptr, comment);
}
	

void dna_io_dna::read_ren_file(const string& filename, pv_string_vstring_pair stnRenaming)
{
	std::ifstream renaming_file;

	// create file pointer
	open_file_pointer(&renaming_file, filename);

	string version;
	INPUT_DATA_TYPE idt;
	CDnaDatum datum;
	string geoidVersion, fileEpsg, fileEpoch;
	UINT32 count(0);

	// read header information
	read_dna_header(&renaming_file, version, idt, datum, false, false, fileEpsg, fileEpoch, geoidVersion, count);

	read_ren_data(&renaming_file, stnRenaming);

	renaming_file.close();
}

void dna_io_dna::read_ren_data(std::ifstream* ptr, pv_string_vstring_pair stnRenaming)
{
	string sBuf;

	string_vstring_pair stnNames;
	vstring altStnNames;

	while (!ptr->eof())			// while EOF not found
	{
		try {
			getline((*ptr), sBuf);
		}
		catch (...) {
			if (ptr->eof())
				return;
			stringstream ss;
			ss << "read_ren_data(): Could not read from the renaming file." << endl;
			boost::enable_current_exception(runtime_error(ss.str()));
		}

		// blank or whitespace?
		if (trimstr(sBuf).empty())			
			continue;

		// Ignore lines with blank station name
		if (trimstr(sBuf.substr(dsl_.stn_name, dsw_.stn_name)).empty())			
			continue;
		
		// Ignore lines with comments
		if (sBuf.compare(0, 1, "*") == 0)
			continue;
		
		string tmp;
		
		// Add the preferred name
		try {
			tmp = trimstr(sBuf.substr(dsl_.stn_name, dsw_.stn_name));
			stnNames.first = tmp;
		}
		catch (...) {
			stringstream ss;
			ss << "read_ren_data(): Could not extract station name from the record:  " << 
				endl << "    " << sBuf << endl;
			boost::enable_current_exception(runtime_error(ss.str()));
		}		

		// Alernative names
		altStnNames.clear();
		UINT32 positionEndName(dsw_.stn_name+dsw_.stn_name);
		UINT32 positionNextName(dsw_.stn_name);
		while (sBuf.length() > positionNextName)
		{
			if (sBuf.length() > positionEndName)
				tmp = trimstr(sBuf.substr(positionNextName, dsw_.stn_name));
			else
				tmp = trimstr(sBuf.substr(positionNextName));

			if (!tmp.empty())
				altStnNames.push_back(tmp);

			positionNextName = positionEndName;
			positionEndName += dsw_.stn_name;
		}
		
		// sort for faster searching
		sort(altStnNames.begin(), altStnNames.end());

		// Add the alternate names
		stnNames.second = altStnNames;

		// Okay, add the pair to the list
		stnRenaming->push_back(stnNames);
	}
}
	

void dna_io_dna::read_dna_header(std::ifstream* ptr, string& version, INPUT_DATA_TYPE& idt, 
	CDnaDatum& referenceframe, bool user_supplied_frame, bool override_input_frame,
	string& fileEpsg, string& fileEpoch, string& geoidversion, UINT32& count)
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

	// Attempt to get the file's type
	try {
		determineDNASTNFieldParameters<UINT16>(version, dsl_, dsw_);
		determineDNAMSRFieldParameters<UINT16>(version, dml_, dmw_);
	}
	catch (const runtime_error& e) {
		stringstream ssError;
		ssError << "- Error: Unable to determine DNA file version." << endl <<
			sBuf << endl << " " << e.what() << endl;
		throw boost::enable_current_exception(runtime_error(ssError.str()));
	}

	// Version 1
	if (iequals(version, "1.00"))
	{
		idt = unknown;							// could be stn or msr
		count = 100;							// set default 100 stations
		return;
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
	if (iequals(type, "stn"))
		idt = stn_data;
	// MEasurement file
	else if (iequals(type, "msr"))
		idt = msr_data;
	// Geoid information file
	else if (iequals(type, "geo"))
		idt = geo_data;
	// Station renaming
	else if (iequals(type, "ren"))
		idt = ren_data;
	else
		idt = unknown;

	if (sBuf.length() < 29)
	{
		count = 100;
		return;
	}

	string file_referenceframe;
	// Attempt to get the default reference frame
	try {
		if (sBuf.length() > 42)
			file_referenceframe = trimstr(sBuf.substr(29, 14));
		else if (sBuf.length() > 29)
			file_referenceframe = trimstr(sBuf.substr(29));
		else
			file_referenceframe = "";
	}
	catch (...) {
		// datum not provided?
		file_referenceframe = "";
	}

	string epoch_version;
	// Attempt to get the default epoch / geoid version
	try {
		if (sBuf.length() > 56)
			epoch_version = trimstr(sBuf.substr(43, 14));
		else if (sBuf.length() > 43)
			epoch_version = trimstr(sBuf.substr(43));
		else
			epoch_version = "";
	}
	catch (...) {
		// epoch / version not provided?
		epoch_version = "";
	}

	// Try to get the reference frame
	try {
		

		switch (idt)
		{
		case stn_data:
		case msr_data:

			// Capture file epsg
			if (file_referenceframe.empty())
				// Get the epsg code from the default datum 
				fileEpsg = referenceframe.GetEpsgCode_s();
			else
				fileEpsg = epsgStringFromName<string>(file_referenceframe);

			if (epoch_version.empty())
				// No, so get the epoch from the default datum 
				fileEpoch = referenceframe.GetEpoch_s();
			else
				fileEpoch = epoch_version;

			// Presently, the logic for handling default reference frame is duplicated for 
			// each file type, here and in dnaparser_pimpl.cxx:
			//     void referenceframe_pimpl::post_type(...)

			// 2. Does the user want to override the default datum?
			if (override_input_frame)
				// Do nothing, just return as referenceframe will become 
				// the default for all stations and measurements loaded
				// from the file.
				break;
			
			// 3. Does the user want the referenceframe attribute in the file to become the default?
			if (!user_supplied_frame)
			{
				if (!file_referenceframe.empty())
					// adopt the reference frame supplied in the file
					referenceframe.SetDatumFromName(file_referenceframe, epoch_version);
			}

			// Since import doesn't offer an option to capture epoch on the command line,
			// take the epoch from the file (if not empty).
			if (!epoch_version.empty())
				referenceframe.SetEpoch(epoch_version);
						
			break;

		case geo_data:
			geoidversion = epoch_version;
			break;
		default:
			break;
		}
	}
	catch (const runtime_error& e) {
		stringstream ssError;
		ssError << "The supplied frame is not recognised" << endl <<
			e.what() << endl;
		throw boost::enable_current_exception(runtime_error(ssError.str()));
	}	

	// Attempt to get the data count
	try {
		if (sBuf.length() > 66)
			count = val_uint<UINT32, string>(trimstr(sBuf.substr(57, 10)));
		else if (sBuf.length() > 57)
			count = val_uint<UINT32, string>(trimstr(sBuf.substr(57)));
		else 
			count = 100;
	}
	catch (...) {
		count = 100;
	}
}


void dna_io_dna::write_stn_file(pvstn_t vbinary_stn, const string& stnfilename,  
				const project_settings& p, const CDnaDatum& datum, 
				const CDnaProjection& projection, bool flagUnused,
				const string& comment)
{
	// Print DNA stations from a vector of stn_t
	std::ofstream dna_stn_file;

	try {
		// Create file pointer
		create_file_stn(&dna_stn_file, stnfilename);

		// Write header and comment
		write_stn_header(&dna_stn_file, vbinary_stn, p, datum, flagUnused, comment);

		// Sort on original file order
		prepare_sort_list(static_cast<UINT32>(vbinary_stn->size()));
		CompareStnFileOrder<station_t, UINT32> stnorderCompareFunc(vbinary_stn);
		sort(vStationList_.begin(), vStationList_.end(), stnorderCompareFunc);

		dnaStnPtr stnPtr(new CDnaStation(datum.GetName(), datum.GetEpoch_s()));

		// print stations
		// Has the user specified --flag-unused-stations, in wich case, do not
		// print stations marked unused?
		if (flagUnused) 
		{
			// Print stations in DNA format
			for_each(vStationList_.begin(), vStationList_.end(),
				[&dna_stn_file, &stnPtr, &projection, &datum, &vbinary_stn, this](const UINT32& stn) {
					if (stnPtr->IsNotUnused())
					{
						stnPtr->SetStationRec(vbinary_stn->at(stn));
						stnPtr->WriteDNAXMLStnCurrentEstimates(&dna_stn_file, datum.GetEllipsoidRef(), 
							const_cast<CDnaProjection*>(&projection), dna, &dsw_);
					}
			});
		}
		else
		{
			// Print stations in DNA format
			for_each(vStationList_.begin(), vStationList_.end(),
				[&dna_stn_file, &stnPtr, &projection, &datum, &vbinary_stn, this](const UINT32& stn) {
					stnPtr->SetStationRec(vbinary_stn->at(stn));
					stnPtr->WriteDNAXMLStnCurrentEstimates(&dna_stn_file, datum.GetEllipsoidRef(), 
						const_cast<CDnaProjection*>(&projection), dna, &dsw_);
			});
		}

		dna_stn_file.close();
		
	}
	catch (const std::ifstream::failure& f) {
		throw boost::enable_current_exception(runtime_error(f.what()));
	}
}
	

void dna_io_dna::write_stn_file(vdnaStnPtr* vStations, const string& stnfilename,  
			const project_settings& p, const CDnaDatum& datum, 
			const CDnaProjection& projection, bool flagUnused,
			const string& comment)
{
	// Print DNA stations from a vector of dnaStnPtr
	std::ofstream dna_stn_file;

	try {
		// Create file pointer
		create_file_stn(&dna_stn_file, stnfilename);

		// Write header and comment
		write_stn_header(&dna_stn_file, vStations, p, datum, flagUnused, comment);

		// Sort on original file order
		sort(vStations->begin(), vStations->end(), CompareStnFileOrder_CDnaStn<CDnaStation>());

		// print stations
		// Has the user specified --flag-unused-stations, in wich case, do not
		// print stations marked unused?
		if (flagUnused) 
		{
			// Print stations in DNA format
			for_each(vStations->begin(), vStations->end(),
				[&dna_stn_file, &projection, &datum, this](const dnaStnPtr& stn) {
					if (stn.get()->IsNotUnused())
						stn.get()->WriteDNAXMLStnCurrentEstimates(&dna_stn_file, datum.GetEllipsoidRef(), 
							const_cast<CDnaProjection*>(&projection), dna, &dsw_);
			});
		}
		else
		{
			// Print stations in DNA format
			for_each(vStations->begin(), vStations->end(),
				[&dna_stn_file, &projection, &datum, this](const dnaStnPtr& stn) {
					stn.get()->WriteDNAXMLStnCurrentEstimates(&dna_stn_file, datum.GetEllipsoidRef(), 
						const_cast<CDnaProjection*>(&projection), dna, &dsw_);
			});
		}

		dna_stn_file.close();
		
	}
	catch (const std::ifstream::failure& f) {
		throw boost::enable_current_exception(runtime_error(f.what()));
	}	
}

void dna_io_dna::write_msr_header(std::ofstream* ptr, vdnaMsrPtr* vMeasurements, 
	const project_settings& p, const CDnaDatum& datum, bool flagUnused,
	const string& comment)
{
	// Write version line
	dna_header(*ptr, "3.01", "MSR", datum.GetName(), datum.GetEpoch_s(), vMeasurements->size());

	// Write header comment line
	dna_comment(*ptr, comment);

}
	

void dna_io_dna::write_msr_header(std::ofstream* ptr, pvmsr_t vbinary_msrn, 
	const project_settings& p, const CDnaDatum& datum, bool flagUnused,
	const string& comment)
{
	// Write version line
	dna_header(*ptr, "3.01", "MSR", datum.GetName(), datum.GetEpoch_s(), vbinary_msrn->size());

	// Write header comment line
	dna_comment(*ptr, comment);
}


void dna_io_dna::write_msr_file(const vstn_t& vbinary_stn, pvmsr_t vbinary_msr, const string& msrfilename, 
	const project_settings& p, const CDnaDatum& datum,
	const string& comment)
{
	// Print DNA measurements	

	std::ofstream dna_msr_file;

	it_vmsr_t _it_msr(vbinary_msr->begin());
	dnaMsrPtr msrPtr;

	try {
		// Create file pointer
		create_file_msr(&dna_msr_file, msrfilename);

		// Write header and comment
		write_msr_header(&dna_msr_file, vbinary_msr, p, datum, true, comment);

		// print measurements
		for (_it_msr=vbinary_msr->begin(); _it_msr!=vbinary_msr->end(); ++_it_msr)
		{
			ResetMeasurementPtr<char>(&msrPtr, _it_msr->measType);
			msrPtr->SetMeasurementRec(vbinary_stn, _it_msr);
			msrPtr->WriteDNAMsr(&dna_msr_file, dmw_, dml_);
		}

		dna_msr_file.close();

	}
	catch (const std::ifstream::failure& f) {
		throw boost::enable_current_exception(runtime_error(f.what()));
	}
}
	
void dna_io_dna::write_msr_file(vdnaMsrPtr* vMeasurements, const string& msrfilename, 
	const project_settings& p, const CDnaDatum& datum,
	const string& comment)
{
	// Print DNA measurements	
	
	std::ofstream dna_msr_file;
	
	_it_vdnamsrptr _it_msr(vMeasurements->begin());
	
	try {
		// Create file pointer
		create_file_msr(&dna_msr_file, msrfilename);

		// Write header and comment
		write_msr_header(&dna_msr_file, vMeasurements, p, datum, true, comment);

		// print measurements
		for (_it_msr=vMeasurements->begin(); _it_msr!=vMeasurements->end(); _it_msr++)
			_it_msr->get()->WriteDNAMsr(&dna_msr_file, dmw_, dml_);
		
		dna_msr_file.close();
		
	}
	catch (const std::ifstream::failure& f) {
		throw boost::enable_current_exception(runtime_error(f.what()));
	}
	

}


} // dnaiostreams
} // dynadjust

