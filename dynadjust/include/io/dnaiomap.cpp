//============================================================================
// Name         : dnaiomap.cpp
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
// Description  : DynAdjust station map file io operations
//============================================================================

#include <fstream>

#include <include/io/dnaiomap.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

namespace dynadjust { 
namespace iostreams {

void dna_io_map::load_map_file(const string& map_filename, pv_string_uint32_pair stnsMap) 
{	
	std::ifstream map_file;
	stringstream ss;
	ss << "load_map_file(): An error was encountered when opening " << map_filename << "." << endl;

	try {
		// open stations map file.  Throws runtime_error on failure.
		file_opener(map_file, map_filename, ios::in | ios::binary, binary, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_map_file(): An error was encountered when reading from " << map_filename << "." << endl;

	stnsMap->clear();
	UINT32 mapsize;
	string_uint32_pair stnID;
	char stationName[STN_NAME_WIDTH];	// 56 characters
	
	try {
		// read the file information
		readFileInfo(map_file);
		
		// read the number of records
		map_file.read(reinterpret_cast<char *>(&mapsize), sizeof(UINT32));
		stnsMap->reserve(mapsize);
		
		// read the records
		for (UINT32 i=0; i<mapsize; i++)
		{
			map_file.read(const_cast<char *>(stationName), sizeof(stationName));
			stnID.first = stationName;
			map_file.read(reinterpret_cast<char *>(&stnID.second), sizeof(UINT32));
			stnsMap->push_back(stnID);
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

	map_file.close();

	if (stnsMap->size() < mapsize)
		throw boost::enable_current_exception(
			runtime_error("load_map_file(): Could not allocate sufficient memory for the Station map."));
}

void dna_io_map::write_map_file(const string& map_filename, pv_string_uint32_pair stnsMap)
{
	std::ofstream map_file;
	stringstream ss;
	ss << "write_map_file(): An error was encountered when opening " << map_filename << "." << endl;

	try {
		// Open station map file.  Throws runtime_error on failure.
		file_opener(map_file, map_filename, 
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
	ss << "write_map_file(): An error was encountered when writing to " << map_filename << "." << endl;

	v_string_uint32_pair::const_iterator _it_stnmap(stnsMap->begin());
	UINT32 mapval(static_cast<UINT32>(stnsMap->size()));
	try {
		// write the file information
		writeFileInfo(map_file);
		
		// write the data
		map_file.write(reinterpret_cast<char *>(&mapval), sizeof(UINT32));
		char stationName[STN_NAME_WIDTH];
		memset(stationName, '\0', sizeof(stationName));
		for (; _it_stnmap!=stnsMap->end(); ++_it_stnmap)
		{
			strcpy(stationName, _it_stnmap->first.c_str());
			mapval = _it_stnmap->second;
			map_file.write(stationName, sizeof(stationName));
			map_file.write(reinterpret_cast<char *>(&mapval), sizeof(UINT32));
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

	map_file.close();
}
	

void dna_io_map::write_map_file_txt(const string& map_filename, pv_string_uint32_pair stnsMap)
{
	std::ofstream map_file;
	stringstream ss;
	ss << "write_map_file_txt(): An error was encountered when opening " << map_filename << "." << endl;

	try {
		// Create station map text file.  Throws runtime_error on failure.
		file_opener(map_file, map_filename);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	ss.str("");
	ss << "write_map_file_txt(): An error was encountered when writing to " << map_filename << "." << endl;

	try {
		stringstream ss_map;
		ss_map << stnsMap->size() << " stations";
		map_file << left << setw(STATION) << ss_map.str();
		map_file << setw(HEADER_20) << right << "Stn. index" << endl;
		v_string_uint32_pair::const_iterator _it_stnmap(stnsMap->begin());
		for (_it_stnmap=stnsMap->begin(); _it_stnmap!=stnsMap->end(); ++_it_stnmap)
		{
			map_file << setw(STATION) << left << 
				_it_stnmap->first.c_str() <<
				setw(HEADER_20) << right << _it_stnmap->second << endl;
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

	map_file.close();
}

void dna_io_map::load_renaming_file(const string& renaming_filename, pv_string_string_pair stnRenaming)
{
	std::ifstream renaming_file;

	stringstream ss;
	ss << "load_renaming_file(): An error was encountered when opening " << renaming_filename << "." << endl;

	// The contents of the renaming file is as follows:
	//
	//  --------------------------------------------------------------------------------
	//  DYNADJUST STATION RENAMING FILE
	//  
	//  Station count                      3 
	//  Station name width                 20
	//  --------------------------------------------------------------------------------
	//  
	//  STATION NAMES
	//  
	//  OLD NAME             NEW NAME
	//  -------------------- --------------------
	//  GABO                 262600060
	//  BEEC                 209901750
	//  PTLD                 341404410

	//
	try {
		// Load renaming file.  Throws runtime_error on failure.
		file_opener(renaming_file, renaming_filename, 
			ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	
	ss.str("");
	ss << "load_renaming_file(): An error was encountered when reading from " << renaming_filename << "." << endl;

	stnRenaming->clear();

	string_string_pair stnNames;
	string sBuf("");
	UINT32 stationWidth(STATION), stationCount(100);
	UINT32 line(0);
	
	try {
		
		// continue until "STATION NAMES" is found
		do 
		{
			line++;
			getline(renaming_file, sBuf);

			if (iequals(trimstr(sBuf), "STATION NAMES"))
				break;

			// blank or whitespace?
			if (trimstr(sBuf).empty())		
				continue;

			if (iequals(trimstr(sBuf.substr(0, 13)), "Station count"))
			{
				stationCount = lexical_cast<UINT16, string>(trimstr(sBuf.substr(PRINT_VAR_PAD)));
				continue;
			}

			if (iequals(trimstr(sBuf.substr(0, 18)), "Station name width"))
			{
				stationWidth = lexical_cast<UINT16, string>(trimstr(sBuf.substr(PRINT_VAR_PAD)));
				continue;
			}

		}
		while (!iequals(trimstr(sBuf), "STATION NAMES"));
		
		stnRenaming->reserve(stationCount);
		
		// Okay, now get the data
		while (!renaming_file.eof())
		{
			getline(renaming_file, sBuf);

			// blank or whitespace?
			if (trimstr(sBuf).empty())			
				continue;

			if (trimstr(sBuf).length() < stationWidth)
				continue;

			if (iequals(trimstr(sBuf.substr(0, 8)), "OLD NAME"))
				continue;

			if (iequals(trimstr(sBuf.substr(0, 3)), "---"))
				continue;

			// Ignore lines with blank station name
			if (trimstr(sBuf.substr(0, stationWidth)).empty())			
				continue;

			// Ignore lines with blank substitute name
			if (trimstr(sBuf.substr(stationWidth+1)).empty())			
				continue;

			// initialise
			stnNames.first = "";
			stnNames.second = "";

			// get the names
			stnNames.first = trimstr(sBuf.substr(0, stationWidth));	
			stnNames.second = trimstr(sBuf.substr(stationWidth+1));
			stnRenaming->push_back(stnNames);
		}
	}
	catch (const ios_base::failure& f) {
		if (renaming_file.eof())
		{
			renaming_file.close();
			return;
		}
		ss << f.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (const runtime_error& e) {
		if (renaming_file.eof())
		{
			renaming_file.close();
			return;
		}
		ss << e.what();
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	catch (...) {
		if (renaming_file.eof())
		{
			renaming_file.close();
			return;
		}
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	renaming_file.close();
}


} // dnaiostreams
} // dynadjust
