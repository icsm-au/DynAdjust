//============================================================================
// Name         : dnaplotwrapper.cpp
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
// Description  : DynAdjust Plot Executable
//============================================================================

#include <dynadjust/dnaplotwrapper/dnaplotwrapper.hpp>

void ProcessGnuPlot(dna_plot* plotDynaML, project_settings& p, plotGraphMode mode)
{
	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "+ Preparing the "; 
		switch (mode)
		{
		case StationsMode:
			cout << "stations";
			break;
		case MeasurementsMode:
			cout << "measurements";
			break;
		}
		cout << " histogram via gnuplot... ";
		cout.flush();
	}
	
	plotDynaML->CreategnuplotGraphEnvironment(&p, mode);

	if (p.g.verbose > 0 || p.g.quiet != 1)
		cout << "done." << endl;

	// if option --supress-pdf-creation is provided to dnaplotwrapper, then don't
	// execute the bat/shell scripts to call GMT or gnuplot.
	if (p.p._supress_pdf_creation)
	{
		if (p.g.verbose > 0 || p.g.quiet != 1)
		{
			cout << "+ The input data files and " <<
#if defined(_WIN32) || defined(__WIN32__)
			"batch file";
#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
			"shell script";
#endif
			cout << " to create the final PDF histogram" << endl << 
				"  via gnuplot have been generated." << endl;
			cout << "+ To configure the histogram's font sizes, colours, scale, annotations, etc.," << endl <<
				"  edit the " <<
#if defined(_WIN32) || defined(__WIN32__)
			"batch file";
#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
			"shell script";
#endif
			cout << " directly and execute as normal." << endl << endl;
		}

		return;
	}

	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "+ Creating the PDF... ";
		cout.flush();		
	}

	// Invoke gnuplot to create the PDF plot.
	// if option --supress-pdf-creation is provided to dnaplotwrapper, then don't
	// execute the bat/shell scripts to call gnuplot.
	plotDynaML->CreateSegmentationGraph();

	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "done." << endl << endl;
		cout.flush();
	}
}

