//============================================================================
// Name         : dnaimportwrapper.cpp
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
// Description  : DynAdjust Interoperability library Executable
//============================================================================

#include <dynadjust/dnaimportwrapper/dnaimportwrapper.hpp>
#include <include/parameters/dnaepsg.hpp>

using namespace dynadjust;
using namespace dynadjust::epsg;

bool running;
boost::mutex cout_mutex;

void PrintOutputFileHeaderInfo(std::ofstream* f_out, const string& out_file, project_settings* p, const string& header)
{
	// Print formatted header
	print_file_header(*f_out, header);

	*f_out << setw(PRINT_VAR_PAD) << left << "File name:" << system_complete(out_file).string() << endl << endl;
	
	*f_out << setw(PRINT_VAR_PAD) << left << "Command line arguments: ";
	*f_out << p->i.command_line_arguments << endl << endl;

	*f_out << setw(PRINT_VAR_PAD) << left << "Network name:" <<  p->g.network_name << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Input folder: " << system_complete(p->g.input_folder).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Output folder: " << system_complete(p->g.output_folder).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Stations file:" << system_complete(p->i.bst_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Measurements file:" << system_complete(p->i.bms_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Associated station file:" << system_complete(p->i.asl_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Associated measurement file:" << system_complete(p->i.aml_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Duplicate stations output file:" <<  system_complete(p->i.dst_file).string() << endl;
	*f_out << setw(PRINT_VAR_PAD) << left << "Similar measurement output file:" <<  system_complete(p->i.dms_file).string() << endl;

	if (!p->i.input_files.empty())
	{
		_it_vstr _it_files(p->i.input_files.begin());
		string str("Input files:");
		while (_it_files!=p->i.input_files.end())
		{
			*f_out << setw(PRINT_VAR_PAD) << left << str << *_it_files++ << endl;
			str = " ";
		}
	}
	
	if (!p->i.reference_frame.empty())
		*f_out << setw(PRINT_VAR_PAD) << left << "Default reference frame:" << p->i.reference_frame << endl;
	
	if (!p->i.include_msrs.empty())
		*f_out << setw(PRINT_VAR_PAD) << left << "Strip all measurements except:" << p->i.include_msrs << endl;
	else if (!p->i.exclude_msrs.empty())
		*f_out << setw(PRINT_VAR_PAD) << left << "Strip measurement types:" << p->i.exclude_msrs << endl;
	
	if (p->i.search_nearby_stn)
		*f_out << setw(PRINT_VAR_PAD) << left << "Search for nearby stations:" << "tolerance = " << p->i.search_stn_radius << "m" << endl;
	if (p->i.search_similar_msr)
		*f_out << setw(PRINT_VAR_PAD) << left << "Search for similar measurements:" << "yes" << endl;
	if (p->i.search_similar_msr_gx)
		*f_out << setw(PRINT_VAR_PAD) << left << "Search for similar GNSS measurements:" << "yes" << endl;
	
	if (!p->i.bounding_box.empty())
	{
		*f_out << setw(PRINT_VAR_PAD) << left << "Bounding box: " << p->i.bounding_box << endl;
		if (p->i.split_clusters)
			*f_out << setw(PRINT_VAR_PAD) << left << "Split GNSS clusters: " << (p->i.split_clusters ? "Yes" : "No") << endl;
	}
	else 
	{
		if (!p->i.stn_associated_msr_include.empty())
			*f_out << setw(PRINT_VAR_PAD) << left << "Stations to include: " << p->i.stn_associated_msr_include << endl;
		if (!p->i.stn_associated_msr_exclude.empty())
			*f_out << setw(PRINT_VAR_PAD) << left << "Stations to exclude: " << p->i.stn_associated_msr_exclude << endl;

		if (p->i.split_clusters)
			*f_out << setw(PRINT_VAR_PAD) << left << "Split GNSS clusters: " << (p->i.split_clusters ? "Yes" : "No") << endl;
	}

	if (!p->i.seg_file.empty())
	{
		*f_out << setw(PRINT_VAR_PAD) << left << "Segmentation file:" << system_complete(p->i.seg_file).string() << endl;
		*f_out << setw(PRINT_VAR_PAD) << left << "Import stns & msrs from block: " << p->i.import_block_number << endl;
	}

	*f_out << OUTPUTLINE << endl << endl;
}
	
int ParseCommandLineOptions(const int& argc, char* argv[], const variables_map& vm, project_settings& p)
{
	// capture command line arguments
	for (int cmd_arg(0); cmd_arg<argc; ++cmd_arg)
	{
		 p.i.command_line_arguments += argv[cmd_arg];
		 p.i.command_line_arguments += " ";
	}

	if (vm.count(PROJECT_FILE))
	{
		if (exists(p.g.project_file))
		{
			try {
				CDnaProjectFile projectFile(p.g.project_file, importSetting);
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

	if (!vm.count(IMPORT_FILE) && !vm.count(IMPORT_SEG_BLOCK) && !vm.count(SEG_FILE))
	{
		cout << endl << "- Nothing to do - no files specified. " << endl << endl;  
		return EXIT_FAILURE;
	}

	// Normalise files using input folder
	for_each(p.i.input_files.begin(), p.i.input_files.end(),
		[&p] (string& file) { 
			formPath<string>(p.g.input_folder, file);
		}
	);

	//////////////////////////////////////////////////////////////////////////////
	// General options and file paths
	// Network name
	if (p.g.network_name == "network1")
	{
		// Iterate through network1, network2, network3, etc 
		// until the first name not used is found
		stringstream netname_ss;
		string networkASL;
		UINT32 networkID(1);
		
		// This loop terminates when a file name cannot be found
		while (true)
		{
			// 1. Form ASL file and see if it exists
			networkASL = formPath<string>(p.g.output_folder, p.g.network_name, "asl");

			// 2. Does this network exist?
			if (!exists(networkASL))
				break;

			// 3. Flush and look for the next network
			netname_ss.str("");
			netname_ss << "network" << ++networkID;
			p.g.network_name = netname_ss.str();			
		}
	}

	p.g.project_file = formPath<string>(p.g.output_folder, p.g.network_name, "dnaproj");

	// binary station file location (output)
	if (vm.count(BIN_STN_FILE))
		p.i.bst_file = formPath<string>(p.g.output_folder, p.i.bst_file);
	else
		p.i.bst_file = formPath<string>(p.g.output_folder, p.g.network_name, "bst");
	
	// binary station file location (output)
	if (vm.count(BIN_MSR_FILE))
		p.i.bms_file = formPath<string>(p.g.output_folder, p.i.bms_file);
	else
		p.i.bms_file = formPath<string>(p.g.output_folder, p.g.network_name, "bms");

	if (vm.count(IMPORT_GEO_FILE))
		p.i.import_geo_file = 1;

	// output files
	p.i.asl_file = formPath<string>(p.g.output_folder, p.g.network_name, "asl");	// associated stations list
	p.i.aml_file = formPath<string>(p.g.output_folder, p.g.network_name, "aml");	// associated measurements list
	p.i.map_file = formPath<string>(p.g.output_folder, p.g.network_name, "map");	// station names map
	p.i.dst_file = formPath<string>(p.g.output_folder, p.g.network_name, "dst");	// fuplicate stations
	p.i.dms_file = formPath<string>(p.g.output_folder, p.g.network_name, "dms");	// duplicate measurements
	p.i.imp_file = formPath<string>(p.g.output_folder, p.g.network_name, "imp");	// log
	
	//////////////////////////////////////////////////////////////////////////////
	// Ref frame options
	if (vm.count(OVERRIDE_INPUT_FRAME))
		p.i.override_input_rfame = 1;
	if (vm.count(REFERENCE_FRAME))
		p.i.user_supplied_frame = 1;

	//////////////////////////////////////////////////////////////////////////////
	// Data screening options
	if (vm.count(GET_MSRS_TRANSCENDING_BOX))
		p.i.include_transcending_msrs = 1;

	if (vm.count(SPLIT_CLUSTERS))
		p.i.split_clusters = 1;

	// User supplied segmentation file
	if (vm.count(IMPORT_SEG_BLOCK))
		p.i.import_block = 1;

	// User supplied segmentation file
	if (vm.count(SEG_FILE))
	{
		// Does it exist?
		if (!exists(p.i.seg_file))
			// Look for it in the input folder
			p.i.seg_file = formPath<string>(p.g.input_folder, leafStr<string>(p.i.seg_file));
	}

	// convert all single X measurements to G measurements?
	if (vm.count(PREFER_X_MSR_AS_G))
		p.i.prefer_single_x_as_g = 1;

	if (vm.count(TEST_NEARBY_STNS))
		p.i.search_nearby_stn = 1;

	if (vm.count(TEST_SIMILAR_MSRS))
		p.i.search_similar_msr = 1;

	if (vm.count(TEST_SIMILAR_GNSS_MSRS))
		p.i.search_similar_msr_gx = 1;

	if (vm.count(IGNORE_SIMILAR_MSRS))
		p.i.ignore_similar_msr = 1;

	if (vm.count(REMOVE_IGNORED_MSRS))
		p.i.remove_ignored_msr = 1;

	if (vm.count(FLAG_UNUSED_STNS))
		p.i.flag_unused_stn = 1;

	if (vm.count(TEST_INTEGRITY))
		p.i.test_integrity = 1;
	
	//////////////////////////////////////////////////////////////////////////////
	// GNSS scaling
	if (vm.count(VSCALE) ||
		vm.count(PSCALE) ||
		vm.count(LSCALE) ||
		vm.count(HSCALE) ||
		vm.count(SCALAR_FILE))
		p.i.apply_scaling = 1;

	if (vm.count(SCALAR_FILE))
		if (!exists(p.i.scalar_file))			// does it exist?
			// No.  Assume it is a filename contained in the input folder.  import will throw
			// an exception if it cannot be found.
			p.i.scalar_file = formPath<string>(p.g.input_folder, p.i.scalar_file);	
	
	//////////////////////////////////////////////////////////////////////////////
	// Export options
	
	if (vm.count(OUTPUT_MSR_TO_STN))
		p.o._msr_to_stn = 1;

	p.o._m2s_file = formPath<string>(p.g.output_folder, p.g.network_name, "m2s");	// measurement to stations table

	// Create file name based on the provided block
	string fileName(p.g.network_name);
	if (p.i.import_block)
	{
		stringstream blk("");
		blk << ".block-" << p.i.import_block_number;
		fileName += blk.str();
	}

	// Export to dynaml?
	if (vm.count(EXPORT_XML_FILES))
	{
		p.i.export_dynaml = 1;
	
		// Export from binary
		if (vm.count(EXPORT_FROM_BINARY))
			p.i.export_from_bfiles = 1;

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

	if (vm.count(EXPORT_ASL_FILE))
		p.i.export_asl_file = 1;
	if (vm.count(EXPORT_AML_FILE))
		p.i.export_aml_file = 1;
	if (vm.count(EXPORT_MAP_FILE))
		p.i.export_map_file = 1;
	if (vm.count(EXPORT_DISCONT_FILE))
		p.i.export_discont_file = 1;
	
	// Simulate
	if (vm.count(SIMULATE_MSR_FILE))
	{
		p.i.simulate_measurements = 1;
		p.i.simulate_msrfile = formPath<string>(p.g.output_folder,
			p.g.network_name, "simulated.msr");
	}

	// Station renaming file
	if (vm.count(STATION_RENAMING_FILE))
		p.i.rename_stations = 1;

	return EXIT_SUCCESS;
}

void PrintMeasurementstoStations(MsrTally* parsemsrTally, dna_import* parserDynaML, 
	project_settings* p, pvASLPtr vAssocStnList)
{
	// Print measurements to stations table
	if (p->o._msr_to_stn)
	{
		if (!p->g.quiet)
		{
			cout << "+ Printing summary of measurements connected to each station...";
			cout.flush();
		}
		parserDynaML->PrintMeasurementsToStations(p->o._m2s_file, parsemsrTally, 
			p->i.bst_file, p->i.bms_file, p->i.aml_file, vAssocStnList);
		if (!p->g.quiet)
			cout << " done." << endl;
	}
}

int SearchForSimilarMeasurements(dna_import* parserDynaML, project_settings* p, std::ofstream* imp_file,
	vdnaStnPtr* vstationsTotal, vdnaMsrPtr* vmeasurementsTotal)
{
	std::ofstream dms_file;
	UINT32 msr;
	vdnaMsrPtr vSimilarMeasurements;

	try {
		if (!p->g.quiet)
		{
			cout << "+ Searching for similar measurements... ";
			cout.flush();
		}
		*imp_file << "+ Searching for similar measurements... ";

		// at this stage, perform only one search option, giving preference to GNSS searching 
		// if both are provided
		if (p->i.search_similar_msr_gx)
			msr = parserDynaML->FindSimilarGXMeasurements(vmeasurementsTotal, &vSimilarMeasurements);
		else
			msr = parserDynaML->FindSimilarMeasurements(vmeasurementsTotal, &vSimilarMeasurements);

		if (!vSimilarMeasurements.empty())
		{
			ostringstream ss_msg;
			if (!p->i.ignore_similar_msr)
				ss_msg << endl << "- Error: ";
			else
				ss_msg << endl << "- Warning: ";

			ss_msg << msr << " measurement" << (msr > 1 ? "s were" : " was") << 
				" found to be very similar (if not identical)\n  to " << 
				(msr > 1 ? "other measurements" : "another measurement") << 
				".  See " << p->i.dms_file << " for details." << endl;

			*imp_file << ss_msg.str();
			if (!p->g.quiet)
				cout << ss_msg.str();
			ss_msg.str("");

			try {
				// Create duplicate measurements file
				file_opener(dms_file, p->i.dms_file);
			}
			catch (const ios_base::failure& f) {
				stringstream ss;
				ss << "- Error: Could not open " << p->i.dms_file << "." << endl;
				ss << "  Check that the file exists and that the file is not already opened." << endl << f.what();
				if (!p->g.quiet)
					cout << ss.str();
				*imp_file << ss.str();
				imp_file->close();
				return EXIT_FAILURE;
			}

			PrintOutputFileHeaderInfo(&dms_file, p->i.dms_file, p, "DUPLICATE MEASUREMENTS FILE");

			// output message
			dms_file << endl << "- " << msr << " measurement" << (msr > 1 ? "s were" : " was") << 
				" found to be very similar (if not identical)\n  to " << 
				(msr > 1 ? "other measurements." : "another measurement.  ");

			if (p->i.ignore_similar_msr)
				dms_file << endl << "+ These measurements have been ignored.";
			dms_file << endl << endl;

			// dump measurements to dms file
			for_each (vSimilarMeasurements.begin(), vSimilarMeasurements.end(),
				[&dms_file] (dnaMsrPtr& m) {
					m->WriteDynaMLMsr(&dms_file);
			});

			dms_file.close();

			if (p->i.ignore_similar_msr)
				ss_msg << "  These measurements have been ignored." << endl;
			else
				ss_msg << endl <<
				"  If the listed measurements are true duplicates, either remove each duplicate " << endl <<
				"  from the measurement file and re-run " << __BINARY_NAME__ << ", or re-run " << __BINARY_NAME__ << " with the" << endl <<
				"  --" << IGNORE_SIMILAR_MSRS << " option.  Alternatively, if each measurement " << endl <<
				"  is unique, then call " << __BINARY_NAME__ << " without the --" << TEST_SIMILAR_MSRS << " option." << endl << endl;

			if (!p->g.quiet)
				cout << ss_msg.str();
			*imp_file << ss_msg.str();

			if (!p->i.ignore_similar_msr)
			{
				imp_file->close();
				return EXIT_FAILURE;
			}
		}
		else
		{
			*imp_file << "Done. ";
			if (!p->g.quiet)
			{
				cout << "Done. ";
				cout.flush();
			}
		}

		*imp_file << endl;
		if (!p->g.quiet)
		{
			cout << endl;
			cout.flush();
		}

	} 
	catch (const XMLInteropException& e) {
		cout << endl << "- Error: " << e.what() << endl;
		*imp_file << endl << "- Error: " << e.what() << endl;
		imp_file->close();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
	

void ExportStationsandMeasurements(dna_import* parserDynaML, const project_settings& p, std::ofstream* imp_file, vifm_t* vinput_file_meta,
	vdnaStnPtr* vstationsTotal, vdnaMsrPtr* vmeasurementsTotal, const UINT32& stnCount, const UINT32& msrCount)
{
	stringstream ssEpsgWarning;
	bool displayEpsgWarning(false);
	string epsgCode(epsgStringFromName<string>(p.i.reference_frame));

	// Check inconsistent reference frames
	if ((p.i.export_dynaml || p.i.export_dna_files) && !p.i.override_input_rfame && !p.i.user_supplied_frame)
	{
		for (UINT32 i(0); i<vinput_file_meta->size(); ++i)
		{
			if (!iequals(epsgCode, vinput_file_meta->at(i).epsgCode))
			{
				string inputFrame(datumFromEpsgString<string>(vinput_file_meta->at(i).epsgCode));
				ssEpsgWarning << endl << "- Warning: The default reference frame (used for all exported files)" << endl << 
					"  does not match the reference frame of one or more input files. To" << endl <<
					"  suppress this warning, override the default reference frame using" << endl <<
					"  --reference-frame, or provide --override-input-ref-frame.";
				displayEpsgWarning = true;
				break;
			}
		}
	}

	// Sort on original file order
	parserDynaML->SortStationsForExport(vstationsTotal);

	// DynaML file format
	if (p.i.export_dynaml && (stnCount > 0 || msrCount > 0)) 
	{
		// The export from binary files here is merely to test functionality and to
		// diagnose problems.  It is somewhat excessive to allow the user to choose 
		// the source from which XML files should be exported
		if (p.i.export_from_bfiles)
		{
			if (p.i.export_single_xml_file)
			{
				// Single output file
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
					cout.flush();
				}
				*imp_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
				parserDynaML->SerialiseDynaMLfromBinary(
					p.i.xml_outfile, p, vinput_file_meta,
					(p.i.flag_unused_stn ? true : false));
			}
			else
			{
				// Separate output files (default)
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_stnfile) << " and " << leafStr<string>(p.i.xml_msrfile) << "... ";
					cout.flush();
				}
				*imp_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_stnfile) << " and " << leafStr<string>(p.i.xml_msrfile) << "... ";
				parserDynaML->SerialiseDynaMLSepfromBinary(
					p.i.xml_stnfile, p.i.xml_msrfile, p, vinput_file_meta,
					(p.i.flag_unused_stn ? true : false));	
			}
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
		}
		else
		{
			if (p.i.export_single_xml_file)
			{
				// Single output file
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
					cout.flush();
				}
				*imp_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_outfile) << "... ";
				parserDynaML->SerialiseDynaMLfromMemory(
					vstationsTotal, vmeasurementsTotal, 
					p.i.xml_outfile, p, vinput_file_meta, (p.i.flag_unused_stn ? true : false));
			}	
			else
			{
				// Separate output files (default)
				if (!p.g.quiet)
				{
					cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_stnfile) << " and " << leafStr<string>(p.i.xml_msrfile) << "... ";
					cout.flush();
				}
				*imp_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.xml_stnfile) << " and " << leafStr<string>(p.i.xml_msrfile) << "... ";
				parserDynaML->SerialiseDynaMLSepfromMemory(
					vstationsTotal, vmeasurementsTotal, 
					p.i.xml_stnfile, p.i.xml_msrfile, p, vinput_file_meta, (p.i.flag_unused_stn ? true : false));
			}
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
		}		
	}

	// DNA file format
	if (p.i.export_dna_files && (stnCount > 0 || msrCount > 0)) 
	{
		// Separate output files (default)
		if (!p.g.quiet)
		{
			cout << "+ Exporting stations and measurements to " << leafStr<string>(p.i.dna_stnfile) << " and " << leafStr<string>(p.i.dna_msrfile) << "... ";
			cout.flush();
		}
		*imp_file << "+ Exporting stations and measurements to " << leafStr<string>(p.i.dna_msrfile) << " and " << leafStr<string>(p.i.dna_msrfile) << "... ";
		parserDynaML->SerialiseDNA(
			vstationsTotal, vmeasurementsTotal,
			p.i.dna_stnfile, p.i.dna_msrfile, p, vinput_file_meta, (p.i.flag_unused_stn ? true : false));
		if (!p.g.quiet)
			cout << "Done." << endl;
		*imp_file << "Done." << endl;

	}

	if (displayEpsgWarning)
	{
		cout << ssEpsgWarning.str() << endl;
		*imp_file << ssEpsgWarning.str() << endl;
		cout.flush();
	}
}

