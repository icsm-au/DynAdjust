//============================================================================
// Name         : dnasegmentwrapper.cpp
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
// Description  : DynAdjust Network Segmentation Executable
//============================================================================

#include <dynadjust/dnasegmentwrapper/dnasegmentwrapper.hpp>

using namespace dynadjust;

bool running;
boost::mutex cout_mutex;

int ParseCommandLineOptions(const int& argc, char* argv[], const variables_map& vm, project_settings& p)
{
	// capture command line arguments
	for (int cmd_arg(0); cmd_arg<argc; ++cmd_arg)
	{
		 p.s.command_line_arguments += argv[cmd_arg];
		 p.s.command_line_arguments += " ";
	}

	if (vm.count(PROJECT_FILE))
	{
		if (exists(p.g.project_file))
		{
			try {
				CDnaProjectFile projectFile(p.g.project_file, segmentSetting);
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

	// input files
	p.s.asl_file = formPath<string>(p.g.input_folder, p.g.network_name, "asl");		// associated stations list
	p.s.aml_file = formPath<string>(p.g.input_folder, p.g.network_name, "aml");		// associated measurements list
	p.s.map_file = formPath<string>(p.g.input_folder, p.g.network_name, "map");		// station names map
	
	if (vm.count(NET_FILE))
		p.s.net_file = formPath<string>(p.g.input_folder, p.g.network_name, "net");		// Starting stations file
	
	// binary station file location (input)
	if (vm.count(BIN_STN_FILE))
		p.s.bst_file = formPath<string>(p.g.input_folder, p.s.bst_file);
	else
		p.s.bst_file = formPath<string>(p.g.output_folder, p.g.network_name, "bst");

	// binary station file location (input)
	if (vm.count(BIN_MSR_FILE))
		p.s.bms_file = formPath<string>(p.g.input_folder, p.s.bms_file);
	else
		p.s.bms_file = formPath<string>(p.g.output_folder, p.g.network_name, "bms");

	if (!exists(p.s.bst_file) || !exists(p.s.bms_file))
	{
		cout << endl << "- Nothing to do: ";  
			
		if (p.g.network_name.empty())
			cout << endl << "network name has not been specified specified, and " << endl << "               ";  
		cout << p.s.bst_file << " and " << p.s.bms_file << " do not exist." << endl << endl;  
		return EXIT_FAILURE;
	}

	// output files
	// User supplied segmentation file
	if (vm.count(SEG_FILE))
	{
		// Does it exist?
		if (!exists(p.s.seg_file))
		{
			// Look for it in the input folder
			p.s.seg_file = formPath<string>(p.g.input_folder, leafStr<string>(p.s.seg_file));

			if (!exists(p.s.seg_file))
			{
				cout << endl << "- Error: ";  
				cout << "Segmentation file " << leafStr<string>(p.s.seg_file) << " does not exist." << endl << endl;  
				return EXIT_FAILURE;
			}
		}
	}
	else
		p.s.seg_file = formPath<string>(p.g.input_folder, p.g.network_name, "seg");

	// Station appearance file
	p.s.sap_file = formPath<string>(p.g.input_folder, p.g.network_name, "sap");
	
	if (vm.count(TEST_INTEGRITY))
		p.i.test_integrity = 1;

	if (vm.count(SEG_FORCE_CONTIGUOUS))
		p.s.force_contiguous_blocks = 1;

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{	
	// create banner message
	string cmd_line_banner;
	fileproc_help_header(&cmd_line_banner);

	string stnfilename, msrfilename;
	
	project_settings p;

	variables_map vm;
	positional_options_description positional_options;

	options_description standard_options("+ " + string(ALL_MODULE_STDOPT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description config_options("+ " + string(SEGMENT_MODULE_CONFIG), PROGRAM_OPTIONS_LINE_LENGTH);
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
				"Network name. User defined name for all input and output files. Default is \"network#\".")
			(INPUT_FOLDER_I, value<string>(&p.g.input_folder),
				"Path containing all input files")
			(OUTPUT_FOLDER_O, value<string>(&p.g.output_folder),		// default is ./,
				"Path for all output files")
			(BIN_STN_FILE, value<string>(&p.s.bst_file),
				"Binary station file name. Overrides network name.")
			(BIN_MSR_FILE, value<string>(&p.s.bms_file),
				"Binary measurement file name. Overrides network name.")
			(SEG_FILE, value<string>(&p.s.seg_file),
				"Segmentation output file name. Overrides network name.")
			;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config options        
		config_options.add_options()
			(NET_FILE,
				"Look for a .net file containing stations to be incorporated within the first block.")
			(SEG_STARTING_STN, value<string>(&p.s.seg_starting_stns),
				"Additional stations to be incorporated within the first block. arg is a comma delimited string \"stn1, stn 2,stn3 , stn 4\".")
			(SEG_MIN_INNER_STNS, value<UINT16>(&p.s.min_inner_stations),
				(string("Minimum number of inner stations within each block. Default is ")+
					StringFromT(p.s.min_inner_stations)+string(".")).c_str())
			(SEG_THRESHOLD_STNS, value<UINT16>(&p.s.max_total_stations),
				(string("Threshold limit for maximum number of stations per block. Default is ")+
					StringFromT(p.s.max_total_stations)+string(".")).c_str())
			(SEG_FORCE_CONTIGUOUS, value<UINT16>(&p.s.force_contiguous_blocks),
				(string("Treatment of isolated networks:\n")+
				string("  0: Isolated networks as individual blocks ")+
				(p.s.force_contiguous_blocks==0 ? "(default)\n" : "\n")+
				string("  1: Force production of contiguous blocks ")+
				(p.s.force_contiguous_blocks==1 ? "(default)" : "")
				).c_str())
			(SEG_SEARCH_LEVEL, value<UINT16>(&p.s.seg_search_level),
				"Level to which searches should be conducted to find stations with the lowest measurement count. Default is 0.")
			(TEST_INTEGRITY,
				"Test the integrity of all output files.")
			;

		generic_options.add_options()
			(VERBOSE, value<UINT16>(&p.g.verbose),
				string("Give detailed information about what ").append(__BINARY_NAME__).append(" is doing.\n  0: No information (default)\n  1: Helpful information\n  2: Extended information\n  3: Debug level information").c_str())
				(QUIET,
					string("Suppresses all explanation of what ").append(__BINARY_NAME__).append(" is doing unless an error occurs").c_str())
					(VERSION_V, "Display the current program version")
			(HELP_H, "Show this help message")
			(HELP_MODULE, value<string>(),
				"Provide help for a specific help category.")
			;

		allowable_options.add(standard_options).add(config_options).add(generic_options);

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
		else if (str_upper<string, char>(SEGMENT_MODULE_CONFIG).find(help_text) != string::npos) {
			cout << config_options << endl;
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

	bool userSuppliedSegFile(false);
	if (!p.s.seg_file.empty())
		userSuppliedSegFile = true;
	bool userSuppliedBstFile(false);
	if (!p.s.bst_file.empty())
		userSuppliedBstFile = true;
	bool userSuppliedBmsFile(false);
	if (!p.s.bms_file.empty())
		userSuppliedBmsFile = true;

	if (ParseCommandLineOptions(argc, argv, vm, p) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	if (vm.count(QUIET))
		p.g.quiet = 1;
	
	if (!p.g.quiet)
	{
		cout << endl << cmd_line_banner;
		
		cout << "+ Options:" << endl; 
		cout << setw(PRINT_VAR_PAD) << left << "  Network name: " <<  p.g.network_name << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Input folder: " << p.g.input_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Output folder: " << p.g.output_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Associated station file: " << p.s.asl_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Associated measurement file: " << p.s.aml_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary station file: " << p.s.bst_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary measurement file: " << p.s.bms_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Segmentation output file: " << p.s.seg_file << endl;
		if (!p.s.net_file.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  Block 1 stations file: " << p.s.net_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Minimum inner stations: " <<  p.s.min_inner_stations << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Block size threshold: " <<  p.s.max_total_stations << endl;
		if (!p.s.seg_starting_stns.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  Additional Block 1 stations: " << p.s.seg_starting_stns << endl;
		else if (p.s.net_file.empty())
			cout << "  No initial station specified. The first station will be used." << endl;
		
		cout << endl;
	}

	// Should segment look for a net file?
	if (!p.s.net_file.empty())
	{
		if (!exists(p.s.net_file))
		{
			cout_mutex.lock();
			cout << endl;
			cout << "- Error: " << p.s.net_file << " does not exist." << endl;  
			cout << "  A file named " << p.g.network_name << ".net must exist in the input folder\n  in order to use this option." << endl << endl;
			cout_mutex.unlock();
			return EXIT_FAILURE;
		}
	}

	dna_segment netSegment;
	milliseconds elapsed_time(milliseconds(0));
	_SEGMENT_STATUS_ segmentStatus;
	string status_msg;

	try {
		netSegment.InitialiseSegmentation();
		running = true;

		// segment blocks using group thread
		thread_group ui_segment_threads;
		if (!p.g.quiet)
			ui_segment_threads.create_thread(dna_segment_progress_thread(&netSegment, &p));
		ui_segment_threads.create_thread(dna_segment_thread(&netSegment, &p, &segmentStatus, &elapsed_time, &status_msg));
		ui_segment_threads.join_all();
		
		switch (netSegment.GetStatus())
		{
		case SEGMENT_EXCEPTION_RAISED:
			running = false;
			cout << status_msg << endl << endl;
			return EXIT_FAILURE;
		default:
			break;
		}

		//// print station appearance file
		//if (!p.g.quiet)
		//	cout << "+ Printing station appearance list... ";
		//netSegment.WriteStationAppearanceList(p.s.sap_file);
		//if (!p.g.quiet)
		//	cout << "done." << endl;

		if (p.g.verbose > 1 && !p.g.quiet)
			netSegment.coutSummary(); 

		if (segmentStatus != SEGMENT_SUCCESS)
			cout << status_msg << endl;

		if (!p.g.quiet)
		{
			cout << "+ Segmentation statistics:" << endl;
			cout << endl << left << "  " <<
				setw(STATION) << "No. blocks" << 
				setw(STAT) << "Max size" << 
				setw(STAT) << "Min size" << 
				setw(STAT) << "Average" <<
				setw(STATION) << "Total size" << endl;
			cout << "  ";
			for (UINT32 i(0), j(STATION + STAT*3 + STATION); i<j; ++i)
				cout << "-";
			cout << endl << "  " << left <<
				setw(STATION) << netSegment.blockCount() << 
				setw(STAT) << setprecision(0) << netSegment.maxBlockSize() << 
				setw(STAT) << setprecision(0) << netSegment.minBlockSize() << 
				setw(STAT) << setprecision(2) << fixed << netSegment.averageblockSize() <<
				setw(STATION) << setprecision(0) << netSegment.stationSolutionCount() << endl;

			cout << endl;
		
		}

		if (!p.g.quiet)
			cout << "+ Verifying station connections... ";
		netSegment.VerifyStationConnections();
		if (!p.g.quiet)
			cout << "done." << endl;

		// print network segmentation block file
		if (!p.g.quiet)
			cout << "+ Printing blocks to " << leafStr<string>(p.s.seg_file) << "... ";
		netSegment.WriteSegmentedNetwork(p.s.seg_file);
		if (!p.g.quiet)
			cout << "done." << endl;

		
	} 
	catch (const NetSegmentException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return EXIT_FAILURE;
	} 
	catch (const runtime_error& e) {
		cout << "+ Exception of unknown type: " << e.what();
		return EXIT_FAILURE;
	}

	if (!userSuppliedSegFile)
		p.s.seg_file = "";
	if (!userSuppliedBstFile)
		p.s.bst_file = "";
	if (!userSuppliedBmsFile)
		p.s.bms_file = "";

	// Look for a project file.  If it exists, open and load it.
	// Update the import settings.
	// Print the project file. If it doesn't exist, it will be created.
	CDnaProjectFile projectFile;
	if (exists(p.g.project_file))
		projectFile.LoadProjectFile(p.g.project_file);
	
	projectFile.UpdateSettingsSegment(p);
	projectFile.PrintProjectFile();

	if (p.g.quiet)
		return EXIT_SUCCESS;

	cout << endl << formatedElapsedTime<string>(&elapsed_time, "+ Network segmentation took ") << endl;
	cout << "+ " << p.g.network_name << " is now ready for sequential phased adjustment." << endl << endl;
	
	return EXIT_SUCCESS;
}