void ProcessGMTPlot(dna_plot* plotDynaML, project_settings& p, string& measurement_types)
{
	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "+ Preparing shell script and data files in GMT format... ";
		cout.flush();
	}	

	p.p._gmt_params.clear();
	p.p._gmt_params.push_back(string_string_pair("MAP_GRID_PEN_PRIMARY", "0.5p,128/128/128"));
	p.p._gmt_params.push_back(string_string_pair("MAP_FRAME_TYPE", "fancy"));
	p.p._gmt_params.push_back(string_string_pair("PS_MEDIA", "A0"));
	p.p._gmt_params.push_back(string_string_pair("MAP_FRAME_PEN", "1p"));
	
	p.p._separate_msrs.clear();

	if (!measurement_types.empty())
	{
		str_toupper<int>(measurement_types);
		for_each(measurement_types.begin(), measurement_types.end(),
			[&p] (char& m) { p.p._separate_msrs.push_back(m); });
	}
	else
	{
		// the order of measurement types in _separate_msrs determines the order of measurement layers
		// last is on top

		// GPS point clusters
		p.p._separate_msrs.push_back('Y');		// Large circles (larger than P, R, Q)

		// astronomic azimuth
		p.p._separate_msrs.push_back('K');
		
		// position and height measurements
		// These measurements are printed as circles, the diameter of which is 
		// a multiple of the station circle diameter (as follows)
		p.p._separate_msrs.push_back('I');		// Circle diameter is 3.0 * station circle diameter
		p.p._separate_msrs.push_back('P');		// ''
		p.p._separate_msrs.push_back('J');		// Circle diameter is 2.4 * station circle diameter
		p.p._separate_msrs.push_back('Q');		// ''
		p.p._separate_msrs.push_back('H');		// Circle diameter is 1.7 * station circle diameter
		p.p._separate_msrs.push_back('R');		// ''
		
		// verticals, zeniths, angles, geodetic azimuths, directions
		p.p._separate_msrs.push_back('Z');
		p.p._separate_msrs.push_back('V');
		p.p._separate_msrs.push_back('A');
		p.p._separate_msrs.push_back('B');
		p.p._separate_msrs.push_back('D');
		
		// diff heights
		p.p._separate_msrs.push_back('L');
		
		// distances
		p.p._separate_msrs.push_back('S');
		p.p._separate_msrs.push_back('M');
		p.p._separate_msrs.push_back('E');
		p.p._separate_msrs.push_back('C');
		
		// GPS baselines
		p.p._separate_msrs.push_back('G');
		p.p._separate_msrs.push_back('X');
	}

	// prepare the schell scripts and data files
	plotDynaML->CreateGMTPlotEnvironment(&p);
	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "done." << endl << endl;

		cout << setw(PRINT_VAR_PAD) << "+ Plot details:" << endl;
		cout << setw(PRINT_VAR_PAD) << "  PDF file name:" << p.p._pdf_file_name << endl;
		if (!measurement_types.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  Measurement types: " << measurement_types << endl;
		if (p.p._plot_station_labels)
			cout << setw(PRINT_VAR_PAD) << "  Label stations:" << "Yes" << endl;
		if (p.p._plot_alt_name)
			cout << setw(PRINT_VAR_PAD) << "  Use alternate name:" << "Yes" << endl;
		if (p.p._plot_station_constraints)
			cout << setw(PRINT_VAR_PAD) << "  Label constraints:" << "Yes" << endl;
		if (p.p._plot_correction_arrows)
			cout << setw(PRINT_VAR_PAD) << "  Plot correction arrows:" << "Yes" << endl;
		if (p.p._plot_correction_labels)
			cout << setw(PRINT_VAR_PAD) << "  Label corrections:" << "Yes" << endl;
		if (p.p._plot_error_ellipses)
			cout << setw(PRINT_VAR_PAD) << "  Plot error ellipses:" << "Yes" << endl;
		if (p.p._plot_positional_uncertainty)
			cout << setw(PRINT_VAR_PAD) << "  Plot positional uncertainty:" << "Yes" << endl;
		
		if (!are_floats_equal(p.p._correction_scale, 1.))
			cout << setw(PRINT_VAR_PAD) << left << "  Scale correction arrows: " << fixed << setprecision(3) << p.p._correction_scale << endl;
		if (!are_floats_equal(p.p._pu_ellipse_scale, 1.))
			cout << setw(PRINT_VAR_PAD) << left << "  Scale uncertainties/ellipses: " << fixed << setprecision(3) << p.p._pu_ellipse_scale << endl;
				
		if (!p.p._bounding_box.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  Bounding box: " << p.p._bounding_box << endl;
		else
		{
			if (!p.p._plot_station_centre.empty())
				cout << setw(PRINT_VAR_PAD) << "  Centre on Station:" << p.p._plot_station_centre << endl;
			else
			{
				if (!are_floats_equal(p.p._plot_centre_latitude, -999.))
					cout << setw(PRINT_VAR_PAD) << "  Centre latitude:" << fixed << setprecision(9) << FormatDmsString(DegtoDms(p.p._plot_centre_latitude), 4, true, true) << endl;
				if (!are_floats_equal(p.p._plot_centre_longitude, -999.))
					cout << setw(PRINT_VAR_PAD) << "  Centre longitude:" << fixed << setprecision(9) << FormatDmsString(DegtoDms(p.p._plot_centre_longitude), 4, true, true) << endl;
			}
			if (!are_floats_equal(p.p._plot_area_radius, 5000.))
				cout << setw(PRINT_VAR_PAD) << "  Area radius:" << fixed << setprecision(2) << p.p._plot_area_radius << " metres" << endl;
		}
		cout << setw(PRINT_VAR_PAD) << "  Map projection:" << projectionTypes[p.p._projection] << endl;
		cout << setw(PRINT_VAR_PAD) << "  Ground width:" << fixed << setprecision(4) << p.p._ground_width / 1000. << " kilometres" << endl;
		cout << setw(PRINT_VAR_PAD) << "  Page width:" << fixed << setprecision(2) << p.p._page_width << " centimetres" << endl;
		cout << setw(PRINT_VAR_PAD) << "  Coastline resolution:" << coastResolutionTypes[p.p._coasline_resolution] << endl;
		cout << setw(PRINT_VAR_PAD) << "  Scale:" << "1:" << fixed << setprecision(0) << p.p._plot_scale << endl;					
		cout << setw(PRINT_VAR_PAD) << "  Label font size:" << fixed << setprecision(0) << p.p._label_font_size << endl;
		
		cout << endl;
	}

	// Does the user not want to create the PDf plots?
	if (p.p._supress_pdf_creation)
	{
		if (p.g.verbose > 0 || p.g.quiet != 1)
		{
			cout << "+ The input data files and " <<
#if defined(_WIN32) || defined(__WIN32__)
			"batch file";
#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
			"shell script";
#endif
			if (p.p._plot_phased_blocks && p.p._plot_block_number < 1)
				cout << "s";

			cout << " to create the final PDF plot" << endl << "  via GMT have been generated." << endl;
			cout << "+ To configure the plot's font sizes, colours, scale, annotations, etc.," << endl <<
				"  edit the " <<
#if defined(_WIN32) || defined(__WIN32__)
			"batch file";
#elif defined(__linux) || defined(sun) || defined(__unix__) || defined(__APPLE__)
			"shell script";
#endif
			if (p.p._plot_phased_blocks && p.p._plot_block_number < 1)
				cout << "s";
			cout << " directly and execute as normal." << endl << endl;
		}

		return;
	}

	// create the PDF image..
	if (p.g.verbose > 0 || p.g.quiet != 1)
	{					
		cout << "+ Generating PDF via GMT... ";
		cout.flush();
	}

	// Invoke concurrent GMT process to create the PDF plot.
	// If phased, plotDynaML will aggregate all PDFs into a single
	// plot.
	//
	// if option --supress-pdf-creation is provided to dnaplotwrapper, then don't
	// execute the bat/shell scripts to call GMT.
	plotDynaML->CreateGMTPlot();

	if (p.g.verbose > 0 || p.g.quiet != 1)
	{
		cout << "done." << endl << endl <<
			"+ Open " << p.p._pdf_file_name << " to view the final PDF plot." << endl << endl;
		cout.flush();
	}

}
	
