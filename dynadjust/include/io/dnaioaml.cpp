//============================================================================
// Name         : dnaioaml.cpp
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
// Description  : DynAdjust associated measurement file io operations
//============================================================================

#include <include/io/dnaioaml.hpp>
#include <include/functions/dnatemplatestnmsrfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust { 
namespace iostreams {

void dna_io_aml::load_aml_file(const std::string& aml_filename, v_aml_pair* vbinary_aml, pvmsr_t bmsRecords)
{	
	std::ifstream aml_file;
	std::stringstream ss;
	ss << "load_aml_file(): An error was encountered when opening " << aml_filename << "." << std::endl;

	try {
		// open stations aml file.  Throws runtime_error on failure.
		file_opener(aml_file, aml_filename, std::ios::in | std::ios::binary, binary, true);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_aml_file(): An error was encountered when reading from " << aml_filename << "." << std::endl;

	UINT32 msrValue;
	
	try {
		// read the file information
		readFileInfo(aml_file);
		
		// Get number of records and resize AML vector
		aml_file.read(reinterpret_cast<char *>(&msrValue), sizeof(UINT32));
		
		vbinary_aml->clear();
		vbinary_aml->resize(msrValue);

		it_aml_pair _it_aml;

		for (_it_aml = vbinary_aml->begin(); 
			_it_aml != vbinary_aml->end();
			++_it_aml)
		{
			aml_file.read(reinterpret_cast<char *>(&msrValue), sizeof(UINT32));
			// Ignored measurements are retained in the AML, so consume them!
			_it_aml->bmsr_index = msrValue;
			if (bmsRecords->at(msrValue).ignore)
				_it_aml->consume();
		}

	}
	catch (const std::ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}

	aml_file.close();
}

void dna_io_aml::write_aml_file(const std::string& aml_filename, pvUINT32 vbinary_aml) 
{	
	std::ofstream aml_file;
	std::stringstream ss;
	ss << "write_aml_file(): An error was encountered when opening " << aml_filename << "." << std::endl;

	try {
		// create binary aml file.  Throws runtime_error on failure.
		file_opener(aml_file, aml_filename,
			std::ios::out | std::ios::binary, binary);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "write_aml_file(): An error was encountered when writing to " << aml_filename << "." << std::endl;

	// Calculate number of measurement records and write to binary file
	UINT32 amlCount(static_cast<UINT32>(vbinary_aml->size()));
	it_vUINT32 _it_aml;
	
	try {
		// write the file information
		writeFileInfo(aml_file);
		
		// write the data
		aml_file.write(reinterpret_cast<char *>(&amlCount), sizeof(UINT32));
		for (_it_aml = vbinary_aml->begin(); 
			_it_aml != vbinary_aml->end();
			++_it_aml)
		{
			aml_file.write(reinterpret_cast<char *>(&(*_it_aml)), sizeof(UINT32));
		}
	}
	catch (const std::ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	aml_file.close();
}

void dna_io_aml::create_msr_to_stn_tally(const pvASLPtr vAssocStnList, v_aml_pair& vAssocMsrList, 
	vmsrtally& stnmsrTally, vmsr_t& bmsBinaryRecords)
{	
	stnmsrTally.clear();
	stnmsrTally.resize(vAssocStnList->size());

	vASLPtr::iterator _it_asl;
	UINT32 i, m, amlIndex, msrCount;

	for (_it_asl=vAssocStnList->begin(), i=0; _it_asl!=vAssocStnList->end(); ++_it_asl, ++i)
	{
		msrCount = _it_asl->get()->GetAssocMsrCount();

		for (m=0; m<msrCount; ++m)
		{
			amlIndex = _it_asl->get()->GetAMLStnIndex() + m;

			if (bmsBinaryRecords.at(vAssocMsrList.at(amlIndex).bmsr_index).ignore)
				continue;

			stnmsrTally.at(i).IncrementMsrType(bmsBinaryRecords.at(vAssocMsrList.at(amlIndex).bmsr_index).measType);
		}

		stnmsrTally.at(i).TotalCount();
	}
}
	
	
void dna_io_aml::write_msr_to_stn(std::ostream &os, pvstn_t bstBinaryRecords, 
	pvUINT32 vStationList, vmsrtally& v_stnmsrTally, MsrTally* parsemsrTally)
{
	// Print measurement to station summary header
	std::string header("MEASUREMENT TO STATIONS ");
	MsrToStnSummaryHeader(os, header);

	it_vUINT32 _it_stn(vStationList->begin());

	UINT32 msrRedundancies(0);
	vstring stationRedundantMsrs;

	// Print measurements to each station and the total count for each station
	for (_it_stn=vStationList->begin(); _it_stn != vStationList->end(); ++_it_stn)
	{
		v_stnmsrTally.at(*_it_stn).coutSummaryMsrToStn(os, bstBinaryRecords->at(*_it_stn).stationName);

		// Test for possible redundancies
		if ((v_stnmsrTally.at(*_it_stn).MeasurementCount('G') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('X') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('Y')) 
			&&
			(v_stnmsrTally.at(*_it_stn).MeasurementCount('P') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('Q') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('R') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('H') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('I') ||
			v_stnmsrTally.at(*_it_stn).MeasurementCount('J')))
		{
			msrRedundancies++;
			stationRedundantMsrs.push_back(bstBinaryRecords->at(*_it_stn).stationName);
		}
	}
	
	// Print "the bottom line"
	MsrToStnSummaryHeaderLine(os);
	
	// Print the total count per measurement	
	parsemsrTally->coutSummaryMsrToStn(os, "Totals");

	os << std::endl << std::endl;

	if (msrRedundancies > 0)
	{
		os << "WARNING: " << msrRedundancies;
		if (msrRedundancies == 1)
			os << " station was";
		else
			os << " stations were";

		os << " found to have GNSS measurements and absolute terrestrial measurements:" << std::endl << std::endl;
		
		// Print header
		os << std::setw(STATION) << std::left << "Station" << std::setw(30) << "Measurement types" << "Count" << std::endl;
		os << "------------------------------------------------------------" << std::endl;

		// Print measurements to each station and the total count for each station
		for (_it_stn=vStationList->begin(); _it_stn != vStationList->end(); ++_it_stn)
		{
			// Test for possible redundancies
			if ((v_stnmsrTally.at(*_it_stn).MeasurementCount('G') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('X') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('Y')) 
				&&
				(v_stnmsrTally.at(*_it_stn).MeasurementCount('P') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('Q') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('R') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('H') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('I') ||
				v_stnmsrTally.at(*_it_stn).MeasurementCount('J')))
			{
				v_stnmsrTally.at(*_it_stn).coutSummaryMsrToStn_Compressed(os, bstBinaryRecords->at(*_it_stn).stationName);
			}
		}	
	}
}
	

void dna_io_aml::write_aml_file_txt(const std::string& bms_filename, const std::string& aml_filename, pvUINT32 vbinary_aml, const pvASLPtr vAssocStnList, vdnaStnPtr* vStations)
{	
	vmsr_t binaryMsrRecords;
	std::stringstream ss;
	ss << "write_aml_file(): An error was encountered when opening " << bms_filename << "." << std::endl;

	binary_file_meta_t bms_meta;
	try {
		dna_io_bms bms;
		// Load binary stations data.  Throws runtime_error on failure.
		bms.load_bms_file(bms_filename, &binaryMsrRecords, bms_meta);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}

	std::ofstream aml_file;
	ss.str("");
	ss << "write_aml_file(): An error was encountered when opening " << aml_filename << "." << std::endl;

	try {
		// create binary aml file.  Throws runtime_error on failure.
		file_opener(aml_file, aml_filename,
			std::ios::out | std::ios::binary, binary);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "write_aml_file(): An error was encountered when writing to " << aml_filename << "." << std::endl;

	// Write number of measurement records to file
	std::stringstream ss_aml;
	ss_aml << vbinary_aml->size() << " records";
	aml_file << std::left << std::setw(HEADER_18) << ss_aml.str();
	aml_file << std::left << std::setw(MSR) << "Msr index";
	aml_file << std::left << std::setw(MSR) << "Msr type";
	aml_file << std::left << std::setw(MSR) << "Cluster";
	aml_file << std::left << std::setw(MSR) << "Ignored msr?" << std::endl;

	it_vUINT32_const _it_aml(vbinary_aml->begin());
	
	UINT32 stn_index(0);

	UINT32 nextAMLStationIndex(vAssocStnList->at(stn_index)->GetAMLStnIndex() + vAssocStnList->at(stn_index)->GetAssocMsrCount());
	UINT32 currAMLStationIndex(0);
	vUINT32 msrIndices;

	it_vUINT32 _it_msr;
	
	try {
		do
		{
			if (stn_index >= vAssocStnList->size())
				break;

			if (_it_aml >= vbinary_aml->end())
				break;

			// Is the curent station not connected to any measurements?
			if (vAssocStnList->at(stn_index)->GetAssocMsrCount() == 0)
			{
				// Get the index of the next station in the AML
				++stn_index;
				nextAMLStationIndex = vAssocStnList->at(stn_index)->GetAMLStnIndex() + vAssocStnList->at(stn_index)->GetAssocMsrCount();
				continue;
			}

			// Has a new station index been reached?
			if (currAMLStationIndex == nextAMLStationIndex)
			{
				// Get the index of the next station in the AML
				++stn_index;
				nextAMLStationIndex = vAssocStnList->at(stn_index)->GetAMLStnIndex() + vAssocStnList->at(stn_index)->GetAssocMsrCount();
				continue;
			}

			// At this point, the current station is connected to measurements.
			// However, such measurements may be ignored, in which case a station
			// may be unused.

			// Print station name
			aml_file << std::setw(HEADER_18) << std::left << vStations->at(stn_index)->GetName();
			
			// print the bmsr index
			// For clusters, this will be the index of the first binary element 
			aml_file <<	std::setw(MSR) << std::left << *_it_aml;

			GetMsrIndices<UINT32>(binaryMsrRecords, *_it_aml, msrIndices);

			ss_aml.str("");
			ss_aml << binaryMsrRecords.at(*_it_aml).measType;

			for (_it_msr=msrIndices.begin(); _it_msr!=msrIndices.end(); ++_it_msr)
			{
				if (stn_index == binaryMsrRecords.at(*_it_msr).station1)
				{
					// All measurements have a first station
					ss_aml << std::left << " (First)";
					aml_file << std::setw(MSR) << std::left << ss_aml.str();
					ss_aml.str("");
					switch (binaryMsrRecords.at(*_it_msr).measType)
					{
					case 'D': // Direction set
					case 'X': // GPS Baseline cluster
					case 'Y': // GPS point cluster
						ss_aml << std::setw(MSR) << std::left << binaryMsrRecords.at(*_it_msr).clusterID;
						break;
					default:
						ss_aml << std::setw(MSR) << std::left << " ";
						break;
					}
					continue;
				}

				if (MsrTally::Stations(binaryMsrRecords.at(*_it_msr).measType) >= TWO_STATION)
				{
					// Measurements which have a second or third station
					switch (binaryMsrRecords.at(*_it_msr).measType)
					{
					case 'D': // Direction set
						if (stn_index == binaryMsrRecords.at(*_it_msr).station2)
						{
							if (binaryMsrRecords.at(*_it_msr).vectorCount1 > 0)
								ss_aml << std::left << " (Second)";
							else
								ss_aml << std::left << " (Target)";
						}
						aml_file << std::setw(MSR) << std::left << ss_aml.str();
						ss_aml.str("");
						ss_aml << std::setw(MSR) << std::left << binaryMsrRecords.at(*_it_msr).clusterID;
						break;
					case 'B': // Geodetic azimuth
					case 'C': // Chord dist
					case 'E': // Ellipsoid arc
					case 'G': // GPS Baseline (treat as single-baseline cluster)
					case 'K': // Astronomic azimuth
					case 'L': // Level difference
					case 'M': // MSL arc
					case 'S': // Slope distance
					case 'V': // Zenith distance
					case 'Z': // Vertical angle
						if (stn_index == binaryMsrRecords.at(*_it_msr).station2)
							ss_aml << std::left << " (Second)";
						aml_file << std::setw(MSR) << std::left << ss_aml.str();
						ss_aml.str("");
						ss_aml << std::setw(MSR) << std::left << " ";
						break;	
					case 'X': // GPS Baseline cluster
						if (stn_index == binaryMsrRecords.at(*_it_msr).station2)
							ss_aml << std::left << " (Second)";
						aml_file << std::setw(MSR) << std::left << ss_aml.str();
						ss_aml.str("");
						ss_aml << std::setw(MSR) << std::left << binaryMsrRecords.at(*_it_msr).clusterID;
						break;	
					case 'A': // Horizontal angle
						if (stn_index == binaryMsrRecords.at(*_it_msr).station2)
							ss_aml << std::left << " (Second)";
						else if (stn_index == binaryMsrRecords.at(*_it_msr).station3)
							ss_aml << std::left << " (Third)";
						aml_file << std::setw(MSR) << std::left << ss_aml.str();
						ss_aml.str("");
						ss_aml << std::setw(MSR) << std::left << " ";
						break;
					}
				}
			}			

			aml_file << std::setw(MSR) << std::left << ss_aml.str();

			if (binaryMsrRecords.at(*_it_aml).ignore)
				aml_file << std::setw(MSR) << std::left << "*" << std::endl;	// Ignored
			else
				aml_file << std::setw(MSR) << std::left << " " << std::endl;
			
			// Move to the next AML record
			if (++_it_aml == vbinary_aml->end())
				break;

			++currAMLStationIndex;

		// has the end of the AML been reached?
		} while (_it_aml != vbinary_aml->end());

	}
	catch (const std::ios_base::failure& f) {
		ss << f.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	aml_file.close();
}

} // dnaiostreams
} // dynadjust

