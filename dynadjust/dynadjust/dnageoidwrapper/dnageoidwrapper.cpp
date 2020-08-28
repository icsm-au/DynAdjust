//============================================================================
// Name         : dnageoidwrapper.cpp
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
// Description  : AusGeoid Grid File (NTv2) Interpolation library Executable
//============================================================================

#include <dynadjust/dnageoidwrapper/dnageoidwrapper.hpp>

using namespace dynadjust;

void PrintHelp(string message, const options_description* options)
{
	string title;
	fileproc_help_header(&title);

	cout << endl << title << endl << message << endl << endl << *options << endl;
}
	

void PrintVersion()
{
	string version;
	fileproc_help_header(&version);
	cout << version << endl;
}
	

bool CreateNTv2Grid(dna_geoid_interpolation* g, const char* dat_gridfilePath, const n_file_par* grid)
{
	//cout << "+ Creating NTv2 grid file... ";
	try {
		g->CreateNTv2File(dat_gridfilePath, grid);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}
	cout << endl;

	reportGridProperties(g, grid->filename, grid->filetype);

	return true;
}


	
bool createGridIndex(dna_geoid_interpolation* g, const char* gridfilePath, const char* gridfileType, const int& quiet)
{
	if (!quiet)
		cout << "+ Opening grid file... ";
	try {
		g->CreateGridIndex(gridfilePath, gridfileType);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}

	if (!quiet)
		cout << "done." << endl;

	return true;
}
	

bool reportGridProperties(dna_geoid_interpolation* g, const char* gridfilePath, const char* gridfileType)
{
	n_file_par grid_properties;
	
	try {
		g->ReportGridProperties(gridfilePath, gridfileType, &grid_properties);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}

	cout << "+ Grid properties for " << gridfilePath << ":" << endl;
	cout << "  - GS_TYPE  = " << grid_properties.chGs_type << endl;							// grid shift type (GS_TYPE)
	cout << "  - VERSION  = " << grid_properties.chVersion << endl;							// grid file version (VERSION)
	cout << "  - SYSTEM_F = " << grid_properties.chSystem_f << endl;						// reference system (SYSTEM_F)
	cout << "  - SYSTEM_T = " << grid_properties.chSystem_t << endl;						// reference system (SYSTEM_T)
	cout << "  - MAJOR_F  = " << setprecision(3) << fixed << grid_properties.daf << endl;	// semi major of from system (MAJOR_F)
	cout << "  - MAJOR_T  = " << setprecision(3) << fixed << grid_properties.dat << endl;	// semi major of to system (MAJOR_T)
	cout << "  - MINOR_F  = " << setprecision(3) << fixed << grid_properties.dbf << endl;	// semi minor of from system (MINOR_F)
	cout << "  - MINOR_T  = " << setprecision(3) << fixed << grid_properties.dbt << endl;	// semi minor of to system (MINOR_T)
	cout << "  - NUM_OREC = " << grid_properties.iH_info << endl;							// Number of header identifiers (NUM_OREC)
	cout << "  - NUM_SREC = " << grid_properties.iSubH_info << endl;						// Number of sub-header idents (NUM_SREC)
	cout << "  - NUM_FILE = " << grid_properties.iNumsubgrids << endl;						// number of subgrids in file (NUM_FILE)

	for (int i=0; i<grid_properties.iNumsubgrids; ++i)
	{
		cout << "  - SUBGRID " << i << ":" << endl;
		cout << "    - SUB_NAME = " << grid_properties.ptrIndex[i].chSubname << endl;  		// name of subgrid (SUB_NAME)
		cout << "    - PARENT   = " << grid_properties.ptrIndex[i].chParent << endl;		// name of parent grid (PARENT)
		cout << "    - CREATED  = " << grid_properties.ptrIndex[i].chCreated << endl;		// date of creation (CREATED)
		cout << "    - UPDATED  = " << grid_properties.ptrIndex[i].chUpdated << endl;		// date of last file update (UPDATED)
		cout << "    - S_LAT    = " << grid_properties.ptrIndex[i].dSlat << endl;			// lower latitude (S_LAT)
		cout << "    - N_LAT    = " << grid_properties.ptrIndex[i].dNlat << endl;			// upper latitude (N_LAT)
		cout << "    - E_LONG   = " << grid_properties.ptrIndex[i].dElong << endl;			// lower longitude (E_LONG)
		cout << "    - W_LONG   = " << grid_properties.ptrIndex[i].dWlong << endl;			// upper longitude (W_LONG)
		cout << "    - LAT_INC  = " << grid_properties.ptrIndex[i].dLatinc << endl;			// latitude interval (LAT_INC)
		cout << "    - LONG_INC = " << grid_properties.ptrIndex[i].dLonginc << endl;		// longitude interval (LONG_INC)
		cout << "    - GS_COUNT = " << grid_properties.ptrIndex[i].lGscount << endl;		// number of nodes (GS_COUNT)
	}
	cout << endl;
	return true;
}