int main(int argc, char* argv[])
{
	// create banner message
	string cmd_line_banner, tmp;
	fileproc_help_header(&cmd_line_banner);

	string stnfilename, msrfilename, measurement_types;
	
	project_settings p;
	
	variables_map vm;
	positional_options_description positional_options;
	
	options_description standard_options("+ " + string(ALL_MODULE_STDOPT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description config_options("+ " + string(PLOT_MODULE_CONFIG), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description pdfviewer_options("+ " + string(PLOT_MODULE_PDFVIEWER), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description map_options("+ " + string(PLOT_MODULE_MAP), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description generic_options("+ " + string(ALL_MODULE_GENERIC), PROGRAM_OPTIONS_LINE_LENGTH);

	tmp = "+ ";
	tmp.append(__BINARY_NAME__).append(" usage:  ").append(__BINARY_NAME__).append(" ").append(NETWORK_NAME).append(" [options]");
	options_description allowable_options(tmp, PROGRAM_OPTIONS_LINE_LENGTH);

	try {
		// Declare a group of options that will be 
		// allowed only on command line		
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

		standard_options.add_options()
			(PROJECT_FILE_P, value<string>(&p.g.project_file),
				"Project file name. Full path to project file. If none specified, a new file is created using input-folder and network-name.")
			(NETWORK_NAME_N, value<string>(&p.g.network_name), 
				"Network name. User defined name for all output files. Default is \"network#\".")
			(INPUT_FOLDER_I, value<string>(&p.g.input_folder),
				"Path containing all input files")
			(OUTPUT_FOLDER_O, value<string>(&p.g.output_folder),		// default is ./,
				"Path for all output files")
			(BIN_STN_FILE, value<string>(&p.i.bst_file),
				"Binary station file name. Overrides network name.")
			(BIN_MSR_FILE, value<string>(&p.i.bms_file),
				"Binary measurement file name. Overrides network name.")
			;

		// Declare a group of options that will be 
		// allowed both on command line and in
		// config file        
		config_options.add_options()
			(PLOT_MSRS, value<string>(&measurement_types),
				"Plot the specified measurement types. arg is a non-delimited string of measurement types (eg \"GXY\").")
			(PLOT_MSRS_IGNORED,
				"Plot ignored measurements.")
			(PLOT_BLOCKS,
				"Plot the blocks of a segmented network in individual sheets.  Requires a corresponding segmentation file.")
			(SEG_FILE, value<string>(&p.s.seg_file),
				"Network segmentation file. Filename overrides network name.")
			(BLOCK_NUMBER, value<UINT32>(&p.p._plot_block_number),
				"When plotting phased adjustments, plot this block only. Zero (default) plots all blocks.")
			(PLOT_STN_LABELS,
				"Plot the station labels.")
			(PLOT_ALT_NAME,
				"Plot alternate station names.")
			(PLOT_CONSTRAINT_LABELS,
				"Plot the station constraints.")
			(PLOT_CORRECTION_ARROWS,
				"Plot arrows representing the direction and magnitude of corrections to the station coordinates.")
			(PLOT_CORRECTION_LABELS,
				"Plot correction labels.")
			(OMIT_MEASUREMENTS,
				"Do not print any measurements.")
			(COMPUTE_CORRECTIONS,
				"Compute corrections from binary station file.")
			(PLOT_ERROR_ELLIPSES,
				"Plot error ellipses.")
			(PLOT_POSITIONAL_UNCERTAINTY,
				"Plot positional uncertainty.")
			(CORRECTION_SCALE, value<double>(&p.p._correction_scale),
				"The amount by which to scale the size of the correction arrows.")
			(PU_ELLIPSE_SCALE, value<double>(&p.p._pu_ellipse_scale),
				"The amount by which to scale the size of error ellipses and positional uncertainty cirlces.")
			(BOUNDING_BOX, value<string>(&p.p._bounding_box),
				"Plot stations and measurements within bounding box. arg is a comma delimited string \"lat1,lon1,lat2,lon2\" (in dd.mmss) defining the upper-left and lower-right limits.")
			(PLOT_CENTRE_LAT, value<double>(&p.p._plot_centre_latitude),
				"Centre the plot according to this latitude. Format: dd.mmsssss")
			(PLOT_CENTRE_LON, value<double>(&p.p._plot_centre_longitude),
				"Centre the plot according to this longitude. Format: ddd.mmsssss")
			(PLOT_CENTRE_STATION, value<string>(&p.p._plot_station_centre),
				(string("The station name upon which to centre the plot. The plot area is circumscribed by ")+
				string(PLOT_AREA_RADIUS)+string(".")).c_str())
			(PLOT_AREA_RADIUS, value<double>(&p.p._plot_area_radius),
				(string("The radius (in metres) of an area to bound the plot.  Default is ")+
				StringFromT(p.p._plot_area_radius)+string("m")).c_str())
			(GRAPH_SEGMENTATION_STNS, 
				"Plot a graph of the block stations resulting from network segmentation.")
			(GRAPH_SEGMENTATION_MSRS, 
				"Plot a graph of the block measurements resulting from network segmentation.")
			(KEEP_FILES,
				"Don't delete command and data files used to generate EPS and PDF plots.")
			;

		pdfviewer_options.add_options()
			(PDF_VIEWER, value<string>(&p.p._pdf_viewer),
				(string("The application to use when displaying PDF files.  Default is ")+
				StringFromT(p.p._pdf_viewer)+string(".")).c_str())
			(ACROBAT_DDENAME, value<string>(&p.p._acrobat_ddename),
				(string("When using Adobe Acrobat Reader, use this DDE message.  Default is ")+
				StringFromT(p.p._acrobat_ddename)+string(".")).c_str())
			;

		// mapping options
		map_options.add_options()
			//	0 Allow plot to determine the projection from the data spatial extents
			//	1 World plot
			//	2 Orthographic (globe plot)
			//	3 Mercator
			//	4 Transverse Mercator
			//	5 Albers conic equal-area
			//	6 Lambert conformal
			//  7 General stereographic
			(PROJECTION, value<UINT16>(&p.p._projection),
				string("Map projection type.\n").
				append("  0: Let ").append(__BINARY_NAME__).append(" choose best projection (default)\n").
				append("  1: ").append(projectionTypes[1]).append("\n").
				append("  2: ").append(projectionTypes[2]).append("\n").
				append("  3: ").append(projectionTypes[3]).append("\n").
				append("  4: ").append(projectionTypes[4]).append("\n").
				append("  5: ").append(projectionTypes[5]).append("\n").
				append("  6: ").append(projectionTypes[6]).append("\n").
				append("  7: ").append(projectionTypes[7]).c_str())
			(LABEL_FONT_SIZE, value<double>(&p.p._label_font_size),
				(string("Label font size for station names and constraints.  Default is ")+
				StringFromT(p.p._label_font_size)+string(".")).c_str())
			(MSR_LINE_WIDTH, value<double>(&p.p._msr_line_width),
				(string("Measurement line width.  Default is ")+
				StringFromT(p.p._msr_line_width)+string(".")).c_str())
			(OMIT_TITLE_BLOCK,
				"Do not print a title block and measurements legend.")
			(USE_PDFLATEX,
				"Use PdfLaTeX to create the PDF (available only if a suitable TeX implementation is installed). Default pdf creation uses ps2pdf (invokes Ghostscript).  PdfLaTeX may help resolve problems associated with creating large landscape plots.")
			(DONT_CREATE_PDF,
				"Don't create a pdf, just the command files.")
			;

		allowable_options.add(standard_options).add(config_options).add(map_options).add(pdfviewer_options).add(generic_options);

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
		else if (str_upper<string, char>(PLOT_MODULE_CONFIG).find(help_text) != string::npos) {
			cout << config_options << endl;
		}
		else if (str_upper<string, char>(PLOT_MODULE_PDFVIEWER).find(help_text) != string::npos) {
			cout << pdfviewer_options << endl;
		}
		else if (str_upper<string, char>(PLOT_MODULE_MAP).find(help_text) != string::npos) {
			cout << map_options << endl;
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

	if (vm.count(QUIET))
		p.g.quiet = 1;
	else
		cout << endl << cmd_line_banner;

	if (!vm.count(NETWORK_NAME))
	{
		cout << endl << "- Nothing to do - no network name specified. " << endl << endl;  
		return EXIT_FAILURE;
	}

	// capture command line arguments
	for (int cmd_arg(0); cmd_arg<argc; ++cmd_arg)
	{
		 p.p.command_line_arguments += argv[cmd_arg];
		 p.p.command_line_arguments += " ";
	}

	// the network name
	if (vm.count(NETWORK_NAME))
		p.g.network_name = p.g.network_name;

	// use pdflatex tool (discard???)
	if (vm.count(USE_PDFLATEX))
		p.p._use_pdflatex = true;

	// plot ignored measurements?
	if (vm.count(PLOT_MSRS_IGNORED))
		p.p._plot_ignored_msrs = true;

	// Segmented network view?
	if (vm.count(PLOT_BLOCKS) || vm.count(BLOCK_NUMBER))
		p.p._plot_phased_blocks = true;

	// It doesn't make sense to plot a network, centred on a station or point, or
	// within a bounding box in phased mode, so swich phased off in these cases.
	if (vm.count(BOUNDING_BOX) || 
		vm.count(PLOT_CENTRE_LAT) || vm.count(PLOT_CENTRE_LON) ||
		vm.count(PLOT_CENTRE_STATION))
		p.p._plot_phased_blocks = false;

	// graph mode?
	bool graph_mode(false);
	bool graph_mode_stns(false);
	bool graph_mode_msrs(false);
	if (vm.count(GRAPH_SEGMENTATION_STNS) || vm.count(GRAPH_SEGMENTATION_MSRS))
	{
		if (vm.count(GRAPH_SEGMENTATION_STNS))
			graph_mode_stns = true;
		if (vm.count(GRAPH_SEGMENTATION_MSRS))
			graph_mode_msrs = true;
		p.p._plot_phased_blocks = true;
		graph_mode = true;
	}

	// correction arrows?
	if (vm.count(PLOT_CORRECTION_ARROWS))
		p.p._plot_correction_arrows = true;

	// compute corrections, or use corrections file
	if (vm.count(COMPUTE_CORRECTIONS))
		p.p._compute_corrections = true;

	// correction labels
	if (vm.count(PLOT_CORRECTION_LABELS))
		p.p._plot_correction_labels = true;

	// plot error ellipses?
	if (vm.count(PLOT_ERROR_ELLIPSES))
		p.p._plot_error_ellipses = true;

	// plot positional uncertainty?
	if (vm.count(PLOT_POSITIONAL_UNCERTAINTY))
		p.p._plot_positional_uncertainty = true;

	// has the user specified a projection
	if (vm.count(PROJECTION) && p.p._projection > 0)
	{
		p.p._user_defined_projection = true;
		if (p.p._projection > 7)
			p.p._projection = bestProjection;			
	}

	if (vm.count(OMIT_TITLE_BLOCK))
		p.p._omit_title_block = true;

	if (vm.count(OMIT_MEASUREMENTS))
		p.p._omit_measurements = true;

	if (vm.count(KEEP_FILES))
		p.p._keep_gen_files = true;

	if (vm.count(DONT_CREATE_PDF))
	{
		p.p._supress_pdf_creation = true;
		// to supress creation of PDF implies keeping the 
		// files to generate the plot manually
		p.p._keep_gen_files = true;
	}

	//////////////////////////////////////////////////////
	// Form paths

	// binary station file
	if (!p.i.bst_file.empty())
		p.i.bst_file = formPath<string>(p.g.input_folder, p.i.bst_file);
	else
		p.i.bst_file = formPath<string>(p.g.input_folder, p.g.network_name, "bst");

	// binary measurement file
	if (!p.i.bms_file.empty())
		p.i.bms_file = formPath<string>(p.g.input_folder, p.i.bms_file);
	else
		p.i.bms_file = formPath<string>(p.g.input_folder, p.g.network_name, "bms");

	// station map file
	p.i.map_file = formPath<string>(p.g.input_folder, p.g.network_name, "map");

	// segmentation file
	if (p.p._plot_phased_blocks)
	{
		if (vm.count(SEG_FILE))
			p.s.seg_file = formPath<string>(p.g.input_folder, p.s.seg_file);
		else
			p.s.seg_file = formPath<string>(p.g.input_folder, p.g.network_name, "seg");
	}

	// correction file
	if (p.p._plot_correction_arrows)
	{
		if (p.p._plot_phased_blocks)
			p.o._cor_file = formPath<string>(p.g.output_folder, p.g.network_name, "phased.cor");
		else
			p.o._cor_file = formPath<string>(p.g.output_folder, p.g.network_name, "simult.cor");
	}

	// adjusted positional uncertainty file
	if (p.p._plot_error_ellipses || p.p._plot_positional_uncertainty)
	{
		if (p.p._plot_phased_blocks)
			p.o._apu_file = formPath<string>(p.g.output_folder, p.g.network_name, "phased.apu");
		else
			p.o._apu_file = formPath<string>(p.g.output_folder, p.g.network_name, "simult.apu");
	}
	//
	//////////////////////////////////////////////////////

	if (p.g.quiet != 1)
	{	
		cout << endl << "+ Options:" << endl; 
		cout << setw(PRINT_VAR_PAD) << left << "  Network name: " <<  p.g.network_name << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Input folder: " << p.g.input_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Output folder: " << p.g.output_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary station file: " << p.i.bst_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary measurement file: " << p.i.bms_file << endl;
		if (p.p._plot_phased_blocks)
			cout << setw(PRINT_VAR_PAD) << left << "  Segmentation file: " << p.s.seg_file << endl;
		if (p.p._plot_correction_arrows && !p.p._compute_corrections)
			cout << setw(PRINT_VAR_PAD) << left << "  Corrections file: " << p.o._cor_file << endl;
		if (p.p._plot_error_ellipses || p.p._plot_positional_uncertainty)
			cout << setw(PRINT_VAR_PAD) << left << "  Positional uncertainty file: " << p.o._apu_file << endl;
		cout << endl;
	}
	

	if (!exists(p.i.bst_file) || !exists(p.i.bms_file))
	{
		cout << endl << endl << "- Nothing to do: network";
		if (!vm.count(NETWORK_NAME))
			cout << " name has not been specified, and" << endl << " ";
		
		cout << " files " << p.i.bst_file << " and " << p.i.bms_file << " do not exist." << endl << endl;  
		return EXIT_FAILURE;
	}

	if (p.p._plot_correction_arrows)
	{
		if (!p.p._compute_corrections)
		{
			if (!exists(p.o._cor_file))
			{
				cout << endl << endl << 
					"- Error: The required corrections file does not exist:" << endl;  
				cout << "         " << p.o._cor_file << endl << endl;
				cout << "  Run:  'adjust " << p.g.network_name << " --" << OUTPUT_STN_COR_FILE;
				if (p.p._plot_phased_blocks)
					cout << " --" << MODE_PHASED << "'";
				else
					cout << "'";
				cout << endl << "  to create a corrections file" << endl << endl;
				return EXIT_FAILURE;
			}
		}
	}

	if (p.p._plot_error_ellipses || p.p._plot_positional_uncertainty)
	{
		if (!exists(p.o._apu_file))
		{
			cout << endl << endl << 
				"- Error: The required positional uncertainty file does not exist:" << endl;  
			cout << "         " << p.o._apu_file << endl << endl;
			cout << "  Run:  '" << __adjust_app_name__ << " " << p.g.network_name << " --" << OUTPUT_POS_UNCERTAINTY;
			if (p.p._plot_phased_blocks)
				cout << " --" << MODE_PHASED << "'";
			else
				cout << "'";
			cout << endl << "  to create a positional uncertainty file" << endl << endl;
			return EXIT_FAILURE;
		}
	}

	// check existence of segmentation file - mandatory for:
	//  - plots of segmented network plots (generated by GMT)
	//  - graphs (generated by gnuplot)
	//
	if (p.p._plot_phased_blocks || graph_mode)
	{
		if (!exists(p.s.seg_file))
		{
			cout << endl << endl << 
				"- Error: The required segmentation file does not exist:" << endl;  
			cout << "         " << p.s.seg_file << endl << endl;
			cout << "  Run  '" << __segment_app_name__ << " " << p.g.network_name << "' to create a segmentation file" << endl << endl;
			return EXIT_FAILURE;
		}

		if (last_write_time(p.s.seg_file) < last_write_time(p.i.bst_file) ||
			last_write_time(p.s.seg_file) < last_write_time(p.i.bms_file))
		{
			if (!vm.count(SEG_FILE))
			{
				// Has import been run after the segmentation file was created?
				binary_file_meta_t bst_meta, bms_meta;
				dna_io_bst bst;
				dna_io_bms bms;
				bst.load_bst_file_meta(p.i.bst_file, bst_meta);
				bms.load_bms_file_meta(p.i.bms_file, bms_meta);

				bool bst_meta_import(iequals(bst_meta.modifiedBy, __import_app_name__) ||
					iequals(bst_meta.modifiedBy, __import_dll_name__));
				bool bms_meta_import(iequals(bms_meta.modifiedBy, __import_app_name__) ||
					iequals(bms_meta.modifiedBy, __import_dll_name__));

				if ((bst_meta_import && (last_write_time(p.s.seg_file) < last_write_time(p.i.bst_file))) || 
					(bms_meta_import && (last_write_time(p.s.seg_file) < last_write_time(p.i.bms_file))))
				{
					cout << endl << endl << 
						"- Error: The binary station and measurement files have been modified since" << endl <<
						"  the segmentation file was created:" << endl;

					time_t t_bst(last_write_time(p.i.bst_file)), t_bms(last_write_time(p.i.bms_file));
					time_t t_seg(last_write_time(p.s.seg_file));

					cout << "   " << leafStr<string>(p.i.bst_file) << "  last modified on  " << ctime(&t_bst);
					cout << "   " << leafStr<string>(p.i.bms_file) << "  last modified on  " << ctime(&t_bms) << endl;
					cout << "   " << leafStr<string>(p.s.seg_file) << "  created on  " << ctime(&t_seg) << endl;
					cout << "  Run '" << __segment_app_name__ << " " << p.g.network_name << " [options]' to re-create the segmentation file, or re-run" << endl << 
						"  the plot using the " << SEG_FILE << " option if this segmentation file must\n  be used." << endl << endl;
					return EXIT_FAILURE;
				}
			}
		}
	}

	try {

		if (p.g.verbose > 0 || p.g.quiet != 1)
		{
			cout << "+ Loading network files... ";
			cout.flush();
		}
			
		dna_plot plotDynaML;
		plotDynaML.LoadNetworkFiles(p);
		
		if (p.g.verbose > 0 || p.g.quiet != 1)
		{
			cout << "done." << endl << endl;
			cout.flush();			
		}

		// set up colours for each measurement
		p.p._msr_colours.clear();

		// Add colours.
		// http://w3schools.com/tags/ref_colorpicker.asp
		// see http://www.december.com/html/spec/colorhex.html
		// or http://www.yellowpipe.com/yis/tools/hex-to-rgb/color-converter.php
		// or http://www.easycalculation.com/color-coder.php
		// see U:\vs9\projects\geodesy\doc\colorhex.pdf
		
		p.p._msr_colours.push_back(string_string_pair("A", "#FFC07F"));		// mellow apricot, #FFC07F
		p.p._msr_colours.push_back(string_string_pair("B", "#FE5F55"));		// orange red crayola, #FE5F55
		p.p._msr_colours.push_back(string_string_pair("C", "#A393BF"));		// glossy grape, #A393BF
		p.p._msr_colours.push_back(string_string_pair("D", "#DA5552"));		// indian red, #DA5552
		p.p._msr_colours.push_back(string_string_pair("E", "#717568"));		// nickel, #717568
		p.p._msr_colours.push_back(string_string_pair("G", "#5AA9E6"));		// blue jeans, #5AA9E6
		p.p._msr_colours.push_back(string_string_pair("H", "#DA627D"));		// blush, #DA627D
		p.p._msr_colours.push_back(string_string_pair("I", "#A9F0D1"));		// magic mint, #A9F0D1
		p.p._msr_colours.push_back(string_string_pair("J", "#9AD4D6"));		// powder blue, #9AD4D6
		p.p._msr_colours.push_back(string_string_pair("K", "#FFA07A"));		// lightsalmon, #FFA07A
		p.p._msr_colours.push_back(string_string_pair("L", "#43AA8B"));		// zomp, #43AA8B
		p.p._msr_colours.push_back(string_string_pair("M", "#4C1C00"));		// seal brown, #4C1C00
		p.p._msr_colours.push_back(string_string_pair("P", "#FFD275"));		// orange yellow crayola, #FFD275
		p.p._msr_colours.push_back(string_string_pair("Q", "#FF6B6B"));		// bittersweet, #FF6B6B
		p.p._msr_colours.push_back(string_string_pair("R", "#DBCBD8"));		// languid lavender, #DBCBD8
		p.p._msr_colours.push_back(string_string_pair("S", "#60492C"));		// dark broan, #60492C
		p.p._msr_colours.push_back(string_string_pair("V", "#235789"));		// bdazzled blue, #235789
		p.p._msr_colours.push_back(string_string_pair("X", "#9A348E"));		// vioet crayola, #9A348E
		p.p._msr_colours.push_back(string_string_pair("Y", "#F7A1C4"));		// carnation pink, #F7A1C4
		p.p._msr_colours.push_back(string_string_pair("Z", "#56A3A6"));		// cadet blue, #56A3A6
		p.p._msr_colours.push_back(string_string_pair("x", "#FF0000"));		// red, #FF0000 (destroyed marks)	
		
		
		if (graph_mode)
		{
			if (graph_mode_stns)
			{
				ProcessGnuPlot(&plotDynaML, p, StationsMode);
				if (p.g.verbose > 0 || p.g.quiet != 1)
				{
					cout << "+ Open " << p.p._pdf_file_name << " to view the PDF stations histogram." << endl << endl;
					cout.flush();
				}
			}
			if (graph_mode_msrs)
			{
				ProcessGnuPlot(&plotDynaML, p, MeasurementsMode);
				if (p.g.verbose > 0 || p.g.quiet != 1)
				{
					cout << "+ Open " << p.p._pdf_file_name << " to view the PDF measurements histogram." << endl << endl;
					cout.flush();
				}
			}

			return EXIT_SUCCESS;
		}

		// If not in graph mode, then generate GMT plot		
		if (!vm.count(LABEL_FONT_SIZE))
			p.p._label_font_size = -1.0;		// determine best font according to plot size and map projection

		if (vm.count(PLOT_CENTRE_LAT))
			p.p._plot_centre_latitude = DmstoDeg(p.p._plot_centre_latitude);
		if (vm.count(PLOT_CENTRE_LON))
			p.p._plot_centre_longitude = DmstoDeg(p.p._plot_centre_longitude);

		if (vm.count(PLOT_STN_LABELS))
			p.p._plot_station_labels = true;
		if (vm.count(PLOT_ALT_NAME))
			p.p._plot_alt_name = true;
		if (vm.count(PLOT_CONSTRAINT_LABELS))
			p.p._plot_station_constraints = true;

		// Create the GMT plot, or just the shell script(s) and data files
		ProcessGMTPlot(&plotDynaML, p, measurement_types);			
	

	} 
	catch (const NetPlotException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		return EXIT_FAILURE;
	} 
	catch (const runtime_error& e) {
		cout << "+ Exception of unknown type: " << e.what();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
