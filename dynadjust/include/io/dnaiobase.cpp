//============================================================================
// Name         : dnaiobase.cpp
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
// Description  : DynAdjust file io operations
//============================================================================

#include <include/io/dnaiobase.hpp>
#include <include/config/dnaversion.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace dynadjust {
namespace iostreams {

dna_io_base::dna_io_base(void)
	: m_strVersion(__FILE_VERSION__)
	, m_strDate("")
	, m_strApp("DNA" + std::string(__SHORT_VERSION__))
{
}

// copy constructors
dna_io_base::dna_io_base(const dna_io_base& newFile)
	: m_strVersion(newFile.m_strVersion)
	, m_strDate(newFile.m_strDate)
	, m_strApp(newFile.m_strApp)
{
}
	

dna_io_base::~dna_io_base(void)
{
}
	

dna_io_base& dna_io_base::operator= (const dna_io_base& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	m_strVersion = rhs.m_strVersion;
	m_strDate = rhs.m_strDate;
	m_strApp = rhs.m_strApp;

	return *this;
}

void dna_io_base::writeFileInfo(std::ofstream& file_stream)
{
	writeVersion(file_stream);
	writeDate(file_stream);
	writeApp(file_stream);
}
	

void dna_io_base::readFileInfo(std::ifstream& file_stream)
{
	readVersion(file_stream);
	readDate(file_stream);
	readApp(file_stream);
}
	

void dna_io_base::writeFileMetadata(std::ofstream& file_stream, binary_file_meta_t& file_meta)
{
	// Write the metadata
	file_stream.write(reinterpret_cast<char *>(&file_meta.binCount), sizeof(UINT32)); 
	file_stream.write(reinterpret_cast<char *>(&file_meta.reduced), sizeof(bool)); 
	file_stream.write(reinterpret_cast<char *>(file_meta.modifiedBy), MOD_NAME_WIDTH); 
	
	// Write the epsg code and epoch
	file_stream.write(reinterpret_cast<char *>(file_meta.epsgCode), STN_EPSG_WIDTH);
	file_stream.write(reinterpret_cast<char *>(file_meta.epoch), STN_EPOCH_WIDTH);

	file_stream.write(reinterpret_cast<char*>(&file_meta.reftran), sizeof(bool));
	file_stream.write(reinterpret_cast<char*>(&file_meta.geoid), sizeof(bool));

	// Write file count and file meta
	file_stream.write(reinterpret_cast<char *>(&file_meta.inputFileCount), sizeof(UINT16)); 
	for (UINT16 i(0); i<file_meta.inputFileCount; ++i)
	{
		file_stream.write(reinterpret_cast<char *>(file_meta.inputFileMeta[i].filename), FILE_NAME_WIDTH); 
		file_stream.write(reinterpret_cast<char *>(&file_meta.inputFileMeta[i].filetype), sizeof(UINT16)); 
		file_stream.write(reinterpret_cast<char *>(&file_meta.inputFileMeta[i].datatype), sizeof(UINT16)); 
	}
}
	

void dna_io_base::readFileMetadata(std::ifstream& file_stream, binary_file_meta_t& file_meta)
{
	// Read the metadata
	file_stream.read(reinterpret_cast<char *>(&file_meta.binCount), sizeof(UINT32)); 
	file_stream.read(reinterpret_cast<char *>(&file_meta.reduced), sizeof(bool)); 
	file_stream.read(reinterpret_cast<char *>(file_meta.modifiedBy), MOD_NAME_WIDTH);

	// Read the epsg code and epoch
	file_stream.read(reinterpret_cast<char *>(file_meta.epsgCode), STN_EPSG_WIDTH);
	file_stream.read(reinterpret_cast<char *>(file_meta.epoch), STN_EPOCH_WIDTH);

	file_stream.read(reinterpret_cast<char*>(&file_meta.reftran), sizeof(bool));
	file_stream.read(reinterpret_cast<char*>(&file_meta.geoid), sizeof(bool));

	// Read file count and file meta
	file_stream.read(reinterpret_cast<char *>(&file_meta.inputFileCount), sizeof(UINT16));
	if (file_meta.inputFileMeta != NULL)
		delete []file_meta.inputFileMeta;

	file_meta.inputFileMeta = new input_file_meta_t[file_meta.inputFileCount];
		
	for (UINT16 i(0); i<file_meta.inputFileCount; ++i)
	{
		file_stream.read(reinterpret_cast<char *>(file_meta.inputFileMeta[i].filename), FILE_NAME_WIDTH); 
		file_stream.read(reinterpret_cast<char *>(&file_meta.inputFileMeta[i].filetype), sizeof(UINT16)); 
		file_stream.read(reinterpret_cast<char *>(&file_meta.inputFileMeta[i].datatype), sizeof(UINT16)); 
	}
}
	

void dna_io_base::writeVersion(std::ofstream& file_stream)
{
	char versionField[identifier_field_width+1];
	
	// write version field name
	versionField[identifier_field_width] = '\0';
	file_stream.write(const_cast<char *>(version_header), identifier_field_width); 
	
	// write version, with safeguard against overwriting by substr
	sprintf(versionField, "%*s", identifier_field_width, m_strVersion.substr(0, identifier_field_width).c_str());
	file_stream.write(reinterpret_cast<char *>(versionField), identifier_field_width); 
		
}
	

void dna_io_base::readVersion(std::ifstream& file_stream)
{
	char versionField[identifier_field_width+1];

	// read version field name
	versionField[identifier_field_width] = '\0';
	file_stream.read(reinterpret_cast<char *>(versionField), identifier_field_width);
	
	// read version
	file_stream.read(reinterpret_cast<char *>(versionField), identifier_field_width); 
	m_strVersion = versionField;
	m_strVersion = trimstr(m_strVersion);
}
	

void dna_io_base::writeDate(std::ofstream& file_stream)
{
	char dateField[identifier_field_width+1];

	// Form date string
	boost::gregorian::date today(boost::gregorian::day_clock::local_day());
	std::stringstream date_string;
	date_string << std::right << to_iso_extended_string(today);
	m_strDate = date_string.str();
	
	// write creation date field name
	dateField[identifier_field_width] = '\0';
	file_stream.write(const_cast<char *>(create_date_header), identifier_field_width); 
	
	// write creation date, with safeguard against overwriting by substr
	sprintf(dateField, "%*s", identifier_field_width, m_strDate.substr(0, identifier_field_width).c_str());
	file_stream.write(reinterpret_cast<char *>(dateField), identifier_field_width); 
		
}
	

void dna_io_base::readDate(std::ifstream& file_stream)
{
	char dateField[identifier_field_width+1];

	// read creation date field name
	dateField[identifier_field_width] = '\0';
	file_stream.read(reinterpret_cast<char *>(dateField), identifier_field_width);
	
	// read creation date
	file_stream.read(reinterpret_cast<char *>(dateField), identifier_field_width); 
	m_strDate = dateField;
	m_strDate = trimstr(m_strDate);
}
	

void dna_io_base::writeApp(std::ofstream& file_stream)
{
	char appField[identifier_field_width+1];
	
	// write application field name
	appField[identifier_field_width] = '\0';
	file_stream.write(const_cast<char *>(create_by_header), identifier_field_width); 
	
	// write application, with safeguard against overwriting by substr
	sprintf(appField, "%*s", identifier_field_width, m_strApp.substr(0, identifier_field_width).c_str());
	file_stream.write(reinterpret_cast<char *>(appField), identifier_field_width); 
		
}
	

void dna_io_base::readApp(std::ifstream& file_stream)
{
	char appField[identifier_field_width+1];

	// read application field name
	appField[identifier_field_width] = '\0';
	file_stream.read(reinterpret_cast<char *>(appField), identifier_field_width);
	
	// read application
	file_stream.read(reinterpret_cast<char *>(appField), identifier_field_width); 
	m_strApp = appField;
	m_strApp = trimstr(m_strApp);
}

}	// namespace measurements
}	// namespace dynadjust