int ImportSegmentedBlock(dna_import& parserDynaML, vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, 
		StnTally* parsestnTally, MsrTally* parsemsrTally, project_settings& p)
{	
	// Form default seg file path
	bool userSuppliedSegFile(false);

	// Has the user provided a segmentation file?
	if (!p.i.seg_file.empty())
		userSuppliedSegFile = true;
	else
		p.i.seg_file = formPath<string>(p.g.input_folder, p.g.network_name, "seg");

	if (!exists(p.i.seg_file))
	{
		cout << endl << "- Error: The required segmentation file does not exist:" << endl <<  
			"         " << p.i.seg_file << endl << endl <<
			"  Run  'segment " << p.g.network_name << "' to create a segmentation file" << endl << endl;
		return EXIT_FAILURE;
	}

	if (!exists(p.i.bst_file))
	{
		cout << endl << "- Error: The required binary station file does not exist:" << endl << 
			"         " << p.i.bst_file << endl << endl;
		return EXIT_FAILURE;
	}

	if (!exists(p.i.bms_file))
	{
		cout << endl << "- Error: The required binary measurement file does not exist:" << endl << 
			"         " << p.i.bms_file << endl << endl;
		return EXIT_FAILURE;
	}

	// If the user has not provided a seg file, check the meta of the default file
	if (!userSuppliedSegFile)
	{
		if (last_write_time(p.i.seg_file) < last_write_time(p.i.bst_file) ||
			last_write_time(p.i.seg_file) < last_write_time(p.i.bms_file))
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

			if ((bst_meta_import && (last_write_time(p.i.seg_file) < last_write_time(p.i.bst_file))) || 
				(bms_meta_import && (last_write_time(p.i.seg_file) < last_write_time(p.i.bms_file))))
			{

				cout << endl << endl << 
					"- Error: The raw stations and measurements have been imported after" << endl <<
					"  the segmentation file was created:" << endl;

				time_t t_bst(last_write_time(p.i.bst_file)), t_bms(last_write_time(p.i.bms_file));
				time_t t_seg(last_write_time(p.i.seg_file));

				cout << "   " << leafStr<string>(p.i.bst_file) << "  last modified on  " << ctime(&t_bst);
				cout << "   " << leafStr<string>(p.i.bms_file) << "  last modified on  " << ctime(&t_bms) << endl;
				cout << "   " << leafStr<string>(p.i.seg_file) << "  created on  " << ctime(&t_seg) << endl;
				cout << "  Run 'segment " << p.g.network_name << " [options]' to re-create the segmentation file, or re-run" << endl << 
					"  the import using the " << SEG_FILE << " option if this segmentation file must\n  be used." << endl << endl;
				return EXIT_FAILURE;
			}
		}
	}

	// Import stations and measurements from a particular block
	if (!p.g.quiet)
		cout << endl << "+ Importing stations and measurements from block " << p.i.import_block_number << " of\n  " << p.i.seg_file << "... ";
	parserDynaML.ImportStnsMsrsFromBlock(vStations, vMeasurements, p);
	*parsestnTally += parserDynaML.GetStnTally();
	*parsemsrTally += parserDynaML.GetMsrTally();
	if (!p.g.quiet)
		cout << "Done. " << endl;

	// Restore seg_file to null
	if (!userSuppliedSegFile)
		p.i.seg_file = "";

	return EXIT_SUCCESS;
}

