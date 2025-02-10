//============================================================================
// Name         : dnaiopdf.cpp
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
// Description  : DynAdjust pdf file io operations
//============================================================================

#include <include/io/dnaiopdf.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
namespace dynadjust { 
namespace iostreams {

std::string dna_io_pdf::form_pdf_action_command_string(const std::string& pdf_filename, const std::string& ddename, const std::string& action)
{

	if (pdf_filename.empty())
		return "";

#if defined(_WIN32) || defined(__WIN32__)
	
	char viewer_filepath[256];
	FindExecutable(pdf_filename.c_str(), 0, viewer_filepath);
	std::string viewer(path(viewer_filepath).stem());

	if (boost::iequals(viewer, "AcroRd32") || boost::iequals(viewer, "Acrobat"))
	{
		std::stringstream ss;
		ss << "\"" << viewer_filepath << "\" " << pdf_filename;
		return ss.str();
	}

#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
 

#endif
	

	return "";
}

std::string dna_io_pdf::form_pdf_close_string(const std::string& pdf_filename, const std::string& ddename)
{
	//return form_pdf_action_command_string(pdf_filename, ddename, "DocClose");
	return "";	
}

std::string dna_io_pdf::form_pdf_open_string(const std::string& pdf_filename, const std::string& ddename)
{
	// return form_pdf_action_command_string(pdf_filename, ddename, "DocOpen");
	return "";
}

} // dnaiostreams
} // dynadjust
