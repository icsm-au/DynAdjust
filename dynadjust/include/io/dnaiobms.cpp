//============================================================================
// Name         : dnaiobms.cpp
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
// Description  : DynAdjust binary measurement file io operations
//============================================================================

#include <include/io/dnaiobms.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>

namespace dynadjust { 
namespace iostreams {

UINT16 dna_io_bms::create_msr_input_file_meta(vifm_t& vinput_file_meta, input_file_meta_t** input_file_meta)
{
	UINT16 msr_file_count(0);
	std::stringstream ss;
	ss << "create_msr_input_file_meta(): An error was encountered when creating " << std::endl <<
		"  the binary measurement file metadata." << std::endl;

	// Determine how many measurement files were supplied
	try {
		for_each(vinput_file_meta.begin(), vinput_file_meta.end(),
			[&msr_file_count] (input_file_meta_t& ifm) { 
				if (ifm.datatype == msr_data || ifm.datatype == stn_msr_data)
					msr_file_count++;
		});

		if (*input_file_meta != NULL)
			delete [] *input_file_meta;
	
		(*input_file_meta) = new input_file_meta_t[msr_file_count];

		msr_file_count = 0;
		for_each(vinput_file_meta.begin(), vinput_file_meta.end(),
			[&msr_file_count, &input_file_meta] (input_file_meta_t& ifm) { 
				if (ifm.datatype == msr_data || ifm.datatype == stn_msr_data)
				{
					(*input_file_meta)[msr_file_count] = ifm;
					msr_file_count++;
				}
		});
	
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}

	return msr_file_count;
}

void dna_io_bms::load_bms_file_meta(const std::string& bms_filename, binary_file_meta_t& bms_meta) 
{
	std::ifstream bms_file;
	std::stringstream ss;
	ss << "load_bms_file(): An error was encountered when opening " << bms_filename << "." << std::endl;

	try {
		// open binary measurements file.  Throws runtime_error on failure.
		file_opener(bms_file, bms_filename, std::ios::in | std::ios::binary, binary, true);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_bms_file(): An error was encountered when reading from " << bms_filename << "." << std::endl;

	try {
		// read the file information
		readFileInfo(bms_file);
		
		// read the metadata
		readFileMetadata(bms_file, bms_meta);
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
	
	bms_file.close();
}
	
UINT32 dna_io_bms::load_bms_file(const std::string& bms_filename, pvmsr_t vbinary_msr, binary_file_meta_t& bms_meta) 
{	
	std::ifstream bms_file;
	std::stringstream ss;
	ss << "load_bms_file(): An error was encountered when opening " << bms_filename << "." << std::endl;

	try {
		// open binary measurements file.  Throws runtime_error on failure.
		file_opener(bms_file, bms_filename, std::ios::in | std::ios::binary, binary, true);
	}
	catch (const std::runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(std::runtime_error(ss.str()));
	}
	
	measurement_t measRecord;
	UINT32 msr;

	ss.str("");
	ss << "load_bms_file(): An error was encountered when reading from " << bms_filename << "." << std::endl;

	try {
		// read the file information 
		readFileInfo(bms_file);
		
		// read the metadata
		readFileMetadata(bms_file, bms_meta);

		// read the bms data
		vbinary_msr->reserve(bms_meta.binCount);
		for (msr=0; msr<bms_meta.binCount; msr++)
		{
			bms_file.read(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
			vbinary_msr->push_back(measRecord);
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
	
	bms_file.close();

	return bms_meta.binCount;
}

void dna_io_bms::write_bms_file(const std::string& bms_filename, pvmsr_t vbinary_msr, binary_file_meta_t& bms_meta)
{
	std::ofstream bms_file;
	std::stringstream ss;
	ss << "write_bms_file(): An error was encountered when opening " << bms_filename << "." << std::endl;

	try {
		// open binary measurements file.  Throws runtime_error on failure.
		file_opener(bms_file, bms_filename,
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
	ss << "write_bms_file(): An error was encountered when writing to " << bms_filename << "." << std::endl;

	try {
		// write the file information
		writeFileInfo(bms_file);
		
		// write the metadata
		writeFileMetadata(bms_file, bms_meta);
		
		// write the bms data
		it_vmsr_t _it_msr(vbinary_msr->begin());		
		for (_it_msr=vbinary_msr->begin(); _it_msr!=vbinary_msr->end(); ++_it_msr)
			bms_file.write(reinterpret_cast<char *>(&(*_it_msr)), sizeof(measurement_t));
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
	
	bms_file.close();
}

void dna_io_bms::write_bms_file(const std::string& bms_filename, vdnaMsrPtr* vMeasurements, binary_file_meta_t& bms_meta)
{
	std::ofstream bms_file;
	std::stringstream ss;
	ss << "write_bms_file(): An error was encountered when opening " << bms_filename << "." << std::endl;

	try {
		// open binary measurements file.  Throws runtime_error on failure.
		file_opener(bms_file, bms_filename,
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
	ss << "write_bms_file(): An error was encountered when writing to " << bms_filename << "." << std::endl;

	_it_vdnamsrptr _it_msr;
	UINT32 msrIndex(0);

	try {
		// write the file information
		writeFileInfo(bms_file);
		
		// write the metadata
		writeFileMetadata(bms_file, bms_meta);
		
		// write the bms data
		for (_it_msr=vMeasurements->begin(); _it_msr!=vMeasurements->end(); ++_it_msr)
			_it_msr->get()->WriteBinaryMsr(&bms_file, &msrIndex);
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
	
	bms_file.close();
}

} // dnaiostreams
} // dynadjust
