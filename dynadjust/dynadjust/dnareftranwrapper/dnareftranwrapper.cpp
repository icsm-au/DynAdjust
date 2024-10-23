//============================================================================
// Name         : dnareftranwrapper.cpp
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
// Description  : Reference Frame Transformation Executable
//============================================================================

#include <dynadjust/dnareftranwrapper/dnareftranwrapper.hpp>

using namespace dynadjust;

void PrintOutputFileHeaderInfo(std::ofstream* f_out, const string& out_file, project_settings* p, const string& header, UINT32& epsgCode)
{
	// Print formatted header
	print_file_header(*f_out, header);

	*f_out << setw(PRINT_VAR_PAD) << left << "File name:" << system_complete(out_file).string() << endl << endl;
	
	*f_out << setw(PRINT_VAR_PAD) << left << "Command line arguments: ";
	*f_out << p->r.command_line_arguments << endl << endl;

	*f_out << setw(PRINT_VAR_PAD) << left << "Network name:" <<  p->g.network_name << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Input folder: " << system_complete(p->g.input_folder).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Output folder: " << system_complete(p->g.output_folder).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Stations file:" << system_complete(p->r.bst_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Measurements file:" << system_complete(p->r.bms_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Target reference frame:" << p->r.reference_frame << endl;

	if (!p->r.epoch.empty())
	{
		// Has the user supplied the year only?
		if (p->r.epoch.rfind(".") == string::npos)
			p->r.epoch.insert(0, "01.01.");

		*f_out << setw(PRINT_VAR_PAD) << left << "Target epoch: " <<
			formattedDateStringFromNumericString<date>(p->r.epoch);
		if (isEpsgDatumStatic(epsgCode))
			*f_out << " (ignored: " << p->r.reference_frame << " is static)";
		*f_out << endl;
	}
	
	if (p->r.plate_model_option > 0)
	{
		*f_out << setw(PRINT_VAR_PAD) << left << "Plate boundaries file: " << p->r.tpb_file << endl;
		*f_out << setw(PRINT_VAR_PAD) << left << "Plate pole parameter file: " << p->r.tpp_file << endl;
	}
	
	if (p->i.export_dynaml)
	{
		if (p->i.export_single_xml_file)
			*f_out << setw(PRINT_VAR_PAD) << left << "DynaML output file: " << p->i.xml_outfile << endl;
		else
		{
			*f_out << setw(PRINT_VAR_PAD) << left << "DynaML station file: " << p->i.xml_stnfile << endl;
			*f_out << setw(PRINT_VAR_PAD) << left << "DynaML measurement file: " << p->i.xml_msrfile << endl;
		}				
	}
	if (p->i.export_dna_files)
	{
		*f_out << setw(PRINT_VAR_PAD) << left << "DNA station file: " << p->i.dna_stnfile << endl;
		*f_out << setw(PRINT_VAR_PAD) << left << "DNA measurement file: " << p->i.dna_msrfile << endl;
	}

//	if (p->o._export_snx_file)
//		*f_out << setw(PRINT_VAR_PAD) << left << "SINEX file: " << p->o._snx_file << endl;


	*f_out << OUTPUTLINE << endl << endl;
}


int ParseCommandLineOptions(const int& argc, char* argv[], const variables_map& vm, project_settings& p, UINT32& epsgCode)
{
	// capture command line arguments
	for (int cmd_arg(0); cmd_arg<argc; ++cmd_arg)
	{
		 p.r.command_line_arguments += argv[cmd_arg];
		 p.r.command_line_arguments += " ";
	}

	if (vm.count(PROJECT_FILE))
	{
		if (exists(p.g.project_file))
		{
			try {
				CDnaProjectFile projectFile(p.g.project_file, reftranSetting);
				p = projectFile.GetSettings();
			}
			catch (const runtime_error& e) {
				cout << endl << "- Error: " << e.what() << endl;
				return EXIT_FAILURE;
			}
			
			return EXIT_SUCCESS;
		}

		cout << endl << "- Error: project file " << p.g.project_file << " does not exist." << endl << endl;
		return EXIT_FAILURE;
	}

	if (!vm.count(NETWORK_NAME))
	{
		cout << endl << "- Nothing to do - no network name specified. " << endl << endl;  
		return EXIT_FAILURE;
	}
	
	p.g.project_file = formPath<string>(p.g.output_folder, p.g.network_name, "dnaproj");
	p.r.rft_file = formPath<string>(p.g.output_folder, p.g.network_name, "rft");

	bool frameSupplied(true);
	project_settings tmp;

	if (!vm.count(REFERENCE_FRAME))
	{
		if (exists(p.g.project_file))
		{
			try {
				CDnaProjectFile projectFile(p.g.project_file, reftranSetting);
				tmp = projectFile.GetSettings();
				p.r.reference_frame = tmp.r.reference_frame;
				frameSupplied = false;
			}
			catch (const runtime_error& e) {
				cout << endl << "- Error: " << e.what() << endl;
				return EXIT_FAILURE;
			}
		}

		//cout << endl << "- Reference frame was not supplied.  Using project default (" << p.r.reference_frame << ")" << endl << endl;
		
		try
		{
			// Okay, no frame supplied, check the frame in the project settings.
			// The following throws an exception if the frame is unknown
			string epsg = epsgStringFromName<string>(p.r.reference_frame);
		}
		catch (const runtime_error& e) {
			cout << endl << "- Error: " << e.what() << endl;
			return EXIT_FAILURE;
		}

		// update epsgCode
		epsgCode = epsgCodeFromName<UINT32>(p.r.reference_frame);
	}

	if (!vm.count(EPOCH))
	{
		try
		{
			if (frameSupplied)
				// Okay, frame supplied, but no epoch supplied.
				// Set the epoch to be the reference epoch of the supplied reference frame
				p.r.epoch = referenceepochFromEpsgCode<UINT32>(epsgCodeFromName<UINT32, string>(p.r.reference_frame));
			else
				// Take the epoch from the project file
				p.r.epoch = tmp.r.epoch;
		}
		catch (const runtime_error& e) {
			cout << endl << "- Error: " << e.what() << endl;
			return EXIT_FAILURE;
		}
	}

	if (vm.count(TECTONIC_PLATE_BDY_FILE))
	{
		if (!exists(p.r.tpb_file))
		{
			cout << endl << "- Error: ";
			cout << endl << "tectonic plate boundary file " << endl << "               ";
			cout << p.r.tpb_file << " does not exist." << endl << endl;
			return EXIT_FAILURE;
		}
	}

	if (vm.count(TECTONIC_PLATE_POLE_FILE))
	{
		if (!exists(p.r.tpp_file))
		{
			cout << endl << "- Error: ";
			cout << endl << "Euler pole parameters file " << endl << "               ";
			cout << p.r.tpp_file << " does not exist." << endl << endl;
			return EXIT_FAILURE;
		}
	}

	// binary station file location (input)
	if (vm.count(BIN_STN_FILE))
		p.r.bst_file = formPath<string>(p.g.input_folder, p.r.bst_file);
	else
		p.r.bst_file = formPath<string>(p.g.output_folder, p.g.network_name, "bst");

	// binary station file location (input)
	if (vm.count(BIN_MSR_FILE))
		p.r.bms_file = formPath<string>(p.g.input_folder, p.r.bms_file);
	else
		p.r.bms_file = formPath<string>(p.g.output_folder, p.g.network_name, "bms");

	if (!exists(p.r.bst_file) || !exists(p.r.bms_file))
	{
		cout << endl << "- Nothing to do: ";  
			
		if (p.g.network_name.empty())
			cout << endl << "network name has not been specified specified, and " << endl << "               ";  
		cout << p.r.bst_file << " and " << p.r.bms_file << " do not exist." << endl << endl;  
		return EXIT_FAILURE;
	}

	// convert to upper case
	str_toupper<int>(p.r.reference_frame);

	//////////////////////////////////////////////////////////////////////////////
	// Export options
	
	// Create file name based on the provided block
	string fileName(p.g.network_name);
	fileName.append(".").append(p.r.reference_frame);

	if (!isEpsgDatumStatic(epsgCode))
	{
		if (iequals(p.r.epoch, "today"))
			p.r.epoch = stringFromToday<date>();
		fileName.append(".").append(p.r.epoch);
	}

	// Export to dynaml?
	if (vm.count(EXPORT_XML_FILES))
	{
		p.i.export_dynaml = 1;
	
		// single file for both stations and measurements
		if (vm.count(EXPORT_SINGLE_XML_FILE))
		{
			p.i.export_single_xml_file = 1;
			p.i.xml_outfile = formPath<string>(p.g.output_folder, 
				fileName, "xml");
		}
		// unique files for stations and measurements
		else
		{
			p.i.export_single_xml_file = 0;
			p.i.xml_stnfile = formPath<string>(p.g.output_folder, 
				fileName + "stn", "xml");
			p.i.xml_msrfile = formPath<string>(p.g.output_folder, 
				fileName + "msr", "xml");
		}
	}

	// Export dna files
	if (vm.count(EXPORT_DNA_FILES))
	{
		p.i.export_dna_files = 1;		
		p.i.dna_stnfile = formPath<string>(p.g.output_folder, 
			fileName, "stn");
		p.i.dna_msrfile = formPath<string>(p.g.output_folder, 
			fileName, "msr");
	}

//	// Export dna files
//	if (vm.count(EXPORT_SNX_FILE))
//	{
//		p.o._export_snx_file = 1;		
//		p.o._snx_file = formPath<string>(p.g.output_folder, 
//			fileName, "snx");
//	}

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{	
	// create banner message
	string cmd_line_banner;
	fileproc_help_header(&cmd_line_banner);

	project_settings p;

	variables_map vm;
	positional_options_description positional_options;
	
	options_description standard_options("+ " + string(ALL_MODULE_STDOPT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description transformation_options("+ " + string(REFTRAN_MODULE_TRANSFORM), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description export_options("+ " + string(ALL_MODULE_EXPORT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description generic_options("+ " + string(ALL_MODULE_GENERIC), PROGRAM_OPTIONS_LINE_LENGTH);

	string cmd_line_usage("+ ");
	cmd_line_usage.append(__BINARY_NAME__).append(" usage:  ").append(__BINARY_NAME__).append(" ").append(NETWORK_NAME).append(" [options]");
	options_description allowable_options(cmd_line_usage, PROGRAM_OPTIONS_LINE_LENGTH);

	try {
		// Declare a group of options that will be 
		// allowed only on command line		
		standard_options.add_options()
			(PROJECT_FILE_P, value<string>(&p.g.project_file),
				"Project file name. Full path to project file. If none specified, a new file is created using input-folder and network-name.")
			(NETWORK_NAME_N, value<string>(&p.g.network_name), 
				"Network name. User defined name for all output files. Default is \"network#\".")
			(INPUT_FOLDER_I, value<string>(&p.g.input_folder),
				"Path containing all input files")
			(OUTPUT_FOLDER_O, value<string>(&p.g.output_folder),		// default is ./,
				"Path for all output files")
			(BIN_STN_FILE, value<string>(&p.r.bst_file),
				"Binary station file name. Overrides network name.")
			(BIN_MSR_FILE, value<string>(&p.r.bms_file),
				"Binary measurement file name. Overrides network name.")
			;

		transformation_options.add_options()
			(REFERENCE_FRAME_R, value<string>(&p.r.reference_frame), 
				"Target reference frame for all stations and datum-dependent measurements.")
			(EPOCH_E, value<string>(&p.r.epoch),
				"Projected date for the transformed stations and measurements. arg is a dot delimited string \"dd.mm.yyyy\", or \"today\" if today's date is required. If no date is supplied, the reference epoch of the supplied reference frame will be used.")
			(TECTONIC_PLATE_MODEL_OPTION, value<UINT16>(&p.r.plate_model_option),
				string("Plate motion model option.\n"
					"  0: Assume all stations are on the Australian plate (default)\n"
					"  1: Interpolate plate motion model parameters from a defined\n"
					"     set of global tectonic plates. For this option, a global\n"
					"     tectonic plate boundary file and corresponding Euler\n"
					"     pole parameters file must be provided.").c_str())
			(TECTONIC_PLATE_BDY_FILE_B, value<string>(&p.r.tpb_file),
				string("Global tectonic plate boundaries.").c_str())
			(TECTONIC_PLATE_POLE_FILE_M, value<string>(&p.r.tpp_file), 
				string("Euler pole parameters corresponding to the global tectonic plate boundaries supplied with option --" +
					StringFromT(TECTONIC_PLATE_BDY_FILE) +
					".").c_str())
			;

		export_options.add_options()
			(EXPORT_XML_FILES,
				"Export transformed stations and measurements to DynaML (DynAdjust XML) format.")
			(EXPORT_SINGLE_XML_FILE,
				"Create a single DynaML file for stations and measurements.")
			(EXPORT_DNA_FILES,
				"Export transformed stations and measurements to DNA STN and MSR format.")
			//(EXPORT_SNX_FILE,
			//	"Export transformed station coordinates and full variance matrix to SINEX file.")
			;

		generic_options.add_options()
			(VERBOSE, value<UINT16>(&p.g.verbose),
				string("Give detailed information about what ").append(__BINARY_NAME__).append(" is doing.\n  0: No information (default)\n  1: Helpful information\n  2: Extended information\n  3: Debug level information").c_str())
			(QUIET,
				string("Suppresses all explanation of what ").append(__BINARY_NAME__).append(" is doing unless an error occurs").c_str())
			(VERSION_V, "Display the current program version")
			(HELP_H, "Show this help message")
			(HELP_MODULE_H, value<string>(),
				"Provide help for a specific help category.")
			;

		allowable_options.add(standard_options).add(transformation_options).add(export_options).add(generic_options);

		// add "positional options" to handle command line tokens which have no option name
		positional_options.add(NETWORK_NAME, -1);
		
		command_line_parser parser(argc, argv);
		store(parser.options(allowable_options).positional(positional_options).run(), vm);
		notify(vm);
	} 
	catch (const std::exception& e) {
		cout << "- Error: " << e.what() << endl;
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_FAILURE;
	}
	catch (...) 
	{
		cout << "+ Exception of unknown type!\n";
		return EXIT_FAILURE;
	}

	if (argc < 2)
	{
		cout << endl << "- Nothing to do - no options provided. " << endl << endl;  
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_FAILURE;
	}

	if (vm.count(VERSION))
	{
		cout << cmd_line_banner << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count(HELP))
	{
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_SUCCESS;
	}

	if (vm.count(HELP_MODULE))
	{
		cout << cmd_line_banner;
		string original_text = vm[HELP_MODULE].as<string>();
		string help_text = str_upper<string>(original_text);

		if (str_upper<string, char>(ALL_MODULE_STDOPT).find(help_text) != string::npos) {
			cout << standard_options << endl;
		}
		else if (str_upper<string, char>(REFTRAN_MODULE_TRANSFORM).find(help_text) != string::npos) {
			cout << transformation_options << endl;
		}
		else if (str_upper<string, char>(ALL_MODULE_EXPORT).find(help_text) != string::npos) {
			cout << export_options << endl;
		}
		else if (str_upper<string, char>(ALL_MODULE_GENERIC).find(help_text) != string::npos) {
			cout << generic_options << endl;
		}
		else {
			cout << endl << "- Error: Help module '" <<
				original_text << "' is not in the list of options." << endl;
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	if (vm.count(TECTONIC_PLATE_MODEL_OPTION))
	{
		if (!vm.count(TECTONIC_PLATE_BDY_FILE))
		{
			cout << endl << "- Error: A plate boundary file must be supplied in order to interpolate plate motion model parameters. See command line help for further information." << endl;
			return EXIT_FAILURE;
		}

		if (!vm.count(TECTONIC_PLATE_POLE_FILE))
		{
			cout << endl << "- Error: A Euler pole parameters file must be supplied in order to interpolate plate motion model parameters. See command line help for further information." << endl;
			return EXIT_FAILURE;
		}
	}

	UINT32 epsgCode(epsgCodeFromName<UINT32>(p.r.reference_frame));

	if (ParseCommandLineOptions(argc, argv, vm, p, epsgCode) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	std::ofstream rft_file;
	try {
		// Create import log file.  Throws runtime_error on failure.
		file_opener(rft_file, p.r.rft_file);
	}
	catch (const runtime_error& e) {
		stringstream ss;
		ss << "- Error: Could not open " << p.r.rft_file << ". \n  Check that the file exists and that the file is not already opened." << endl;
		cout << ss.str() << e.what() << endl;
		return EXIT_FAILURE;
	}

	if (vm.count(QUIET))
		p.g.quiet = 1;

	if (!p.g.quiet)
	{
		cout << endl << cmd_line_banner;
	
		cout << "+ Options:" << endl; 
		cout << setw(PRINT_VAR_PAD) << left << "  Network name: " <<  p.g.network_name << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Input folder: " << p.g.input_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Output folder: " << p.g.output_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary station file: " << p.r.bst_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary measurement file: " << p.r.bms_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Target reference frame: " << p.r.reference_frame;
		if (vm.count(REFERENCE_FRAME))
			cout << " (user supplied)" << endl;
		else
			cout << " (project default)" << endl;

		// try to parse user supplied string.  If this fails, then there's not much point in attempting
		// to transform stations and measurements
		try 
		{
			if (!p.r.epoch.empty())
			{
				// Has the user supplied the year only?
				if (p.r.epoch.rfind(".") == string::npos)
					p.r.epoch.insert(0, "01.01.");

				cout << setw(PRINT_VAR_PAD) << left << "  Target epoch: " <<
					formattedDateStringFromNumericString<date>(p.r.epoch);
				if (isEpsgDatumStatic(epsgCode))
					cout << " (ignored: " << p.r.reference_frame << " is static)";
				cout << endl;
			}
		}
		catch (const runtime_error& e) {
			cout << endl << "- Error: " << e.what() << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}
		catch (const RefTranException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}
		catch (...) {
			cout << endl << "- Error: Unknown error." << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}

		if (p.r.plate_model_option > 0)
		{
			cout << setw(PRINT_VAR_PAD) << left << "  Plate boundaries file: " << p.r.tpb_file << endl;
			cout << setw(PRINT_VAR_PAD) << left << "  Plate pole parameter file: " << p.r.tpp_file << endl;
		}

		// Export options
		if (p.i.export_dynaml)
		{
			if (p.i.export_single_xml_file)
				cout << setw(PRINT_VAR_PAD) << left << "  DynaML output file: " << p.i.xml_outfile << endl;
			else
			{
				cout << setw(PRINT_VAR_PAD) << left << "  DynaML station file: " << p.i.xml_stnfile << endl;
				cout << setw(PRINT_VAR_PAD) << left << "  DynaML measurement file: " << p.i.xml_msrfile << endl;
			}				
		}
		if (p.i.export_dna_files)
		{
			cout << setw(PRINT_VAR_PAD) << left << "  DNA station file: " << p.i.dna_stnfile << endl;
			cout << setw(PRINT_VAR_PAD) << left << "  DNA measurement file: " << p.i.dna_msrfile << endl;
		}

//		if (p.o._export_snx_file)
//			cout << setw(PRINT_VAR_PAD) << left << "  SINEX file: " << p.o._snx_file << endl;

		cout << endl;
	}

	PrintOutputFileHeaderInfo(&rft_file, p.r.rft_file, &p, "DYNADJUST REFTRAN LOG FILE", epsgCode);

	dna_reftran refTran(p, &rft_file);
	stringstream ss_msg;

	// Load plate boundary and euler pole information
	if (vm.count(TECTONIC_PLATE_MODEL_OPTION))
	{
		ss_msg << "+ Loading global tectonic plate boundaries and plate motion information... ";

		if (!p.g.quiet)
			cout << ss_msg.str();		
		rft_file << ss_msg.str();
	
		try
		{
			refTran.LoadTectonicPlateParameters(p.r.tpb_file, p.r.tpp_file);
		}
		catch (const runtime_error& e) {
			cout << endl << "- Error: " << e.what() << endl;
			rft_file << endl << "- Error: " << e.what() << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}

		if (!p.g.quiet)
			cout << "done." << endl;

		if (p.g.verbose == 0)
			rft_file << "done." << endl;
		else
			rft_file << endl << "+ Done." << endl << endl;
	}

	ss_msg.str("");
	ss_msg << "+ Transforming stations and measurements... ";
	if (!p.g.quiet)
		cout << ss_msg.str();		
	rft_file << ss_msg.str();

	ss_msg.str("");

	try
	{
		// Transform binary station file
		refTran.TransformBinaryFiles(p.r.bst_file, p.r.bms_file,
			p.r.reference_frame, p.r.epoch);
	}
	catch (const runtime_error& e) {
		cout << endl << "- Error: " << e.what() << endl;
		rft_file << endl << "- Error: " << e.what() << endl;
		rft_file.close();
		return EXIT_FAILURE;
	}
	catch (const RefTranException& e) {
		switch (e.exception_type())
		{
		case REFTRAN_WGS84_TRANS_UNSUPPORTED:
			break;
		}
		cout << endl << endl << "- Error: " << e.what() << endl;
		rft_file << endl << endl << "- Error: " << e.what() << endl;
		rft_file.close();
		return EXIT_FAILURE;
	}
	catch (...) {
		cout << endl << "- Error: Unknown error." << endl;
		rft_file << endl << "- Error: Unknown error." << endl;
		rft_file.close();
		return EXIT_FAILURE;
	}
	
	if (!p.g.quiet)
		cout << "done." << endl;

	if (p.g.verbose == 0)
		rft_file << "done." << endl;
	else
		rft_file << endl << "+ Done." << endl << endl;
	
	ss_msg.str("");

	// Station summary
	if (refTran.StationsTransformed())
	{
		ss_msg << "+ Transformed " << refTran.StationsTransformed();
	
		if (refTran.StationsTransformed() == 1)
			ss_msg << " station." << endl;
		else
			ss_msg << " stations." << endl;
	}
	else
		ss_msg << "+ No stations were transformed." << endl;
	
	if (!p.g.quiet)
		cout << ss_msg.str();
	rft_file << ss_msg.str();

	ss_msg.str("");

	if (refTran.StationsNotTransformed() > 0)
	{
		if (refTran.StationsTransformed())
			ss_msg << "+ Note: ";
		else
			ss_msg << "  ";

		ss_msg << refTran.StationsNotTransformed();
		if (refTran.StationsNotTransformed() == 1)
			ss_msg << " station was";
		else
			ss_msg << " stations were";

		ss_msg << " already referenced to " << p.r.reference_frame;
		if (!isEpsgDatumStatic(epsgCode))
		{
			if (!p.r.epoch.empty())
				ss_msg << ", epoch " << p.r.epoch;
		}
		ss_msg << endl;
	}

	if (!p.g.quiet)
		cout << ss_msg.str();
	rft_file << ss_msg.str();

	ss_msg.str("");

	// Measurement summary
	if (refTran.MeasurementsTransformed())
	{
		ss_msg << "+ Transformed " << refTran.MeasurementsTransformed();
	
		if (refTran.MeasurementsTransformed() == 1)
			ss_msg << " measurement." << endl;
		else
			ss_msg << " measurements." << endl;
	}
	else
		ss_msg << "+ No measurements were transformed." << endl;

	if (!p.g.quiet)
		cout << ss_msg.str();
	rft_file << ss_msg.str();

	ss_msg.str("");

	if (refTran.MeasurementsNotTransformed() > 0)
	{
		if (refTran.MeasurementsTransformed())
			ss_msg << "+ Note: ";
		else
			ss_msg << "  ";

		ss_msg << refTran.MeasurementsNotTransformed();
		if (refTran.MeasurementsNotTransformed() == 1)
			ss_msg << " measurement was";
		else
			ss_msg << " measurements were";

		ss_msg << " already referenced to " << p.r.reference_frame;
		if (!isEpsgDatumStatic(epsgCode))
		{
			if (!p.r.epoch.empty())
				ss_msg << ", epoch " << p.r.epoch;
		}
		ss_msg << endl;
	}
	
	if (!p.g.quiet)
		cout << ss_msg.str() << endl;
	rft_file << ss_msg.str() << endl;

	ss_msg.str("");

	if (p.i.export_dynaml) 
	{
		try {
			
			if (p.i.export_single_xml_file)
			{
				// Single output file
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
					cout.flush();
					rft_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
				}
				refTran.SerialiseDynaML(
					p.i.xml_outfile, 
					(p.i.flag_unused_stn ? true : false));	
			}
			else
			{
				// Separate output files (default)
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to:" << endl << 
						"  - " << leafStr<string>(p.i.xml_stnfile) << endl <<
						"  - " << leafStr<string>(p.i.xml_msrfile) << "... ";
					cout.flush();
					rft_file << "+ Exporting stations and measurements to:" << endl <<
						"  - " << leafStr<string>(p.i.xml_stnfile) << endl <<
						"  - " << leafStr<string>(p.i.xml_msrfile) << "... ";
				}
				refTran.SerialiseDynaML(
					p.i.xml_stnfile, p.i.xml_msrfile, 
					(p.i.flag_unused_stn ? true : false));	
			}

			if (!p.g.quiet)
			{
				cout << "done." << endl;
				cout.flush();
			}

			rft_file << "done." << endl;
			
		}
		catch (const XMLInteropException& e) {
			cout.flush();
			cout << endl << "- Error: " << e.what() << endl;
			rft_file << endl << "- Error: " << e.what() << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}
	}

	if (p.i.export_dna_files) 
	{
		try {
			// Separate output files (default)
			if (!p.g.quiet)
			{
				cout << "+ Exporting stations and measurements to:" << endl <<
					"  - " << leafStr<string>(p.i.dna_stnfile) << endl <<
					"  - " << leafStr<string>(p.i.dna_msrfile) << "... ";
				cout.flush();
			}
			rft_file << "+ Exporting stations and measurements to:" << endl <<
				"  - " << leafStr<string>(p.i.dna_stnfile) << endl <<
				"  - " << leafStr<string>(p.i.dna_msrfile) << "... ";
				
			refTran.SerialiseDNA(
				p.i.dna_stnfile, p.i.dna_msrfile, 
				(p.i.flag_unused_stn ? true : false));
			
			if (!p.g.quiet)
				cout << "Done." << endl;
			rft_file << "Done." << endl;
		}
		catch (const XMLInteropException& e) {
			cout.flush();
			cout << endl << "- Error: " << e.what() << endl;
			rft_file << endl << "- Error: " << e.what() << endl;
			rft_file.close();
			return EXIT_FAILURE;
		}
	}

//	// Print adjusted stations and measurements to SINEX
//	if (p.o._export_snx_file)
//	{
//		// Export to SINEX
//		if (!p.g.quiet)
//			cout << "+ Exporting stations and measurements to " << 
//				leafStr<string>(p.o._snx_file) << "... ";
//		cout.flush();
//		rft_file << "+ Exporting stations and measurements to " << 
//			leafStr<string>(p.o._snx_file) << "... ";
//
//		bool success(refTran.PrintTransformedStationCoordinatestoSNX());
//
//		// SomeFunc()
//		if (!p.g.quiet)
//			cout << " done." << endl;
//		rft_file << " done." << endl;
//
//		ss_msg.str("");
//
//		if (!success)
//		{
//			ss_msg << "- Warning: The SINEX export process produced some warnings." << endl;
//			ss_msg << "  See " << p.g.network_name << "*.snx.err for details." << endl; 
//		}
//		cout << ss_msg.str();
//		rft_file << ss_msg.str();
//	}

	if (!p.g.quiet)
	{
		cout << endl;
	}

	// Look for a project file.  If it exists, open and load it.
	// Update the import settings.
	// Print the project file. If it doesn't exist, it will be created.
	CDnaProjectFile projectFile;
	if (exists(p.g.project_file))
		projectFile.LoadProjectFile(p.g.project_file);
	
	projectFile.UpdateSettingsReftran(p);
	projectFile.PrintProjectFile();

	rft_file.close();
	return REFTRAN_SUCCESS;
}