bool InterpolateGridPoint(dna_geoid_interpolation* g, const char* gridfilePath, geoid_point* apInterpolant, 
	const int& method, const int& coordinate_format, const string& inputLatitude, const string& inputLongitude)
{
	try {
		if (method == BICUBIC)
			g->BiCubicTransformation(apInterpolant);
		else
			g->BiLinearTransformation(apInterpolant);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}

	if (apInterpolant->cVar.IO_Status != ERR_TRANS_SUCCESS)
		return false;
	
	cout << "+ Interpolation results for ";
	cout << inputLatitude << ", " << inputLongitude;
	if (coordinate_format == DMS)
		//cout << fixed << setprecision(6) << DegtoDms<double>(apInterpolant->cVar.dLatitude) << ", " << DegtoDms<double>(apInterpolant->cVar.dLongitude) << " (ddd.mmssss):" << endl;
		cout << " (ddd.mmssss):" << endl;
	else
		//cout << fixed << setprecision(6) << apInterpolant->cVar.dLatitude << ", " << apInterpolant->cVar.dLongitude << " (ddd.dddddd):" << endl;
		cout << " (ddd.dddddd):" << endl;
	
	cout << endl;

	cout << "  N value          = " << setw(6) << 
		right << setprecision(3) << apInterpolant->gVar.dN_value << " metres" << endl;			// N value
	cout << "  Deflections:" << endl;
	cout << "  - Prime meridian = " << setw(6) << 
		right << fixed << setprecision(2) << apInterpolant->gVar.dDefl_meridian << " seconds" << endl;			// N value
	cout << "  - Prime vertical = " << setw(6) << 
		right << apInterpolant->gVar.dDefl_primev << " seconds" << endl;			// N value
	cout << endl;
	return true;

} // InterpolateGridPoint
	

bool InterpolateGridPointFile(dna_geoid_interpolation* g, const char* inputfilePath, 
	const int& method, const int EllipsoidtoOrtho, const int& coordinate_format, 
	bool exportDnaGeoidFile, const char* dnageofilePath, string& outputfilePath)
{
	path inputFile(inputfilePath);
	if (inputFile.has_extension())
		outputfilePath = inputFile.parent_path().string() + inputFile.stem().string() + "_out" + inputFile.extension().string();
	else
	{
		outputfilePath = inputfilePath; 
		outputfilePath.append("_out");
	}

	char outfilePath[601];
	strcpy(outfilePath, outputfilePath.c_str());

	try {
		g->FileTransformation(inputfilePath, outfilePath, 
			method, EllipsoidtoOrtho, coordinate_format, 
			exportDnaGeoidFile, dnageofilePath);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}

	return true;

} // InterpolateGridPointFile


bool InterpolateGridBinaryStationFile(dna_geoid_interpolation* g, const string& bstnfilePath,
	const int& method, bool convertHeights, 
	bool exportDnaGeoidFile, const char* dnageofilePath)
{
	try {
		g->PopulateBinaryStationFile(bstnfilePath, method, convertHeights, 
			exportDnaGeoidFile, dnageofilePath);
	}
	catch (const NetGeoidException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return false;
	}

	return true;

} // InterpolateGridBinaryStationFile