int ImportDataFiles(dna_import& parserDynaML, vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements,
		vdnaStnPtr* vstationsTotal, vdnaMsrPtr* vmeasurementsTotal,
		std::ofstream* imp_file, vifm_t* vinput_file_meta, StnTally* parsestnTally, MsrTally* parsemsrTally, 
		UINT32& errorCount, project_settings& p)
{
	// For consideration:  
	//	- All input files could be read concurrently using multi-thread for faster input
	//	- The number of files that can be processed concurrently should be restricted to
	//	  the number of available cores
	//	- If multi-threading is to be used, then the progress thread will need to be redesigned.
	//	  Currently, the progress of each file (as a percentage) is written to cout as each file
	//	  is processed. If files are processed concurrently, progress reporting will need to be
	//    redesigned if multiple progress threads are writing to cout.
	//	- There is significant gain in using multi-thread.
	//	- The catch will be the need to carefully design one progress thread that captures progress
	//	  of all other "read" threads (rather than one progress for each), and somehow write to 
	//	  cout the respective progress.  For example, loading four files could be displayed as 
	//	  follows:
	//
	//	  + Parsing file1.xml (32%), file2.xml (16%), file3.xml (86%), file4.xml (56%)
	//	    Done: Loaded 190 stations from file3.xml in 00.032s
	//	  + Parsing file1.xml (72%), file2.xml (48%), file4.xml (91%)
	//	    Done: Loaded 236 stations from file4.xml in 00.050s
	//	  + Parsing file1.xml (87%), file2.xml (76%)
	//	    Done: Loaded 1596 measurements from file1.xml in 01.380s
	//	    Done: Loaded 3142 measurements from file2.xml in 02.034s
	//
	UINT32 stnCount(0), msrCount(0), clusterID(0);
	
	size_t pos = string::npos;
	size_t strlen_arg = 0;
	for_each(p.i.input_files.begin(), p.i.input_files.end(),
		[&strlen_arg](string& file) {
			if (leafStr<string>(file).length() > strlen_arg)
				strlen_arg = leafStr<string>(file).length();
	});

	strlen_arg += (6 + PROGRESS_PERCENT_04);

	size_t i, nfiles(p.i.input_files.size());		// for each file...
	string input_file, ss, status_msg;
	vstring input_files;
	ostringstream ss_time, ss_msg;
	input_file_meta_t input_file_meta;
	milliseconds elapsed_time(milliseconds(0));
	ptime pt;

	if (!p.g.quiet)
		cout << "+ Parsing: " << endl;
	*imp_file << "+ Parsing " << endl;

	for (i=0; i<nfiles; i++)
	{
		stnCount = msrCount = 0;
		input_file = p.i.input_files.at(i);
		if (!exists(input_file))
		{
			input_file = formPath<string>(p.g.input_folder, input_file);
			if (!exists(input_file))
			{	
				cout << "- Error:  " << input_file << " does not exist" << endl;
				return EXIT_FAILURE;
			}
		}
			
		input_files.push_back(input_file);
		ss = leafStr<string>(p.i.input_files.at(i)) + "... ";
		if (!p.g.quiet)
			cout << "  " << setw(strlen_arg) << left << ss;
		*imp_file << "  " << setw(strlen_arg) << left << ss;

		running = true;
		thread_group ui_interop_threads;
		if (!p.g.quiet)
			ui_interop_threads.create_thread(dna_import_progress_thread(&parserDynaML, &p));
		ui_interop_threads.create_thread(dna_import_thread(&parserDynaML, &p, input_file,
			vStations, &stnCount, vMeasurements, &msrCount,
			&clusterID, &input_file_meta, &status_msg,
			&elapsed_time));
		ui_interop_threads.join_all();

		switch (parserDynaML.GetStatus())
		{
		case PARSE_EXCEPTION_RAISED:
			*imp_file << endl << status_msg;
			running = false;
			return EXIT_FAILURE;
			break;
		case PARSE_UNRECOGNISED_FILE:
			*imp_file << status_msg << endl;
			errorCount++;
			continue;
		case PARSE_SUCCESS:
			running = false;
			break;
		default:
			errorCount++;
			cout << endl;
			continue;
		}

		vinput_file_meta->push_back(input_file_meta);

		ss_time.str("");

		if (stnCount > 0 || msrCount > 0) // stations or measurements only
		{			
			ss_time << "  Done. Loaded ";
			if (stnCount)
				ss_time << stnCount << " stations";
			if (stnCount && msrCount)
				ss_time << " and ";
			if (msrCount)
				ss_time << msrCount << " measurements";
			ss_time << " in ";
				
			pt = ptime(gregorian::day_clock::local_day(), elapsed_time);
			if (elapsed_time < seconds(3))
			{
				time_facet* facet(new time_facet("%s"));
				ss_time.imbue(locale(ss_time.getloc(), facet));
				ss_time << pt << "s";			
			}
			else if (elapsed_time < seconds(61))
			{		
				time_facet* facet(new time_facet("%S"));
				ss_time.imbue(locale(ss_time.getloc(), facet));
				ss_time << pt << "s";
			}
			else
				ss_time << elapsed_time;

			string time_message = ss_time.str();
			while ((pos = time_message.find("0s")) != string::npos)
				time_message = time_message.substr(0, pos) + "s";

			if ((pos = time_message.find(" 00.")) != string::npos)
				time_message = time_message.replace(pos, 4, " 0.");
			if ((pos = time_message.find(" 0.s")) != string::npos)
				time_message = time_message.replace(pos, 4, " 0s");

			if (!p.g.quiet)
			{
				if (isatty(fileno(stdout)))
					cout << PROGRESS_BACKSPACE_04;
				cout << time_message << endl;
			}
			*imp_file << time_message << endl;

			// Produce a warning if the input file's default reference frame
			// is different to the project reference frame
			string epsgCode(epsgStringFromName<string>(p.i.reference_frame));
			string inputFileEpsg;
			try {
				inputFileEpsg = datumFromEpsgString<string>(input_file_meta.epsgCode);
			}
			catch (...) {
				inputFileEpsg = epsgCode;
			}

			if (!iequals(epsgCode, input_file_meta.epsgCode))
			{
				stringstream ssEpsgWarning;
				

				ssEpsgWarning << "- Warning: Input file reference frame (" << inputFileEpsg <<
					") does not match the " << endl << "  default reference frame.";
				if (!p.g.quiet)
					cout << ssEpsgWarning.str() << endl;
				*imp_file << ssEpsgWarning.str() << endl;
			}
		}

		// Handle discontinuities for non-sinex files, if and only if a discontinuity file has been loaded.
		// NOTE: If a station is marked as a discontinuity site, and that same station is in the renaming
		// file, station renaming for that site will not occur.  This is because the site name will be changed
		// to <site-name>_yyyymmdd.
		// If renaming must still occur, then it might be possible to rename using the first part of the name.
		

		if (stnCount > 0) // stations only
		{
			//vstationsTotal.reserve(vstationsTotal.size() + stnCount);
			// combine stations and station tally
			vstationsTotal->insert(vstationsTotal->end(), vStations->begin(), vStations->end());
			*parsestnTally += parserDynaML.GetStnTally();
			vStations->clear();
		}
		if (msrCount > 0) // measurements only
		{		
			//vmeasurementsTotal.reserve(vmeasurementsTotal.size() + msrCount);
			// combine measurements
			vmeasurementsTotal->insert(vmeasurementsTotal->end(), vMeasurements->begin(), vMeasurements->end());
			*parsemsrTally += parserDynaML.GetMsrTally();
			vMeasurements->clear();
		}
	}

	if (errorCount == nfiles || (vstationsTotal->empty() && vmeasurementsTotal->empty()))
		return EXIT_FAILURE;

	running = false;
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
	options_description ref_frame_options("+ " + string(IMPORT_MODULE_FRAME), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description data_screening_options("+ " + string(IMPORT_MODULE_SCREEN), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description gnss_scaling_options("+ " + string(IMPORT_MODULE_GNSS_VAR), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description simulation_options("+ " + string(IMPORT_MODULE_SIMULATE), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description output_options("+ " + string(ALL_MODULE_OUTPUT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description export_options("+ " + string(ALL_MODULE_EXPORT), PROGRAM_OPTIONS_LINE_LENGTH);
	options_description generic_options("+ " + string(ALL_MODULE_GENERIC), PROGRAM_OPTIONS_LINE_LENGTH);

	string cmd_line_usage("+ ");
	cmd_line_usage.append(__BINARY_NAME__).append(" usage:  ").append(__BINARY_NAME__).append(" [options] [files]...");
	options_description allowable_options(cmd_line_usage, PROGRAM_OPTIONS_LINE_LENGTH);

	try {
		// Declare a group of options that will be 
		// allowed only on command line		
		standard_options.add_options()
			(PROJECT_FILE_P, value<string>(&p.g.project_file),
				"Project file name. Full path to project file. If none specified, a new file is created using input-folder and network-name.  If a project file exists, the file is used to set all command line options.")
			(NETWORK_NAME_N, value<string>(&p.g.network_name), 
				"Network name. User defined name for all input and output files. Default is \"network#\", where # is incremented until the first available network is reached.")
			(IMPORT_FILE_F, value< vstring >(&p.i.input_files), 
				"Station and measurement input file(s). Switch is not required.")
			(IMPORT_GEO_FILE_G, value<string>(&p.i.geo_file),
				"Import DNA geoid file.")
			(INPUT_FOLDER_I, value<string>(&p.g.input_folder),
				"Path containing all input files.")
			(OUTPUT_FOLDER_O, value<string>(&p.g.output_folder),		// default is ./,
				"Path for all output files.")
			(BIN_STN_FILE_S, value<string>(&p.i.bst_file),
				"Binary station output file name. Overrides network name.")
			(BIN_MSR_FILE_M, value<string>(&p.i.bms_file),
				"Binary measurement output file name. Overrides network name.")
			;

		ref_frame_options.add_options()
			(REFERENCE_FRAME_R, value<string>(&p.i.reference_frame), 
			(string("Default reference frame for all stations and measurements, and for preliminary reductions on the ellipsoid when input files do not specify a reference frame. Default is ") +
				p.i.reference_frame + ".").c_str())
			(OVERRIDE_INPUT_FRAME,
				"Override the reference frame specified for each measurement in input files.")
			;

		data_screening_options.add_options()
			(BOUNDING_BOX, value<string>(&p.i.bounding_box),
				"Import stations and measurements within bounding box. arg is a comma delimited string \"lat1,lon1,lat2,lon2\" (in dd.mmss) defining the upper-left and lower-right limits.")
			(GET_MSRS_TRANSCENDING_BOX,
				"Include measurements which transcend bounding box, including associated stations.")
			(INCLUDE_STN_ASSOC_MSRS, value<string>(&p.i.stn_associated_msr_include),
				"Include stations and all associated measurements. arg is a comma delimited string \"stn 1,stn 2,stn 3,...,stn N\" of the stations to include.")
			(EXCLUDE_STN_ASSOC_MSRS, value<string>(&p.i.stn_associated_msr_exclude),
				"Exclude stations and all associated measurements. arg is a comma delimited string \"stn 1,stn 2,stn 3,...,stn N\" of the stations to exclude.")
			(SPLIT_CLUSTERS,
				"Allow bounding-box or get-stns-and-assoc-msrs to split GNSS point and baseline cluster measurements.")
			(IMPORT_SEG_BLOCK, value<UINT32>(&p.i.import_block_number),
				"Extract stations and measurements from this block.")
			(SEG_FILE, value<string>(&p.i.seg_file),
				"Network segmentation input file. Filename overrides network name.")
			(PREFER_X_MSR_AS_G,
				"Import single baseline cluster measurements (X) as single baselines (G).")
			(INCLUDE_MSRS, value<string>(&p.i.include_msrs),
				"Import the specified measurement types. arg is a non-delimited string of measurement types (eg \"GXY\").")
			(EXCLUDE_MSRS, value<string>(&p.i.exclude_msrs),
				"Exclude the specified measurement types. arg is a non-delimited string of measurement typs (eg \"IJK\").")
			(STATION_RENAMING_FILE, value<string>(&p.i.stn_renamingfile),
				"Station renaming file")
			(STATION_DISCONTINUITY_FILE, value<string>(&p.i.stn_discontinuityfile),
				"Station discontinuity file.  Applies discontinuity dates to station names in station and measurement files.")
			(TEST_NEARBY_STNS,
				"Search for nearby stations.")
			(TEST_NEARBY_STN_DIST, value<double>(&p.i.search_stn_radius),
				(string("Specify the radius of the circle within which to search for nearby stations.  Default is ")+
				StringFromT(STN_SEARCH_RADIUS)+string("m")).c_str())
			(TEST_SIMILAR_GNSS_MSRS,
				"Search and provide warnings for GNSS baselines (G) and baseline clusters (X) which appear to have been derived from the same source data.")
			(TEST_SIMILAR_MSRS,
				"Search and provide warnings for similar measurements.")
			(IGNORE_SIMILAR_MSRS,
				"Ignore similar measurements.")
			(REMOVE_IGNORED_MSRS,
				"Remove ignored measurements.")
			(FLAG_UNUSED_STNS,
				"Mark unused stations in binary file.  Stations marked will be excluded from any further processing.")
			(TEST_INTEGRITY,
				"Test the integrity of the association lists and binary files.")
			;

		gnss_scaling_options.add_options()
			(VSCALE, value<double>(&p.i.vscale),
				(string("Global variance (v) matrix scalar for all GNSS measurements.  Replaces existing scalar.  Default is ")+
				StringFromT(p.i.vscale)+string(".")).c_str())
			(PSCALE, value<double>(&p.i.pscale),
				(string("Latitude (p=phi) variance matrix scalar for all GNSS measurements.  Replaces existing scalar.  Default is ")+
				StringFromT(p.i.pscale)+string(".")).c_str())
			(LSCALE, value<double>(&p.i.lscale),
				(string("Longitude (l=lambda) variance matrix scalar for all GNSS measurements.  Replaces existing scalar.  Default is ")+
				StringFromT(p.i.lscale)+string(".")).c_str())
			(HSCALE, value<double>(&p.i.hscale),
				(string("Height (h) variance matrix scalar for all GNSS measurements.  Replaces existing scalar.  Default is ")+
				StringFromT(p.i.hscale)+string(".")).c_str())
			(SCALAR_FILE, value<string>(&p.i.scalar_file),
				"File containing v, p, l and h scalars for GNSS baseline measurements between specific station pairs.  Scalar file values do not apply to GNSS point or baseline clusters.")
			;

		output_options.add_options()
			(OUTPUT_MSR_TO_STN,
				"Output summary of measurements connected to each station.")
			;

		export_options.add_options()
			(EXPORT_XML_FILES,
				"Export stations and measurements to DynaML (DynAdjust XML) format.")
			(EXPORT_SINGLE_XML_FILE,
				"Create a single DynaML file for stations and measurements.")
			//(EXPORT_FROM_BINARY,
			//	"Create DynaML output file using binary files. Default option uses internal memory.")
			(EXPORT_DNA_FILES,
				"Export stations and measurements to DNA STN and MSR format.")
			(EXPORT_ASL_FILE,
				"Export the ASL file as raw text.")
			(EXPORT_AML_FILE,
				"Export the AML file as raw text.")
			(EXPORT_MAP_FILE,
				"Export the MAP file as raw text.")
			(EXPORT_DISCONT_FILE,
				"Export discontinuity information as raw text.")
			;

		simulation_options.add_options()
			(SIMULATE_MSR_FILE,
				"Simulate exact measurements corresponding to the input measurements using the coordinates in the station file. To apply geoid--ellipsoid separations and deflections of the vertical to the simulated measurements, introduce a geoid file using the --geo-file option.")
			;

		generic_options.add_options()
			(VERBOSE, value<UINT16>(&p.g.verbose),
				string("Give detailed information about what ").append(__BINARY_NAME__).append(" is doing.\n  0: No information (default)\n  1: Helpful information\n  2: Extended information\n  3: Debug level information").c_str())
			(QUIET,
				string("Suppresses all explanation of what ").append(__BINARY_NAME__).append(" is doing unless an error occurs.").c_str())
			(VERSION_V, "Display the current program version.")
			(HELP_H, "Show this help message.")
			(HELP_MODULE, value<string>(),
				"Provide help for a specific help category.")
			;

		allowable_options.add(standard_options).add(ref_frame_options).add(data_screening_options).add(gnss_scaling_options).add(simulation_options).add(output_options).add(export_options).add(generic_options);

		// add "positional options" to handle command line tokens which have no option name
		positional_options.add(IMPORT_FILE, -1);
		
		command_line_parser parser(argc, argv);
		store(parser.options(allowable_options).positional(positional_options).run(), vm);
		notify(vm);
	}
	catch(const std::exception& e) 
	{
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
		else if (str_upper<string, char>(IMPORT_MODULE_FRAME).find(help_text) != string::npos) {
			cout << ref_frame_options << endl;
		} 
		else if (str_upper<string, char>(IMPORT_MODULE_SCREEN).find(help_text) != string::npos) {
			cout << data_screening_options << endl;
		} 
		else if (str_upper<string, char>(IMPORT_MODULE_GNSS_VAR).find(help_text) != string::npos) {
			cout << gnss_scaling_options << endl;
		} 
		else if (str_upper<string, char>(IMPORT_MODULE_SIMULATE).find(help_text) != string::npos) {
			cout << simulation_options << endl;
		} 
		else if (str_upper<string, char>(ALL_MODULE_OUTPUT).find(help_text) != string::npos) {
			cout << output_options << endl;
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

	bool userSuppliedSegFile(false);
	if (!p.i.seg_file.empty())
		userSuppliedSegFile = true;
	bool userSuppliedBstFile(false);
	if (!p.i.bst_file.empty())
		userSuppliedBstFile = true;
	bool userSuppliedBmsFile(false);
	if (!p.i.bms_file.empty())
		userSuppliedBmsFile = true;

	if (ParseCommandLineOptions(argc, argv, vm, p) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	UINT32 errorCount(0);
	bool stn_map_created = false, measurements_mapped = false;

	string input_file;
	vstring input_files;
	string status_msg;

	std::ofstream imp_file;
	try {
		// Create import log file.  Throws runtime_error on failure.
		file_opener(imp_file, p.i.imp_file);
	}
	catch (const runtime_error& e) {
		stringstream ss;
		ss << "- Error: Could not open " << p.i.imp_file << ". \n  Check that the file exists and that the file is not already opened." << endl;
		cout << ss.str() << e.what() << endl;
		return EXIT_FAILURE;
	}

	vdnaStnPtr vStations, vstationsTotal;
	vdnaMsrPtr vMeasurements, vmeasurementsTotal;
	vdnaMsrPtr::iterator _itm;
	vASLPtr associatedSL;
	vUINT32 associatedML;
	v_string_uint32_pair vStnsMap_sortName;	// Station Name Map sorted on name (string)
	
	vstationsTotal.clear();
	vmeasurementsTotal.clear();

	if (vm.count(QUIET))
		p.g.quiet = 1;
	
	if (!p.g.quiet)
	{
		cout << endl << cmd_line_banner;

		cout << "+ Options:" << endl; 
		cout << setw(PRINT_VAR_PAD) << left << "  Network name: " <<  p.g.network_name << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Input folder: " << p.g.input_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Output folder: " << p.g.output_folder << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Associated station file: " << p.i.asl_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Associated measurement file: " << p.i.aml_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary station output file: " << p.i.bst_file << endl;
		cout << setw(PRINT_VAR_PAD) << left << "  Binary measurement output file: " << p.i.bms_file << endl;
		
		if (!p.i.reference_frame.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  Default reference frame:" << p.i.reference_frame << endl;
	
		if (p.i.override_input_rfame)
			cout << setw(PRINT_VAR_PAD) << left << "  Override input file ref frame:" << yesno_string(p.i.override_input_rfame) << endl;

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

		if (p.i.simulate_measurements)
		{
			cout << setw(PRINT_VAR_PAD) << left << "  DNA simulated msr file: " << p.i.simulate_msrfile << endl;
		}
		
		if (!p.i.bounding_box.empty())
		{
			cout << setw(PRINT_VAR_PAD) << left << "  Bounding box: " << p.i.bounding_box << endl;
			if (p.i.split_clusters)
				cout << setw(PRINT_VAR_PAD) << left << "  Split GNSS clusters: " << (p.i.split_clusters ? "Yes" : "No") << endl;
		}
		else
		{
			if (!p.i.stn_associated_msr_include.empty())
				cout << setw(PRINT_VAR_PAD) << left << "  Stations to include: " << p.i.stn_associated_msr_include << endl;
			if (!p.i.stn_associated_msr_exclude.empty())
				cout << setw(PRINT_VAR_PAD) << left << "  Stations to exclude: " << p.i.stn_associated_msr_exclude << endl;
			
			if (p.i.split_clusters)
				cout << setw(PRINT_VAR_PAD) << left << "  Split GNSS clusters: " << (p.i.split_clusters ? "Yes" : "No") << endl;
		}
		
		if (p.i.import_block)
		{
			cout << setw(PRINT_VAR_PAD) << left << "  Segmentation file: " << p.i.seg_file << endl;
			cout << setw(PRINT_VAR_PAD) << left << "  Import stns & msrs from block: " << p.i.import_block_number << endl;
		}

		if (!p.i.scalar_file.empty())
			cout << setw(PRINT_VAR_PAD) << left << "  GNSS baseline scalar file: " << p.i.scalar_file << endl;
		
		cout << endl;
	}

	PrintOutputFileHeaderInfo(&imp_file, p.i.imp_file, &p, "DYNADJUST IMPORT LOG FILE");

	dna_import parserDynaML;
	MsrTally parsemsrTally;
	StnTally parsestnTally;

	CDnaProjection projection(UTM);

	vifm_t vinput_file_meta;

	// First things first!
	// Set the 'default' reference frame for the binary station and measurement files
	try {
		// Initialise the 'default' datum for the project.
		parserDynaML.InitialiseDatum(p.i.reference_frame);
	}
	catch (const XMLInteropException& e) {
		stringstream ss;
		ss << "- Error: ";
		cout << ss.str() << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Import discontinuity file and apply discontinuities
	// Due to the structure and format of SINEX files, it is essential that
	// discontinuities be parsed prior to reading any SINEX files.
	if (vm.count(STATION_DISCONTINUITY_FILE))
	{
		p.i.apply_discontinuities = true;
		
		// Does it exist?
		if (!exists(p.i.stn_discontinuityfile))
		{
			path discontPath(p.i.stn_discontinuityfile);
			stringstream ss;
			ss.str("");
			ss << "- Warning: The station discontinuity file " << discontPath.filename().string() << " does not exist... ignoring discontinuity input." << endl;
			imp_file << endl << ss.str();
		}
		else
		{
			if (!p.g.quiet)
			{
				cout << "+ Importing station discontinuities from " << p.i.stn_discontinuityfile << "... ";
				cout.flush();
			}
			imp_file << "+ Importing station discontinuities from " << p.i.stn_discontinuityfile << "... ";

			parserDynaML.ParseDiscontinuities(p.i.stn_discontinuityfile);

			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done." << endl;
		}

		if (p.i.export_discont_file)
		{
			if (!p.g.quiet)
			{
				cout << "+ Exporting discontinuity information to text file... ";
				cout.flush();
			}
			imp_file << "+ Exporting discontinuity information to text file... ";
			parserDynaML.SerialiseDiscontTextFile(p.i.stn_discontinuityfile);
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
			imp_file << "Done." << endl;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// start "total" time
	cpu_timer time;
	
	// Import network information based on a segmentation block?
	if (p.i.import_block)
	{
		if (ImportSegmentedBlock(parserDynaML, &vstationsTotal, &vmeasurementsTotal, 
			&parsestnTally, &parsemsrTally, p) != EXIT_SUCCESS)
			return EXIT_FAILURE;
	}
	// Import data as normal
	else
	{
		// Import all data as-is.
		// All filtering is performed later below
		if (ImportDataFiles(parserDynaML, &vStations, &vMeasurements, &vstationsTotal, &vmeasurementsTotal,
			&imp_file, &vinput_file_meta, &parsestnTally, &parsemsrTally, errorCount, p) != EXIT_SUCCESS)
			return EXIT_FAILURE;
	}	

	if (!p.g.quiet)
		cout << endl;

	// Now, set the 'default' epoch in the binary station and measurement files
	try {
		// Initialise the 'default' datum for the project.
		parserDynaML.UpdateEpoch(&vinput_file_meta);
	}
	catch (const XMLInteropException& e) {
		stringstream ss;
		ss << "- Error: ";
		cout << ss.str() << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Remove ignored measurements (if supplied)
	if (p.i.remove_ignored_msr)
	{
		if (!p.g.quiet)
		{
			cout << "+ Removing ignored measurements... ";
			cout.flush();
		}
		imp_file << "+ Removing ignored measurements... ";
		parserDynaML.RemoveIgnoredMeasurements(&vmeasurementsTotal, &parsemsrTally);
		if (!p.g.quiet)
			cout << "Done. " << endl;
		imp_file << "Done. " << endl;
	}

	// Strip all measurements except required measurements (if supplied)
	if (!p.i.include_msrs.empty())
	{
		if (!p.g.quiet)
		{
			cout << "+ Stripping all measurements except types " << p.i.include_msrs << "... ";
			cout.flush();
		}
		imp_file << "+ Stripping all measurements except types " << p.i.include_msrs << "... ";
		parserDynaML.IncludeMeasurementTypes(p.i.include_msrs, &vmeasurementsTotal, &parsemsrTally);
		if (!p.g.quiet)
			cout << "Done. " << endl;
		imp_file << "Done. " << endl;
	}
	
	// Strip all unwanted measurements (if supplied)
	if (!p.i.exclude_msrs.empty())
	{
		if (!p.g.quiet)
		{
			cout << "+ Stripping measurement types " << p.i.exclude_msrs << "... ";
			cout.flush();
		}
		imp_file << "+ Stripping measurement types " << p.i.exclude_msrs << "... ";
		parserDynaML.ExcludeMeasurementTypes(p.i.exclude_msrs, &vmeasurementsTotal, &parsemsrTally);
		if (!p.g.quiet)
			cout << "Done. " << endl;
		imp_file << "Done. " << endl;
	}

	// Reduce stations.
	// But, only reduce stations if not importing for a segmentation block
	// The reason stations are not reduced if IMPORT_SEG_BLOCK has been set is
	// because stations in the binary file will have already been reduced
	// on last import!
	if (p.i.import_block == 0 && vstationsTotal.size())
	{
		// reduce stations (e.g. convert from UTM to LLH)
		try {
			if (!p.g.quiet)
			{
				cout << "+ Reducing stations... ";
				cout.flush();
			}
			imp_file << "+ Reducing stations... ";
			parserDynaML.ReduceStations(&vstationsTotal, projection);
			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done." << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	UINT32 stn;

	// Extract user-defined stations and all connected measurements
	if (vstationsTotal.size() &&	// cannot be empty
		(!p.i.stn_associated_msr_include.empty() || !p.i.stn_associated_msr_exclude.empty()))
	{
		vstring vUnusedStns;
		bool splitXmsrs(false), splitYmsrs(false);

		try {
			if (!p.g.quiet)
			{
				cout << "+ Extracting stations and associated measurements... ";
				cout.flush();
			}
			imp_file << "+ Extracting stations and associated measurements... ";

			// this method reforms asl and aml
			parserDynaML.ExtractStnsAndAssociatedMsrs(p.i.stn_associated_msr_include, p.i.stn_associated_msr_exclude, &vstationsTotal, &vmeasurementsTotal, 
				&parsestnTally, &parsemsrTally, &vUnusedStns, p, splitXmsrs, splitYmsrs);
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}

		if (!p.g.quiet)
			cout << "Done. " << endl;
		imp_file << "Done." << endl;

		//msrCount = vmeasurementsTotal.size();
	}	

	// Now strip all stations outside the bounding box (if supplied)
	if (vstationsTotal.size() &&	// cannot be empty
		!p.i.bounding_box.empty())
	{
		vstring vUnusedStns;
		bool splitXmsrs(false), splitYmsrs(false);

		try {
			if (!p.g.quiet)
			{
				cout << "+ Stripping stations and measurements outside the bounding box... ";
				cout.flush();
			}
			imp_file << "+ Stripping stations and measurements outside the bounding box... ";
			parserDynaML.ExcludeAllOutsideBoundingBox(&vstationsTotal, &vmeasurementsTotal, 
				&parsestnTally, &parsemsrTally, &vUnusedStns, p, splitXmsrs, splitYmsrs);
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}

		if (!p.g.quiet)
			cout << "Done. " << endl;
		imp_file << "Done." << endl;

		if (!vUnusedStns.empty())
		{
			ostringstream ss_msg;
			ss_msg << "  - " << vstationsTotal.size() << (vstationsTotal.size() == 1 ? " station is" : " stations are") << 
				" within the bounding box." << endl;
			imp_file << ss_msg.str();
			if (!p.g.quiet)
				cout << ss_msg.str();
			ss_msg.str("");
			ss_msg << "  - ";
			if (vUnusedStns.size() > 1)
			{
				ss_msg << vUnusedStns.size() << " stations were";
			}
			else
				ss_msg << "Station " << vUnusedStns.at(0) << " was";

			ss_msg << " found outside the bounding box and " << (vUnusedStns.size() > 1 ? "have been" : "has been") << endl <<
				"    removed together with the corresponding measurements." << endl;
			if (p.i.split_clusters && (splitXmsrs || splitYmsrs))
			{
				ss_msg << "  - Note: GPS ";
				if (splitXmsrs && splitYmsrs)
					ss_msg << "point and baseline";
				else if (splitXmsrs)
					ss_msg << "baseline";
				else //if (splitYmsrs)
					ss_msg << "point";
				ss_msg << " cluster measurements straddling the limits of" << endl << 
				"    the bounding box have been split." << endl;
			}
			imp_file << ss_msg.str() << "  - Excluded stations:" << endl;
			if (!p.g.quiet)
				cout << ss_msg.str() << endl;
			
			for (stn=0; stn<vUnusedStns.size(); ++stn)
				imp_file << "     " << vUnusedStns.at(stn) << endl;
			imp_file << endl;
		}
		else
		{
			imp_file << "  - No stations were found outside the bounding box." << endl;
			if (!p.g.quiet)
				cout << "  - No stations were found outside the bounding box." << endl;
		}
	}

	UINT32 msrRead(parsemsrTally.TotalCount());
	UINT32 stnCount(static_cast<UINT32>(vstationsTotal.size()));
	UINT32 msrCount(static_cast<UINT32>(vmeasurementsTotal.size()));

	if (!p.g.quiet)
		cout << endl;
	imp_file << endl;

	///////////////////////////////////////////////////////////////////////
	// Ok, now that unwanted stations and measurements have been stripped,
	// provide station and measurement Summary
	//
	if ((stnCount + msrCount) > 0)
	{
		if (p.i.import_block)
		{
			if (!p.g.quiet)
				cout << "+ Binary file ";
			imp_file << "+ Binary file ";
		}
		else
		{
			if (!p.g.quiet)
				cout << "+ File ";
			imp_file << "+ File ";
		}
		
		if (!p.g.quiet)
			cout << "parsing summary:" << endl << endl;
		imp_file << "parsing summary:" << endl << endl;
	}
	
	//
	// Station summary
	if (stnCount)
	{
		if (!p.g.quiet)
		{
			parsestnTally.coutSummary(cout, string("  Read"));
			cout << endl;
		}
		parsestnTally.coutSummary(imp_file, string("  Read"));
		imp_file << endl;
	}
	
	//
	// Measurement summary
	if (msrCount)
	{
		if (!p.g.quiet)
		{
			parsemsrTally.coutSummary(cout, string("  Read"));
			cout << endl;
		}
		parsemsrTally.coutSummary(imp_file, string("  Read"));
		imp_file << endl;
	}

	////////////////////////////////////////////////////////////////////////
	// Can we proceed?
	if (stnCount < 1)
	{
		imp_file << "- No further processing can be done as no stations were loaded." << endl;
		if (!p.g.quiet)
			cout << "- No further processing can be done as no stations were loaded." << endl;
		imp_file.close();
		return PARSE_SUCCESS;
	}

	/////////////////////////////////////////////////////////////////////////
	// Rename stations
	if (p.i.rename_stations && (stnCount > 0 || msrCount > 0))
	{
		// Does it exist?
		if (!exists(p.i.stn_renamingfile))
			// Look for it in the input folder
			p.i.stn_renamingfile = formPath<string>(p.g.input_folder, leafStr<string>(p.i.stn_renamingfile));

		// Apply renaming
		try {
			if (!p.g.quiet)
			{
				cout << "+ Renaming stations... ";
				cout.flush();
			}
			imp_file << "+ Renaming stations... ";
			parserDynaML.RenameStations(&vstationsTotal, &vmeasurementsTotal, &p);
			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done." << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	// Apply GNSS scaling (if required)
	if (p.i.import_block != 1 && p.i.apply_scaling)
	{
		// Apply scaling
		try {
			if (!p.g.quiet)
			{
				cout << "+ Applying scalars to GNSS measurements... ";
				cout.flush();
			}
			imp_file << "+ Applying scalars to GNSS measurements... ";
			parserDynaML.EditGNSSMsrScalars(&vmeasurementsTotal, &p);
			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done." << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	std::ofstream dst_file;

	// flush duplicate stations/measurements files
	try {
		if (exists(p.i.dst_file))
			remove(p.i.dst_file);
		if (exists(p.i.dms_file))
			remove(p.i.dms_file);
	}
	catch (const ios_base::failure& f) { 
		// do nothing on failure.
		imp_file << endl << "- Warning: " << f.what() << endl;
	}

	// Prepare file names if importing from a segmentation block
	if (p.i.import_block)
	{
		stringstream blk("");
		blk << ".block-" << p.i.import_block_number;

		// create new output file names based on block number
		// reform file name for each so as to preserve full path for each file
		stringstream ss("");
		ss << formPath<string>(path(p.i.bst_file).parent_path().generic_string(), path(p.i.bst_file).stem().generic_string());
		ss << blk.str() << ".bst";
		p.i.bst_file = ss.str();

		ss.str("");
		ss << formPath<string>(path(p.i.bms_file).parent_path().generic_string(), path(p.i.bms_file).stem().generic_string());
		ss << blk.str() << ".bms";
		p.i.bms_file = ss.str();

		ss.str("");
		ss << formPath<string>(path(p.i.asl_file).parent_path().generic_string(), path(p.i.asl_file).stem().generic_string());
		ss << blk.str() << ".asl";
		p.i.asl_file = ss.str();

		ss.str("");
		ss << formPath<string>(path(p.i.aml_file).parent_path().generic_string(), path(p.i.aml_file).stem().generic_string());
		ss << blk.str() << ".aml";
		p.i.aml_file = ss.str();

		ss.str("");
		ss << formPath<string>(path(p.i.map_file).parent_path().generic_string(), path(p.i.map_file).stem().generic_string());
		ss << blk.str() << ".map";
		p.i.map_file = ss.str();
	}

	// Remove duplicate stations. If required, test nearby stations
	if (stnCount > 0) {
		
		vstring vduplicateStns;
		v_stringstring_doubledouble_pair vnearbyStns;

		// Remove duplicates and, if required, identify station pairs 
		// separated by distances less than search_stn_radius
		try {
			ostringstream ss_msg;
			if (p.i.search_nearby_stn)
				ss_msg << "+ Testing for duplicate and nearby stations... ";
			else
				ss_msg << "+ Testing for duplicate stations... ";
			
			if (!p.g.quiet)
			{
				cout << ss_msg.str();
				cout.flush();
			}

			imp_file << ss_msg.str();			
			
			stn = parserDynaML.RemoveDuplicateStations(&vstationsTotal, &vduplicateStns, &vnearbyStns);
			
			if (!p.g.quiet)
				cout << "Done. ";
			imp_file << "Done. ";

			if (stn > 0)
			{
				try {
					// Create duplicate station file
					file_opener(dst_file, p.i.dst_file);
				}
				catch (const runtime_error& e) {
					ss_msg << "- Error: Could not open " << p.i.dst_file << ". \n  Check that the file exists and that the file is not already opened." << endl;
					ss_msg << e.what() << endl;
					if (!p.g.quiet)
						cout << ss_msg.str();
					imp_file << ss_msg.str();
					imp_file.close();
					return EXIT_FAILURE;
				}
						
				PrintOutputFileHeaderInfo(&dst_file, p.i.dst_file, &p, "DUPLICATE STATION FILE");

				if (!vduplicateStns.empty())
				{
					ss_msg.str("");
					ss_msg << "Removed " << vduplicateStns.size() << " duplicate station" << (vduplicateStns.size() > 1 ? "s" : "");
					
					// print message to .dst file
					dst_file << ss_msg.str() << ":" << endl;
					for (stn=0; stn<vduplicateStns.size(); ++stn)
						dst_file << "  - " << vduplicateStns.at(stn) << endl;
					dst_file << endl;

					ss_msg << "." << endl << "  See " << p.i.dst_file << " for details." << endl;

					imp_file << ss_msg.str();
					if (!p.g.quiet)
						cout << endl << "- Warning: " << ss_msg.str();					
				}				

				if (!vnearbyStns.empty())
				{
					ss_msg.str("");
					ss_msg << vnearbyStns.size() << (vnearbyStns.size() > 1 ? " pairs of stations were" : " pair of station was") << 
						" found to be separated by less than " << setprecision(3) << p.i.search_stn_radius << "m.";

					imp_file << endl << "- Warning: " << ss_msg.str() << endl << 
						"  See " << p.i.dst_file << " for details." << endl << endl;
					
					if (!p.g.quiet)
						cout << endl << "- Warning: " << ss_msg.str() << endl << 
						"  See " << p.i.dst_file << " for details." << endl << endl;
					ss_msg.str("");

					// output message
					dst_file << "Nearby station search results:" << endl << ss_msg.str() << endl << endl;

					dst_file <<  
						setw(HEADER_20) << left << "First station" << 
						setw(HEADER_20) << "Nearby station" << 
						setw(HEADER_20) << right << "Separation (m)" << 
						setw(HEADER_20) << "Diff height (m)" << 
						endl;

					for (UINT32 i(0); i<(HEADER_20*4); ++i)
						dst_file << "-";
					dst_file << endl;

					// dump nearby stations to dst file
					for (stn=0; stn<vnearbyStns.size(); ++stn)
					{
						dst_file <<
							setw(HEADER_20) << left << vnearbyStns.at(stn).first.first << 									// First
							setw(HEADER_20) << vnearbyStns.at(stn).first.second <<									// Nearby
							setw(HEADER_20) << setprecision(3) << fixed << right << vnearbyStns.at(stn).second.first <<		// Separation (m)
							setw(HEADER_20) << setprecision(3) << fixed << vnearbyStns.at(stn).second.second <<		// Diff height (m)
							endl;
					}

					ss_msg <<
						"  If the names in each pair refer to the same station, then update the " << endl <<
						"  station and measurement files with the correct station name and re-run " << __BINARY_NAME__ << "." << endl <<
						"  Alternatively, if the names in each pair are unique, either call " << __BINARY_NAME__ << endl <<
						"  without the --" << TEST_NEARBY_STNS << " option, or decrease the radial search " << endl <<
						"  distance using --" << TEST_NEARBY_STN_DIST << "." << endl << endl;

					if (!p.g.quiet)
						cout << ss_msg.str();
					imp_file << ss_msg.str();
					imp_file.close();
					dst_file.close();
					return EXIT_FAILURE;
				}
				else if (p.g.verbose == 3)
				{
					if (!p.g.quiet)
						cout << "+ Total number of unique stations is " << vstationsTotal.size() << endl;
					imp_file << "+ Total number of unique stations is " << vstationsTotal.size() << endl;
				}

				// If this line is reached, then there are no nearby stations
				dst_file.close();
			}
			if (!p.g.quiet)
				cout << endl;
			imp_file << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}		
	
		/////////////////////////////////////////////////////////////////////////
		// Add discontinuity sites to vStations
		//
		// WARNING: If discontinuity sites are present in the renaming file, and
		// the renaming file changes the name from the four-character id in the 
		// discontinuity file to another name, this function call will not work
		// properly. Hence, sites in the discontinuity file must be consistently
		// named in the station and measurement files and not renamed to another
		// name.
		if (p.i.apply_discontinuities && stnCount > 0)
			parserDynaML.AddDiscontinuityStations(&vstationsTotal);

		/////////////////////////////////////////////////////////////////
		// Now commence sorting and mapping
		// 1. Sort stations
		// 2. Add new discontinuity sites
		// 3. Create station map
		try {
			if (!p.g.quiet)
			{
				cout << "+ Sorting stations... ";
				cout.flush();
			}
			imp_file << "+ Sorting stations... ";
			parserDynaML.FullSortandMapStations((vdnaStnPtr*) &vstationsTotal, &vStnsMap_sortName);
			if (!p.g.quiet)
			{
				cout << "Done." << endl << "+ Serialising station map... ";
				cout.flush();
			}
			imp_file << "Done." << endl << "+ Serialising station map... ";			
			parserDynaML.SerialiseMap(p.i.map_file);		// parserDynaML keeps the map in memory
			stn_map_created = true;
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
			imp_file << "Done." << endl;

			if (p.i.export_map_file)
			{
				if (!p.g.quiet)
				{
					cout << "+ Exporting station map to text file... ";
					cout.flush();
				}
				imp_file << "+ Exporting station map to text file... ";
				parserDynaML.SerialiseMapTextFile(p.i.map_file);
				if (!p.g.quiet)
				{
					cout << "Done." << endl;
					cout.flush();
				}
				imp_file << "Done." << endl;				
			}
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}	
	}

	// Search for similar measurements
	if (msrCount > 0 && (p.i.search_similar_msr || p.i.search_similar_msr_gx || p.i.ignore_similar_msr)) 
	{
		if (SearchForSimilarMeasurements(&parserDynaML, &p, &imp_file,
			&vstationsTotal, &vmeasurementsTotal) != EXIT_SUCCESS)
			return EXIT_FAILURE;	
	}

	// Import DNA geoid file
	if (p.i.import_geo_file && p.i.import_block == 0 && vstationsTotal.size())
	{
		if (!exists(p.i.geo_file))
		{
			path geoPath(p.i.geo_file);
			stringstream ss;
			ss.str("");
			ss << "- Error: The geoid file " << geoPath.filename().string() << " does not exist." << endl;
			cout << endl << ss.str();
			imp_file << endl << ss.str();
			return EXIT_FAILURE;
		}

		if (!p.g.quiet)
		{
			cout << "+ Importing geoid information from " << p.i.geo_file << "... ";
			cout.flush();
		}
		imp_file << "+ Importing geoid information from " << p.i.geo_file << "... ";

		parserDynaML.LoadDNAGeoidFile(p.i.geo_file, &vstationsTotal);

		if (!p.g.quiet)
			cout << "Done." << endl;
		imp_file << "Done." << endl;
	}

	UINT32 ignMsrCount, mapCount;
	vstring vunusedStations;
	vUINT32 vignoredMeasurements;

	// Map measurements to stations
	if (stn_map_created) 
	{	
		try {
			if (!p.g.quiet)
			{
				cout << "+ Mapping measurements to stations... ";
				cout.flush();
			}
			imp_file << "+ Mapping measurements to stations... ";
			
			parserDynaML.MapMeasurementStations((vdnaMsrPtr*) &vmeasurementsTotal,	&associatedSL, &mapCount, &vunusedStations, &vignoredMeasurements);
			
			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done. ";

			if (msrCount > 0)
			{
				measurements_mapped = true;
				imp_file << "Mapped " << mapCount << " measurements to " << vStnsMap_sortName.size() - vunusedStations.size() << " stations." << endl;
			}
			else
				imp_file << endl;

			if (!vunusedStations.empty())
			{
				if (vunusedStations.size() == 1)
				{
					if (p.g.verbose > 2)
					{
						if (!p.g.quiet)
							cout << "- Warning: station " << vunusedStations.at(0) << " was not associated with any measurements." << endl;
						imp_file << "- Warning: station " << vunusedStations.at(0) << " was not associated with any measurements." << endl;
					}
					else
					{
						if (!p.g.quiet)
							cout << "- Warning: " << vunusedStations.size() << " station was not associated with any measurements." << endl;
						imp_file << "- Warning: " << vunusedStations.size() << " station was not associated with any measurements." << endl;
					}
				}
				else
				{
					if (p.g.verbose > 2)
					{
						if (!p.g.quiet)
							cout << "- Warning: The following " << vunusedStations.size() << " stations were not associated with any measurements." << endl;
						imp_file << "- Warning: The following " << vunusedStations.size() << " stations were not associated with any measurements." << endl;
						_it_vstr unused;
						for (unused = vunusedStations.begin(); unused!=vunusedStations.end(); unused++)
						{
							if (!p.g.quiet)
								outputObject(string("  - " + *unused + "\n"), cout);
							outputObject(string("  - " + *unused + "\n"), imp_file);
						}
					}
					else
					{
						if (!p.g.quiet)
							cout << "- Warning: " << vunusedStations.size() << " stations were not associated with any measurements." << endl;
						imp_file << "- Warning: " << vunusedStations.size() << " stations were not associated with any measurements." << endl;
					}
				}
			}
			if (msrRead < mapCount && vignoredMeasurements.empty())
			{
				if (!p.g.quiet)
					cout << "- Warning: Not all measurements were mapped: " << msrRead << " msrs read vs. " << mapCount
						<< " msrs mapped." << endl;
			}
			else if (msrRead != mapCount && !vignoredMeasurements.empty())
			{
				ignMsrCount = parserDynaML.ComputeMeasurementCount(&vmeasurementsTotal, vignoredMeasurements);	
				if (!p.g.quiet)
				{
					cout << "- Warning: " << ignMsrCount << " ignored measurements were not mapped." << endl;
					if ((msrRead - mapCount) != ignMsrCount)
						cout << "-          " << msrRead << " m.read vs. " << mapCount << " m.mapped." << endl;
				}
			}
			if (!p.g.quiet)
				cout.flush();
		} 
		catch (const XMLInteropException& e) {
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	stnCount = static_cast<UINT32>(vstationsTotal.size());
	if (stnCount > static_cast<UINT32>(associatedSL.size()))
		stnCount = static_cast<UINT32>(associatedSL.size());
	
	// Does the user want to simulate measurements?
	if (p.i.simulate_measurements && stnCount > 0 && msrCount > 0) 
	{
		// No need for the following code if the outer 'if' statement checks 
		// for non-zero stnCount value
		//
		// if (stnCount == 0)
		// {
		// 	if (!p.g.quiet)
		// 		cout << "- Error: there are no stations from which to simulate measurements." << endl;
		// 	imp_file << "- Error: there are no stations from which to simulate measurements." << endl;
		// 	imp_file.close();
		// 	return EXIT_FAILURE;
		// }	
	
		try {
			// Simulate measurements
			if (!p.g.quiet)
			{
				cout << "+ Simulating and exporting measurements to " << leafStr<string>(p.i.simulate_msrfile) << "... ";
				cout.flush();
			}
			imp_file << "+ Simulating and exporting measurements to " << leafStr<string>(p.i.simulate_msrfile) << "... ";
			parserDynaML.SimulateMSR(
				((vdnaStnPtr*) &vstationsTotal), 
				((vdnaMsrPtr*) &vmeasurementsTotal), 
				p.i.simulate_msrfile, p);
			if (!p.g.quiet)
				cout << "Done." << endl;
			imp_file << "Done." << endl;
		}
		catch (const XMLInteropException& e) {
			cout.flush();
			cout << endl << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	// Create association lists
	if (measurements_mapped) 
	{
		try {
			if (!p.g.quiet)
			{
				cout << "+ Creating association lists... ";
				cout.flush();
			}
			imp_file << "+ Creating association lists... ";
			parserDynaML.CompleteAssociationLists(&vmeasurementsTotal, &associatedSL, &associatedML, &vunusedStations, &vignoredMeasurements);
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
			imp_file << "Done." << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}

		try {
			if (!p.g.quiet)
			{
				cout << "+ Serialising association lists... ";
				cout.flush();
			}
			imp_file << "+ Serialising association lists... ";
			parserDynaML.SerialiseAsl(p.i.asl_file, &associatedSL);
			parserDynaML.SerialiseAml(p.i.aml_file, &associatedML);
			if (!p.g.quiet)
			{
				cout << "Done." << endl;
				cout.flush();
			}
			imp_file << "Done." << endl;
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	// Create binary station file.
	// Binary station file can be serialised without measurements
	if (stnCount > 0)
	{
		try {
			if (!vstationsTotal.empty())
			{
				if (!p.g.quiet)
				{
					cout << "+ Serialising binary station file " << leafStr<string>(p.i.bst_file) << "... ";
					cout.flush();
				}
				imp_file << "+ Serialising binary station file " << leafStr<string>(p.i.bst_file) << "... ";
				
				parserDynaML.SerialiseBst(
					p.i.bst_file, ((vdnaStnPtr*) &vstationsTotal), &vunusedStations,
					vinput_file_meta, (p.i.flag_unused_stn ? true : false));
				if (!p.g.quiet)
				{
					cout << "Done." << endl;
					cout.flush();
				}
				imp_file << "Done." << endl;
			}
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	// Create binary measurement file.
	// Binary measurement file can be serialised without stations
	if (msrCount > 0) 
	{
		try {
			if (!vmeasurementsTotal.empty())
			{
				if (!p.g.quiet)
				{
					cout << "+ Serialising binary measurement file " << leafStr<string>(p.i.bms_file) << "... ";
					cout.flush();
				}
				imp_file << "+ Serialising binary measurement file " << leafStr<string>(p.i.bms_file) << "... ";

				parserDynaML.SerialiseBms(
					p.i.bms_file, ((vdnaMsrPtr*) &vmeasurementsTotal),
					vinput_file_meta);
				if (!p.g.quiet)
				{
					cout << "Done." << endl;
					cout.flush();
				}
				imp_file << "Done." << endl;
			}
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}
	}

	// Create ASL and AML files
	// Export to text if required
	if (measurements_mapped) 
	{
		try {
			if (p.i.export_asl_file)
			{
				if (!p.g.quiet)
				{
					cout << "+ Exporting associated station list to text file... ";
					cout.flush();
				}
				imp_file << "+ Exporting associated station list to text file... ";
				parserDynaML.SerialiseAslTextFile(p.i.asl_file, &associatedSL, (vdnaStnPtr*) &vstationsTotal);
				if (!p.g.quiet)
				{
					cout << "Done." << endl;
					cout.flush();
				}
				imp_file << "Done." << endl;				
			}
		} 
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}

		try {
			if (p.i.export_aml_file)
			{
				if (!p.g.quiet)
				{
					cout << "+ Exporting associated measurement list to text file... ";
					cout.flush();
				}
				imp_file << "+ Exporting associated measurement list to text file... ";
				parserDynaML.SerialiseAmlTextFile(p.i.bms_file, p.i.aml_file, &associatedML, &associatedSL, (vdnaStnPtr*) &vstationsTotal);
				if (!p.g.quiet)
				{
					cout << "Done." << endl;
					cout.flush();
				}
				imp_file << "Done." << endl;				
			}
		}
		catch (const XMLInteropException& e) {
			cout << "- Error: " << e.what() << endl;
			imp_file << endl << "- Error: " << e.what() << endl;
			imp_file.close();
			return EXIT_FAILURE;
		}

		if (p.i.test_integrity)
		{
			if (!p.g.quiet)
			{
				cout << "+ Testing internal integrity of ASL, AML and binary files... ";
				cout.flush();
			}
			std::ifstream binaryMS(p.i.bms_file.c_str(), ios::in | ios::binary | std::ifstream::ate);	/// Open and seek to end immediately after opening.
			if (!binaryMS.good())
			{
				cout << endl << "- Could not open binary file for reading." << endl;
				imp_file << endl << "- Could not open binary file for reading." << endl;
				imp_file.close();
				return EXIT_FAILURE;
			}
			// get size, then go back to beginning
			size_t sFileSize = (size_t)binaryMS.tellg();
			binaryMS.seekg(0, ios::beg);

			measurement_t measRecord;
			
			UINT32 amlcount, amlindex, msr;
			for (stn=0; stn<stnCount; stn++)
			{
				amlcount = associatedSL.at(stn).get()->GetAssocMsrCount();
				
				// The following is to be used when reading from binary file
				for (msr=0; msr<amlcount; msr++)
				{
					amlindex = associatedSL.at(stn).get()->GetAMLStnIndex() + msr;
					if (associatedML.at(amlindex) * sizeof(measurement_t) >= sFileSize)
					{
						cout << "Error: index " << associatedML.at(amlindex) << " is out of range for the binary file." << endl;
						continue;
					}
					binaryMS.seekg(sizeof(UINT32) + associatedML.at(amlindex) * sizeof(measurement_t), ios::beg);
					binaryMS.read(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
				}
			}
			binaryMS.close();
			if (!p.g.quiet)
				cout << "OK." << endl;
		}
	}

	try {
		// Print measurements to stations table
		PrintMeasurementstoStations(&parsemsrTally, &parserDynaML, &p, &associatedSL);
	}
	catch (const XMLInteropException& e) {
		cout.flush();
		cout << endl << "- Error: " << e.what() << endl;
		imp_file << endl << "- Error: " << e.what() << endl;
		imp_file.close();
		return EXIT_FAILURE;
	}

	// Serialise database ids
	string dbid_file = formPath<string>(p.g.output_folder, p.g.network_name, "dbid");
	parserDynaML.SerialiseDatabaseId(dbid_file, &vmeasurementsTotal);

	// Export stations and measurements
	try {
		if (p.i.export_dynaml || p.i.export_dna_files)
			ExportStationsandMeasurements(&parserDynaML, p, &imp_file, &vinput_file_meta, 
				&vstationsTotal, &vmeasurementsTotal, stnCount, msrCount);
	}
	catch (const XMLInteropException& e) {
		cout.flush();
		cout << endl << "- Error: " << e.what() << endl;
		imp_file << endl << "- Error: " << e.what() << endl;
		imp_file.close();
		return EXIT_FAILURE;
	}
	
	if (!userSuppliedSegFile)
		p.i.seg_file = "";
	if (!userSuppliedBstFile)
		p.i.bst_file = "";
	if (!userSuppliedBmsFile)
		p.i.bms_file = "";

	// Look for a project file.  If it exists, open and load it.
	// Update the import settings.
	// Print the project file. If it doesn't exist, it will be created.
	CDnaProjectFile projectFile;
	if (exists(p.g.project_file))
		projectFile.LoadProjectFile(p.g.project_file);
	
	projectFile.UpdateSettingsImport(p);
	projectFile.UpdateSettingsOutput(p);
	projectFile.PrintProjectFile();

	if (msrCount == 0)
	{
		cout << "- Warning: there are no measurements to process." << endl;
		imp_file << "- Warning: there are no measurements to process." << endl;
	}
	
	if (errorCount)
	{
		cout << "- Warning: some files were not parsed - please read the log file for more details." << endl;
		imp_file << "- Warning: some files were not parsed - please read the log file for more details." << endl;
	}	
	
	milliseconds elapsed_time(milliseconds(time.elapsed().wall/MILLI_TO_NANO));
	string time_message = formatedElapsedTime<string>(&elapsed_time, "+ Total file handling process took ");

	if (!p.g.quiet)
		cout << endl << time_message << endl;
	imp_file << endl << time_message << endl;
	
	if (stnCount > 0 && msrCount > 0)
	{
		if (!p.g.quiet)
			cout << "+ Binary station and measurement files are now ready for processing." << endl << endl;
		imp_file << "+ Binary station and measurement files are now ready for processing." << endl << endl;
	}

	imp_file.close();
	return PARSE_SUCCESS;
}