int ParseCommandLineOptions(const int& argc, char* argv[], const variables_map& vm, project_settings& p)
{
	// capture command line arguments
	for (int cmd_arg(0); cmd_arg<argc; ++cmd_arg)
	{
		 p.n.command_line_arguments += argv[cmd_arg];
		 p.n.command_line_arguments += " ";
	}

	// Has the user supplied a project file?
	if (vm.count(PROJECT_FILE))
	{
		if (exists(p.g.project_file))
		{
			try {
				CDnaProjectFile projectFile(p.g.project_file, geoidSetting);
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

	if (!vm.count(NETWORK_NAME) &&		// User wants to populate a binary station file for a particular DynAdjust project
		!vm.count(INTERACTIVE) &&		// User wants to interpolate geoid information from the command line
		!vm.count(CREATE_NTV2) &&		// User wants to create a NTv2 grid file
		!vm.count(SUMMARY) &&			// User wants to print to the screen the details of the grid file
		!vm.count(INPUT_FILE))			// User wants to interpolate geoid information in text file mode
	{
		cout << endl << "- Nothing to do - no standard options specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for NTv2 file creation
	if (vm.count(CREATE_NTV2) && !vm.count(DAT_FILEPATH))
	{
		cout << endl << "- Error: no dat file specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for interactive mode
	if (vm.count(INTERACTIVE) && !vm.count(LATITUDE))
	{
		cout << endl << "- Error: Interpolation latitide not specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for interactive mode
	if (vm.count(INTERACTIVE) && !vm.count(LONGITUDE))
	{
		cout << endl << "- Error: Interpolation longitude not specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for text file mode
	if (!vm.count(INPUT_FILE) && !vm.count(NTV2_FILEPATH))
	{
		cout << endl << "- Error: Interpolation input file not specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for NTv2 file summary
	if (vm.count(SUMMARY) && !vm.count(NTV2_FILEPATH))
	{
		cout << endl << "- Error: No NTv2 grid file specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// check mandatory arguments for DynAdjust station file population
	if (vm.count(NETWORK_NAME) && !vm.count(NTV2_FILEPATH))
	{
		cout << endl << "- Error: No NTv2 grid file specified. " << endl << endl;
		return EXIT_FAILURE;
	}

	// binary station file location (input)
	if (vm.count(NETWORK_NAME))
	{
		p.g.project_file = formPath<string>(p.g.output_folder, p.g.network_name, "dnaproj");

		// define bst file name
		p.n.bst_file = formPath<string>(p.g.input_folder, p.g.network_name, "bst");
		p.n.file_mode = 1;
		if (!exists(p.n.bst_file))
		{
			// Look for it in the input folder
			p.n.bst_file = formPath<string>(p.g.input_folder, leafStr<string>(p.n.bst_file));

			if (!exists(p.n.bst_file))
			{
				cout << endl << "- Error: ";  
				cout << "Binary station file " << p.n.bst_file << " does not exist." << endl << endl;  
				return EXIT_FAILURE;
			}
		}
	}

	// Geoid DAT grid file file location (input)
	if (vm.count(DAT_FILEPATH))
	{
		if (!exists(p.n.rdat_geoid_file))
		{
			// Look for it in the input folder
			p.n.rdat_geoid_file = formPath<string>(p.g.input_folder, leafStr<string>(p.n.rdat_geoid_file));

			if (!exists(p.n.rdat_geoid_file))
			{
				cout << endl << "- Error: ";  
				cout << "WINTER DAT grid file " << p.n.rdat_geoid_file << " does not exist." << endl << endl;  
				return EXIT_FAILURE;
			}
		}
	}

	// Is geoid to run in file mode?
	if (vm.count(INPUT_FILE))
	{
		p.n.file_mode = 1;

		// Geoid DAT grid file file location (input)
		if (!exists(p.n.input_file))
		{
			// Look for it in the input folder
			p.n.input_file = formPath<string>(p.g.input_folder, leafStr<string>(p.n.input_file));

			if (!exists(p.n.input_file))
			{
				cout << endl << "- Error: ";  
				cout << "Input coordinates text file " << leafStr<string>(p.n.input_file) << " does not exist." << endl << endl;  
				return EXIT_FAILURE;
			}
		}
	}

	if (vm.count(EXPORT_GEO_FILE))
	{
		if (vm.count(NETWORK_NAME))
			p.n.geo_file = formPath<string>(p.g.output_folder, p.g.network_name, "geo");	// dna geoid file
		p.n.export_dna_geo_file = 1;
	}

	if (vm.count(DDEG_FORMAT))
		p.n.coordinate_format = DDEG;
	
	if (vm.count(CONVERT_BST_HT))
		p.n.convert_heights = 1;
	else
		p.n.convert_heights = 0;

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
	variables_map vm;
	positional_options_description positional_options;
	
	options_description standard_options("+ " + string(ALL_MODULE_STDOPT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description ntv2_options("+ " + string(GEOID_MODULE_NTV2), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description interpolate_options("+ " + string(GEOID_MODULE_INTERPOLATE), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description interactive_options("+ " + string(GEOID_MODULE_INTERACTIVE), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description file_interpolate_options("+ " + string(GEOID_MODULE_FILE), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description export_options("+ " + string(ALL_MODULE_EXPORT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description generic_options("+ " + string(ALL_MODULE_GENERIC), PROGRAM_OPTIONS_LINE_LENGTH);

	string cmd_line_usage("+ ");
	cmd_line_usage.append(__BINARY_NAME__).append(" usage:  ").append(__BINARY_NAME__).append(" [options]");
	options_description allowable_options(cmd_line_usage, PROGRAM_OPTIONS_LINE_LENGTH);

	n_file_par ntv2;
	ntv2.ptrIndex = new n_gridfileindex[1];
	geoid_point apInterpolant;

	project_settings p;

	string cmd_line_banner, gs_type("seconds"), version("1.0.0.0"), system_f("GDA94   "), system_t("AHD_1971");
	string subgridname("AUSGEOID"), parent(""), created(""), updated("");

	string inputLatitude, inputLongitude;
	
	fileproc_help_header(&cmd_line_banner);
	p.g.project_file = "";

	try {
		// Declare a group of options that will be 
		// allowed only on command line		
		standard_options.add_options()
			(PROJECT_FILE_P, value<string>(&p.g.project_file),
				"Project file name. Full path to project file. If none specified, a new file is created using input-folder and network-name.")
			(NETWORK_NAME_N, value<string>(&p.g.network_name), 
				string("Network name. If [" + string(NETWORK_NAME) + "].bst exists, all records within the binary station file will be populated with N value and deflections of the vertical.").c_str())
			(INPUT_FOLDER_I, value<string>(&p.g.input_folder),
				"Path containing all input files.")
			(OUTPUT_FOLDER_O, value<string>(&p.g.output_folder),		// default is ./,
				"Path for all output files.")
			;

		interpolate_options.add_options()
			(INTERACTIVE_E, "Interpolate geoid information using coordinates provided on the command line.")
			(INPUT_FILE_T, value<string>(&p.n.input_file),
				"Interpolate geoid information using a file of coordinates. arg is the path of the input text file.")
			(METHOD_M, value<UINT16>(&p.n.interpolation_method),
				"Interpolation method.\n  0  Bi-linear\n  1  Bi-cubic (default)")
			(DDEG_FORMAT, "Specify input coordinates in decimal degrees (dd.dddddd).  Default is degrees, minutes and seconds (dd.mmssss).")
			(CREATE_NTV2_C, "Create NTv2 grid file from standard DAT file.")
			(NTV2_FILEPATH_G, value<string>(&p.n.ntv2_geoid_file), "Full file path of the NTv2 grid file.")
			(SUMMARY_U, "Print a summary of the grid file.")
			;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config file        
		ntv2_options.add_options()
			(DAT_FILEPATH_D, value<string>(&p.n.rdat_geoid_file), 
				"File path of the WINTER DAT grid file.")
			(NTV2_GS_TYPE, value<string>(&gs_type),
				"Units in which the grid parameters and deflections of the vertical will be stored. arg is either 'seconds' or 'radians'. Default is seconds.")
			(NTV2_VERSION, value<string>(&version),
				"Grid file version. Default is 1.0.0.0.")
			(NTV2_SYSTEM_F, value<string>(&system_f),
				"The 'From' reference system. Default is GDA94.")
			(NTV2_SYSTEM_T, value<string>(&system_t),
				"The 'To' reference system. Default is AHD_1971")
			(NTV2_MAJOR_F, value<double>(&ntv2.daf),
				"Semi major of 'From' system. Default is 6378137.000")
			(NTV2_MAJOR_T, value<double>(&ntv2.dat),
				"Semi major of 'To' system. Default is 6378137.000")
			(NTV2_MINOR_F, value<double>(&ntv2.dbf),
				"Semi minor of 'From' system. Default is 6356752.314")
			(NTV2_MINOR_T, value<double>(&ntv2.dbt),
				"Semi minor of 'To' system. Default is 6356752.314")
			(NTV2_SUB_NAME, value<string>(&subgridname),
				"The name of the sub-grid. Default is AUSGEOID")
			(NTV2_CREATED, value<string>(&created), 
				"Date of geoid model creation. arg is a dot delimited string \"dd.mm.yyyy\". Default is today's date if no value is supplied.")
			(NTV2_UPDATED, value<string>(&updated), 
				"Date of last file update. arg is a dot delimited string \"dd.mm.yyyy\". Default is today's date if no value is supplied.")
			;

		interactive_options.add_options()
			(LATITUDE, value<string>(&inputLatitude),
				"Latitude of the interpolant. Default is degrees, minutes and seconds (dd.mmssss).")
			(LONGITUDE, value<string>(&inputLongitude),
				"Longitude of the interpolant. Default is degrees, minutes and seconds (dd.mmssss).")
			;

		file_interpolate_options.add_options()
			(DIRECTION_R, value<UINT16>(&p.n.ellipsoid_to_ortho), 
				"Conversion of heights:\n  0  Orthometric to ellispoid\n  1  Ellispoid to orthometric (default)")
			(CONVERT_BST_HT, 
				"If a user-supplied height in the binary file is orthometric, the height is converted to ellipsoidal.")
			;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config file        
		export_options.add_options()
			(EXPORT_GEO_FILE, 
				"Create a DNA geoid file from interpolated geoid information.")
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

		allowable_options.add(standard_options).add(interpolate_options).add(ntv2_options).add(interactive_options).add(file_interpolate_options).add(export_options).add(generic_options);

		// add "positional options" to handle command line tokens which have no option name
		positional_options.add(NETWORK_NAME, -1);

		command_line_parser parser(argc, argv);
		store(parser.options(allowable_options).positional(positional_options).run(), vm);
		notify(vm);
	} 
	catch (const std::exception& e) {
		cout << endl << "- Error: " << e.what() << endl;
		cout << cmd_line_banner << allowable_options << endl;
		return EXIT_FAILURE;
	}
	catch (...) 
	{
		cout << endl << "- Exception of unknown type!\n";
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
		else if (str_upper<string, char>(GEOID_MODULE_NTV2).find(help_text) != string::npos) {
			cout << ntv2_options << endl;
		}
		else if (str_upper<string, char>(GEOID_MODULE_INTERPOLATE).find(help_text) != string::npos) {
			cout << interpolate_options << endl;
		}
		else if (str_upper<string, char>(GEOID_MODULE_INTERACTIVE).find(help_text) != string::npos) {
			cout << interactive_options << endl;
		}
		else if (str_upper<string, char>(GEOID_MODULE_FILE).find(help_text) != string::npos) {
			cout << file_interpolate_options << endl;
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

	const char* const NTV2_TYPE = "gsb";
	strcpy(ntv2.filetype, NTV2_TYPE);
	
	if (ParseCommandLineOptions(argc, argv, vm, p) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	// Has the user supplied a project file?
	bool userSuppliedProjectFile(false);
	if (!p.g.project_file.empty())
		userSuppliedProjectFile = true;

	// NTv2 file path
	if (!p.n.ntv2_geoid_file.empty())
		strcpy(ntv2.filename, p.n.ntv2_geoid_file.c_str());
	else
		strcpy(ntv2.filename, (p.n.rdat_geoid_file + "." + NTV2_TYPE).c_str());
	
	if (vm.count(QUIET))
		p.g.quiet = 1;
	
	if (!p.g.quiet)
	{
		cout << endl << cmd_line_banner;

		cout << "+ Options:" << endl; 
		
		if (vm.count(NETWORK_NAME))
			cout << setw(PRINT_VAR_PAD) << left << "  Network name: " <<  p.g.network_name << endl;

		if (p.n.file_mode || vm.count(CREATE_NTV2))
		{
			cout << setw(PRINT_VAR_PAD) << left << "  Input folder: " << p.g.input_folder << endl;
			cout << setw(PRINT_VAR_PAD) << left << "  Output folder: " << p.g.output_folder << endl;

			if (!p.n.bst_file.empty())
			{
				cout << setw(PRINT_VAR_PAD) << left << "  Binary station file: " << p.n.bst_file << endl;
				cout << setw(PRINT_VAR_PAD) << left << "  Convert orthometric heights: ";
				if (p.n.convert_heights)
					cout << "Yes" << endl;
				else
					cout << "No" << endl;
			}

			if (!p.n.input_file.empty())
				cout << setw(PRINT_VAR_PAD) << left << "  ASCII file: " << p.n.input_file << endl;
		}
		
		cout << setw(PRINT_VAR_PAD) << left << "  Geoid grid file: " <<  ntv2.filename << endl;
		
		// Not applicable for project file use
		if (vm.count(CREATE_NTV2))
		{
			cout << setw(PRINT_VAR_PAD) << left << "  WINTER DAT file: " << leafStr<string>(p.n.rdat_geoid_file) << endl;

			if (vm.count(NTV2_GS_TYPE))
			{
				gs_type = trimstr(gs_type);
				// Unknown type?
				if (!iequals(gs_type, "seconds") && !iequals(gs_type, "radians"))
					gs_type = "seconds";
				str_toupper<int>(gs_type);
				cout << setw(PRINT_VAR_PAD) << left << "  Grid shift type: " << gs_type.c_str() << endl;
			}
			if (vm.count(NTV2_VERSION))
				cout << setw(PRINT_VAR_PAD) << left << "  Grid file version: " << version.c_str() << endl;
			if (vm.count(NTV2_SYSTEM_F))
				cout << setw(PRINT_VAR_PAD) << left << "  From reference system: " << system_f.c_str() << endl;
			if (vm.count(NTV2_SYSTEM_T))
				cout << setw(PRINT_VAR_PAD) << left << "  To reference system: " << system_t.c_str() << endl;
			if (vm.count(NTV2_MAJOR_F))
				cout << setw(PRINT_VAR_PAD) << left << "  From semi-major: " << fixed << setprecision(3) << ntv2.daf << endl;
			if (vm.count(NTV2_MAJOR_T))
				cout << setw(PRINT_VAR_PAD) << left << "  To semi-major: " << ntv2.dat << endl;
			if (vm.count(NTV2_MINOR_F))
				cout << setw(PRINT_VAR_PAD) << left << "  From semi-minor: " << ntv2.dbf << endl;
			if (vm.count(NTV2_MINOR_T))
				cout << setw(PRINT_VAR_PAD) << left << "  To semi-minor: " << ntv2.dbt << endl;
			if (vm.count(NTV2_SUB_NAME))
				cout << setw(PRINT_VAR_PAD) << left << "  Sub-grid name: " << subgridname.c_str() << endl;
			
			date creationDate, updateDate;
			if (created.empty())
				created = "today";
			
			if (updated.empty())
				updated = "today";
			
			creationDate = dateFromString<date>(created);
			updateDate = dateFromString<date>(updated);

			// Print dates by default
			//if (vm.count(NTV2_CREATED))
			cout << setw(PRINT_VAR_PAD) << left << "  Date of file creation: " << 
				stringFromDate<date>(creationDate, "%d %B %Y") << endl;
			//if (vm.count(NTV2_UPDATED))
			cout << setw(PRINT_VAR_PAD) << left << "  Date of file update: " << 
				stringFromDate<date>(updateDate, "%d %B %Y") << endl;

			created = stringFromDate<date>(creationDate, "%d%m%Y");
			updated = stringFromDate<date>(updateDate, "%d%m%Y");

		}

		if (p.n.file_mode || vm.count(INTERACTIVE))
		{
			cout << setw(PRINT_VAR_PAD) << left << "  Interpolation method: ";
			if (p.n.interpolation_method == BICUBIC)
				cout << "Bi-cubic" << endl;
			else
				cout << "Bi-linear" << endl;

			cout << setw(PRINT_VAR_PAD) << left << "  Input coordinate format: ";
			if (p.n.coordinate_format == DDEG)
				cout << "Decimal degrees" << endl;
			else
				cout << "Degrees minutes seconds" << endl;
		}

		if (p.n.export_dna_geo_file)
			cout << setw(PRINT_VAR_PAD) << left << "  Export to DNA geoid file: " << "Yes" << endl;

		cout << endl;
	
		if (p.n.file_mode)
		{
			if (!p.n.bst_file.empty())
				cout << "+ Binary station file interpolation mode." << endl << endl;
			else
				cout << "+ ASCII file interpolation mode." << endl << endl;
		}

		// Not applicable for project file use
		else if (vm.count(CREATE_NTV2))
			cout << "+ Creating NTv2 file from WINTER DAT file format:" << endl;
	}

	dna_geoid_interpolation g;
	
	// Not applicable for project file use
	if (vm.count(CREATE_NTV2))
	{
		if (vm.count(NTV2_GS_TYPE))
			strcpy(ntv2.chGs_type, gs_type.c_str());
		if (vm.count(NTV2_VERSION))
			strcpy(ntv2.chVersion, version.c_str());
		if (vm.count(NTV2_SYSTEM_F))
			strcpy(ntv2.chSystem_f, system_f.c_str());
		if (vm.count(NTV2_SYSTEM_T))
			strcpy(ntv2.chSystem_t, system_t.c_str());
		
		// subgrid header string handlers
		if (vm.count(NTV2_SUB_NAME))
			strcpy(ntv2.ptrIndex[0].chSubname, subgridname.c_str());
		strcpy(ntv2.ptrIndex[0].chParent, "NONE    ");		// don't give the user the option - must be "NONE    " !!!
		strcpy(ntv2.ptrIndex[0].chCreated, created.c_str());
		strcpy(ntv2.ptrIndex[0].chUpdated, updated.c_str());

		// Create a new grid file from AusGeoid DAT file
		if (!CreateNTv2Grid(&g, p.n.rdat_geoid_file.c_str(), &ntv2))
		{
			if (ntv2.ptrIndex)
				delete [] ntv2.ptrIndex;
			return EXIT_FAILURE;
		}

		if (ntv2.ptrIndex)
			delete [] ntv2.ptrIndex;

		return EXIT_SUCCESS;
	}

	// Not applicable for project file use
	if (vm.count(SUMMARY))
	{
		// Open the grid file and print its properties
		if (!reportGridProperties(&g, ntv2.filename, ntv2.filetype))
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	// Not applicable for project file use
	if (vm.count(INTERACTIVE))
	{
		apInterpolant.cVar.dLatitude = DoubleFromString<double>(inputLatitude);
		apInterpolant.cVar.dLongitude = DoubleFromString<double>(inputLongitude);

		stringstream ssInput;
		ssInput << inputLatitude << ", " << inputLongitude;
		g.SetInputCoordinates(ssInput.str());

		if  (p.n.coordinate_format == DMS)
		{
			apInterpolant.cVar.dLatitude = DmstoDeg<double>(apInterpolant.cVar.dLatitude);
			apInterpolant.cVar.dLongitude = DmstoDeg<double>(apInterpolant.cVar.dLongitude);
		}

		// Get geoid values for a point
		if (!createGridIndex(&g, ntv2.filename, ntv2.filetype, p.g.quiet))
			return EXIT_FAILURE;

		if (!InterpolateGridPoint(&g, ntv2.filename, &apInterpolant, 
			p.n.interpolation_method, p.n.coordinate_format, inputLatitude, inputLongitude))
			return EXIT_FAILURE;
		
		return EXIT_SUCCESS;
	}
	 
	// Should a file be processed?
	if (!p.n.file_mode)
		return EXIT_SUCCESS;
	
	// Get geoid values for a point
	if (!createGridIndex(&g, ntv2.filename, ntv2.filetype, p.g.quiet))
		return EXIT_FAILURE;

	if (!p.g.quiet)
	{
		cout << "+ Interpolating geoid components";
		if (!p.n.bst_file.empty() && p.n.convert_heights)
			cout << " and reducing" << endl <<
				"  heights to the ellipsoid";
		cout << "... ";
	}
	
	cpu_timer time;

	char dnageoFile[601], *geoFileptr;
	geoFileptr = NULL;
	memset(dnageoFile, '\0', sizeof(dnageoFile));
	string outputfilePath;
		
	if (!p.n.geo_file.empty())
	{
		sprintf(dnageoFile, "%s", p.n.geo_file.c_str());
		geoFileptr = dnageoFile;
	}
		
	if (!p.n.bst_file.empty())
	{
		// populate binary station file with geoid separation and deflections
		if (!InterpolateGridBinaryStationFile(&g, p.n.bst_file.c_str(), p.n.interpolation_method, 
			p.n.convert_heights ? true : false, 
			p.n.export_dna_geo_file? true : false, geoFileptr))
				return EXIT_FAILURE;
	}
	else if (!p.n.input_file.empty())
	{
		// Ascii text file (similar to old winter format)
		if (!InterpolateGridPointFile(&g, p.n.input_file.c_str(), p.n.interpolation_method, 
			p.n.ellipsoid_to_ortho, p.n.coordinate_format,
			p.n.export_dna_geo_file? true : false, geoFileptr,
			outputfilePath))
				return EXIT_FAILURE;
	}
	else
		return EXIT_SUCCESS;
		
	if (!p.g.quiet)
	{
		cout << "done." << endl;
		cout << "+ Interpolated data for " << g.PointsInterpolated();
		if (g.PointsInterpolated() == 1)
			cout << " point." << endl;
		else
			cout << " points." << endl;

		if (g.PointsNotInterpolated() > 0)
		{
			cout << "- Warning: Data for " << g.PointsNotInterpolated();

			// Is this wrapper being called to update DynAdjust station file?
			if (!p.n.bst_file.empty())
			{
				if (g.PointsNotInterpolated() == 1)
					cout << " station";
				else
					cout << " stations";

				cout << " could not be interpolated";
					
				if (p.g.verbose > 0)
				{
					cout << ":" << endl;
					cout << g.ReturnBadStationRecords();
				}
				else
					cout << ".  To view the list of stations " << endl <<
						"  for which a height could not be interpolated, call " << __BINARY_NAME__ << " with --verbose-level 1." << endl;
			}
			// If this point is reached, then this wrapper must have been called
			// to interpolate points in interactive mode.
			else //if (!p.n.input_file.empty())
			{
				if (g.PointsNotInterpolated() == 1)
					cout << " point";
				else
					cout << " points";

				cout << " could not be interpolated." << endl;
				cout << "  See " << outputfilePath << " for more information." << endl;
			}
		}
	}

	// Look for a project file (if the user has specified a network name.  
	// If it exists, open it, load it and update the geoid settings.
	if (userSuppliedProjectFile)
	{
		CDnaProjectFile projectFile;
		if (exists(p.g.project_file))
			projectFile.LoadProjectFile(p.g.project_file);

		// Print the project file. If it doesn't exist, it will be created.
		projectFile.UpdateSettingsGeoid(p);
		projectFile.PrintProjectFile();
	}

	if (p.g.quiet)
		return EXIT_SUCCESS;

	// wall time is in nanoseconds
	// cout << time.elapsed().wall << endl << endl;
	milliseconds elapsed_time(milliseconds(time.elapsed().wall/MILLI_TO_NANO));
	cout << endl << formatedElapsedTime<string>(&elapsed_time, "+ Geoid file interpolation took ") << endl << endl;
	
	return EXIT_SUCCESS;
}