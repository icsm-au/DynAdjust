//============================================================================
// Name         : dnaoptions.hpp
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
// Description  : DynAdjust options include file
//============================================================================

#ifndef DNAOPTIONS_HPP
#define DNAOPTIONS_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <boost/operators.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnatypes.hpp>

const bool FREE = true;
const bool NOTFREE = false;

//const UINT16 project_var_width(30);
//const char* const UNDERLINE = "#-----------------------------------------------------------";

//const UINT16 adjustCount(4);

enum adjustMode
{
	SimultaneousMode = 0,
	PhasedMode = 1,
	Phased_Block_1Mode = 2,
	SimulationMode = 3
};

enum adjustOperation
{
	__forward__ = 0,
	__reverse__ = 1,
	__combine__ = 2
};


enum inverseMethod
{
	Cholesky = 0,
	Gaussian = 1,
	Sweep = 2,
	Cholesky_mkl = 3
};


enum settingMode
{
	unknownSetting = 0,
	switchSetting = 1,
	generalSetting = 2,
	reftranSetting = 3,
	geoidSetting = 4,
	importSetting = 5,
	segmentSetting = 6,
	adjustSetting = 7,
	outputSetting = 8,
	plotSetting = 9,
	displaySetting = 10
};

enum printMeasurementsMode
{
	adjustedMsrs = 0,
	computedMsrs = 1,
	ignoredMsrs = 2
};

enum plotGraphMode
{
	StationsMode = 0,
	MeasurementsMode = 1
};

const char* const projectionTypes[] = { 
	"Best", 
	"World plot", 
	"Orthographic", 
	"Mercator", 
	"Transverse Mercator" , 
	"Albers equal-area conic" , 
	"Lambert conformal conic" , 
	"General stereographic" 
};

enum plotProjection
{
	bestProjection = 0,
	world = 1,
	orthographic = 2,
	mercator = 3,
	transverseMercator = 4,
	albersConic = 5,
	lambertEqualArea = 6,
	stereographicConformal = 7
};

const char* const coastResolutionTypes[] = { 
	"Crude", 
	"Low", 
	"Intermediate", 
	"High", 
	"Full" 
};

enum coastResolution
{
	crude = 0,
	low = 1,
	intermediate = 2,
	high = 3,
	full = 4
};

using namespace boost;

// general settings used by all programs
struct general_settings : private boost::equality_comparable<general_settings> {
public:
	general_settings()
		: quiet(0), verbose(0), version(0), interactive(0)
		, log_file("dynadjust.log"), project_file("network1.dnaproj")
		, network_name("network1"), input_folder("."), output_folder(".") { 
	}

	// Ensure operator== methods for all settings test *all* member variables (required for project dialog)
	bool operator==(const general_settings& g) const {
		return quiet == g.quiet && verbose == g.verbose && version == g.version && interactive == g.interactive &&
			log_file == g.log_file &&
			project_file == g.project_file && network_name == g.network_name &&
			input_folder == g.input_folder && output_folder == g.output_folder;
	}

	UINT16					quiet;				// Suppresses all explanation of what dnainterop is doing unless an error occurs
	UINT16					verbose;			// Give detailed information about what dnainterop is doing.\n0: No information (default)\n1: Helpful information\n2: Extended information\n3: Debug level information
	UINT16					version;			// Display the current program version
	UINT16      			interactive;		// whether to display a Qt Dialog or continue with command window
	string					log_file;			// dynadjust log filename
	string					project_file;		// project filename
	string					network_name;		// network name
	string					input_folder;		// Additional include folder containing input files (used for command line help only)
	string					output_folder;		// Path for all output (intermediate) files
	v_string_string_pair	variables;			// Shortcut tags to assist with file naming
};

// data import settings
struct import_settings : private boost::equality_comparable<import_settings> {
public:
	import_settings()
		: reference_frame(DEFAULT_DATUM), user_supplied_frame(0), override_input_rfame(0)
		, test_integrity(0), verify_coordinates(0), export_dynaml(0), export_from_bfiles(0)
		, export_single_xml_file(0), prefer_single_x_as_g(0), export_asl_file(0), export_aml_file(0), export_map_file(0)
		, export_dna_files(0), export_discont_file(0), import_geo_file(0), simulate_measurements(0), split_clusters(0), include_transcending_msrs(0)
		, apply_scaling(0), map_file(""), asl_file(""), aml_file(""), bst_file(""), bms_file("")
		, dst_file(""), dms_file(""), imp_file(""), geo_file(""), seg_file("")
		, xml_outfile(""), xml_stnfile(""), xml_msrfile(""), dna_stnfile(""), dna_msrfile(""), stn_renamingfile("")
		, stn_discontinuityfile(""), simulate_msrfile("")
		, include_msrs(""), exclude_msrs(""), bounding_box(""), stn_associated_msr_include(""), stn_associated_msr_exclude("")
		, rename_stations(0), apply_discontinuities(0), search_nearby_stn(0)
		, search_similar_msr(0), search_similar_msr_gx(0), ignore_similar_msr(0), remove_ignored_msr(0)
		, flag_unused_stn(0), import_block(0), import_block_number(0)
		, cluster_id(0), msr_id(0), search_stn_radius(STN_SEARCH_RADIUS)
		, vscale(1.), pscale(1.), lscale(1.), hscale(1.), scalar_file("")
		, command_line_arguments("") 
	{
			input_files.clear();
	}

	// The following equality function is used in the GUI to test whether any settings have been modified
	bool operator==(const import_settings& i) const {
		return reference_frame == i.reference_frame && user_supplied_frame == i.user_supplied_frame && override_input_rfame == i.override_input_rfame &&
			test_integrity == i.test_integrity && verify_coordinates == i.verify_coordinates && export_dynaml == i.export_dynaml &&
			export_from_bfiles == i.export_from_bfiles && export_single_xml_file == i.export_single_xml_file && prefer_single_x_as_g ==i.prefer_single_x_as_g &&
			export_asl_file == i.export_asl_file && export_aml_file == i.export_aml_file && export_map_file == i.export_map_file &&
			export_dna_files == i.export_dna_files && export_discont_file == i.export_discont_file && import_geo_file == i.import_geo_file &&
			simulate_measurements == i.simulate_measurements && split_clusters == i.split_clusters && include_transcending_msrs == i.include_transcending_msrs && 
			apply_scaling == i.apply_scaling && input_files == i.input_files &&
			map_file == i.map_file && asl_file == i.asl_file && aml_file == i.aml_file && bst_file == i.bst_file && bms_file == i.bms_file &&
			dst_file == i.dst_file && dms_file == i.dms_file && imp_file == i.imp_file && geo_file == i.geo_file && seg_file == i.seg_file && 
			xml_outfile == i.xml_outfile && xml_stnfile == i.xml_stnfile && xml_msrfile == i.xml_msrfile && 
			dna_stnfile == i.dna_stnfile && dna_msrfile == i.dna_msrfile && simulate_msrfile == i.simulate_msrfile &&
			include_msrs == i.include_msrs && exclude_msrs == i.exclude_msrs &&
			stn_associated_msr_include == i.stn_associated_msr_include && stn_associated_msr_exclude == i.stn_associated_msr_exclude && rename_stations == i.rename_stations &&
			search_nearby_stn == i.search_nearby_stn && search_similar_msr == i.search_similar_msr && search_similar_msr_gx == i.search_similar_msr_gx &&
			ignore_similar_msr == i.ignore_similar_msr && remove_ignored_msr == i.remove_ignored_msr &&
			flag_unused_stn == i.flag_unused_stn && import_block == i.import_block && import_block_number == i.import_block_number &&
			cluster_id == i.cluster_id && msr_id == i.msr_id &&
			vscale == i.vscale && pscale == i.pscale && lscale == i.lscale && hscale == i.hscale && scalar_file == i.scalar_file;
	}

	string		reference_frame;			// Default reference frame - used primarily for reductions on the ellipsoid.
	UINT16		user_supplied_frame;		// User has supplied a frame - use this to change the default frame
	UINT16		override_input_rfame;		// Override reference frame specified in input files using the default or user supplied frame.
	UINT16		test_integrity;				// Test integrity of network
	UINT16		verify_coordinates;			// Test integrity of coordinates
	UINT16		export_dynaml;				// Create DynaML output file
	UINT16		export_from_bfiles;			// Create DynaML output file using binary files. Default option uses internal memory
	UINT16		export_single_xml_file;		// Create separate station and measurement DynaML output files
	UINT16		prefer_single_x_as_g;		// Prefer single baseline cluster measurements (X) as single baseline measurements (G)
	UINT16		export_asl_file;			// Create a text file of the ASL
	UINT16		export_aml_file;			// Create a text file of the AML
	UINT16		export_map_file;			// Create a text file of the MAP
	UINT16		export_dna_files;			// Create a DynAdjust STN and MSR files
	UINT16		export_discont_file;		// Create a text file of the discontinuity information
	UINT16		import_geo_file;			// Import DNA geoid file
	UINT16		simulate_measurements;		// Simulate exact measurements corresponding to the input measurements using the coordinates in the station file
	UINT16		split_clusters;				// Allow bounding box or station selection to split GNSS point and baseline clusters.
	UINT16		include_transcending_msrs;	// Include measurements straddling bounding box.
	UINT16		apply_scaling;				// Apply scaling?
	string		map_file;					// Station map output file
	string		asl_file;					// Associated stations output file
	string		aml_file;					// Associated measurements output file
	string		bst_file;					// Binary station output file
	string		bms_file;					// Binary measurement output file
	string		dst_file;					// Duplicate station output file
	string		dms_file;					// Duplicate measurement output file
	string		imp_file;					// import log file
	string		geo_file;					// Geoid file to use on import
	string		seg_file;					// Segmentation input file
	string		xml_outfile;				// Create DynaML output file (combined stn and msr)
	string		xml_stnfile;				// DynaML station file
	string		xml_msrfile;				// DynaML measurement file
	string		dna_stnfile;				// DNA station file
	string		dna_msrfile;				// DNA measurement file
	string		stn_renamingfile;			// Station renaming file
	string		stn_discontinuityfile;		// Station discontinuity file (SINEX format)
	string		simulate_msrfile;			// Simulation control file
	string		include_msrs;				// Import the measurements corresponding to the user-supplied string of measurement types
	string		exclude_msrs;				// Exclude the measurements corresponding to the user-supplied string of measurement types
	string		bounding_box;				// Import stations and measurements within bounding box using comma delimited string \"lat1,lon1,lat2,lon2\" to define upper-left and lower-right limits.
	string		stn_associated_msr_include;	// Include stations and all associated measurements. arg is a comma delimited string \"stn 1,stn 2,stn 3,...,stn N\" of the stations to include.
	string		stn_associated_msr_exclude;	// Exclude stations and all associated measurements. arg is a comma delimited string \"stn 1,stn 2,stn 3,...,stn N\" of the stations to exclude.
	UINT16		rename_stations;			// Rename stations using a station renaming file
	UINT16		apply_discontinuities;		// Rename stations using discontinuities and measurement epochs
	UINT16		search_nearby_stn;			// Search for nearby stations
	UINT16		search_similar_msr;			// Search and provide warnings for duplicate measurements
	UINT16		search_similar_msr_gx;		// Search and provide warnings for duplicate GX measurements
	UINT16		ignore_similar_msr;			// Ignore duplicate measurements
	UINT16		remove_ignored_msr;			// Remove ignored measurements
	UINT16		flag_unused_stn;			// Mark unused stations in binary file.  Stations marked will be excluded from any further processing.
	UINT16		import_block;				// Import stations and measurements from a segmented block
	UINT32		import_block_number;		// Import from this block
	UINT32		cluster_id;					// Index of the first available cluster id 
	UINT32		msr_id;						// Index of the first available measurement id
	double		search_stn_radius;			// Radius of the circle within which to search for nearby stations
	double		vscale;						// global matrix scalar
	double		pscale;						// phi scalar
	double		lscale;						// lambda scalar
	double		hscale;						// height scalar
	string		scalar_file;				// scalar file (individual scalars defined for measurements between specific stations)
	string		command_line_arguments;
	vstring		input_files;				// Default input arguments if no switch is provided.
};

// datum settings
struct reftran_settings : private boost::equality_comparable<reftran_settings> {
public:
	reftran_settings()
		: bst_file(""), bms_file("")
		, reference_frame(DEFAULT_DATUM), epoch("")
		, tpb_file(""), tpp_file("")
		, plate_model_option(0)
		, command_line_arguments("") {}

	bool operator==(const reftran_settings& r) const {
		return bst_file == r.bst_file && bms_file == r.bms_file &&
			reference_frame == r.reference_frame && epoch == r.epoch;
	}

	string		bst_file;					// Binary station output file
	string		bms_file;					// Binary measurement output file
	string		reference_frame;			// Reference frame for all stations and measurements.  Requires datum.conf and ellipsoid.conf.
	string		epoch;						// Epoch
	string		tpb_file;					// Tectonic plate boundary file
	string		tpp_file;					// Tectonic plate pole file
	UINT16		plate_model_option;			// Informs reftran which plate model option to use
	string		command_line_arguments;
};

// geoid settings
struct geoid_settings : private boost::equality_comparable<geoid_settings> {
public:
	geoid_settings()
		: file_mode(0), interpolation_method(1), ellipsoid_to_ortho(0), coordinate_format(DMS)
		, convert_heights(1), export_dna_geo_file(0)
		, rdat_geoid_file(""), ntv2_geoid_file(""), input_file(""), bst_file(""), geo_file("")
		, command_line_arguments("") {}

	bool operator==(const geoid_settings& n) const {
		return interpolation_method == n.interpolation_method &&
			ellipsoid_to_ortho == n.ellipsoid_to_ortho &&
			coordinate_format == n.coordinate_format &&
			convert_heights == n.convert_heights &&
			export_dna_geo_file == n.export_dna_geo_file &&
			rdat_geoid_file == n.rdat_geoid_file && 
			ntv2_geoid_file == n.ntv2_geoid_file &&
			input_file == n.input_file &&
			bst_file == n.bst_file;
	}

	UINT16		file_mode;						// file mode
	UINT16		interpolation_method;			// bi-linear/bi-cubic
	UINT16		ellipsoid_to_ortho;				// direction of conversion of heights
	UINT16		coordinate_format;				// ddeg or dms (default)
	UINT16		convert_heights;				// convert ortho binary station heights to ellipsoidal
	UINT16		export_dna_geo_file;			// Export geoid information in DNA geoid file
	string		rdat_geoid_file;				// raw dat geoid file
	string		ntv2_geoid_file;				// ntv2 geoid file
	string		input_file;						// input file
	string		bst_file;						// bst file
	string		geo_file;						// dna geo file
	string		command_line_arguments;
};

// network segmentation settings
struct segment_settings : private boost::equality_comparable<segment_settings> {
public:
	segment_settings()
		: test_integrity(0), min_inner_stations(5), max_total_stations(65), seg_search_level(0)
		, display_block_network(1), view_block_on_segment(1), show_segment_summary(0), print_segment_debug(0)
		, force_contiguous_blocks(1), map_file(""), asl_file(""), aml_file("")
		, bst_file(""), bms_file(""), seg_file(""), sap_file(""), net_file(""), seg_starting_stns("") 
		, command_line_arguments("") {}

	bool operator==(const segment_settings& s) const {
		return test_integrity == s.test_integrity && min_inner_stations == s.min_inner_stations && 
			max_total_stations == s.max_total_stations && seg_search_level == s.seg_search_level &&
			display_block_network == s.display_block_network && view_block_on_segment == s.view_block_on_segment &&
			show_segment_summary == s.show_segment_summary && print_segment_debug == s.print_segment_debug &&
			map_file == s.map_file && asl_file == s.asl_file && aml_file == s.aml_file && bst_file == s.bst_file && bms_file == s.bms_file &&
			seg_file == s.seg_file && sap_file == s.sap_file &&
			net_file == s.net_file && seg_starting_stns == s.seg_starting_stns;
	}

	UINT16		test_integrity;				// Test integrity of network
	UINT16		min_inner_stations;			// Minumum number of inner stations per block
	UINT16		max_total_stations;			// Maxumum number of total stations per block
	UINT16		seg_search_level;			// Level to which searches should be conducted to look for lowest station count
	UINT16		display_block_network;		// display block/network in GUI
	UINT16		view_block_on_segment;		// view blocks after segmentation
	UINT16		show_segment_summary;		// show segmentation summary dialog
	UINT16		print_segment_debug;		// print segmentation debug information
	UINT16		force_contiguous_blocks;	// force contiguous blocks
	string		map_file;					// Station map file
	string		asl_file;					// Associated stations file
	string		aml_file;					// Associated measurements file
	string		bst_file;					// Binary station file
	string		bms_file;					// Binary measurement file
	string		seg_file;					// Segmentation output file
	string		sap_file;					// Station appearance list file
	string		net_file;					// Starting stations output file
	string		seg_starting_stns;			// Stations to be incorporated within the first block.
	string		command_line_arguments;
};

// network segmentation settings
struct adjust_settings : private boost::equality_comparable<adjust_settings> {
public:
	adjust_settings()
		: adjust_mode(SimultaneousMode)
		, inverse_method_msr(Cholesky_mkl), inverse_method_lsq(Cholesky_mkl)
		, max_iterations(10), confidence_interval(95.0), report_mode(false), multi_thread(false), stage(false), scale_normals_to_unity(false)
		, purge_stage_files(false), recreate_stage_files(false)
		, iteration_threshold((float)0.0005), free_std_dev(10.0), fixed_std_dev(PRECISION_1E6), station_constraints("")
		, map_file(""), bst_file(""), bms_file(""), seg_file(""), comments("") 
		, command_line_arguments("") {}

	bool operator==(const adjust_settings& a) const {
		return adjust_mode == a.adjust_mode && 
			inverse_method_msr == a.inverse_method_msr && inverse_method_lsq == a.inverse_method_lsq && 
			max_iterations == a.max_iterations && confidence_interval == a.confidence_interval && 
			report_mode == a.report_mode && multi_thread == a.multi_thread && stage == a.stage && 
			iteration_threshold == a.iteration_threshold && 
			purge_stage_files == a.purge_stage_files && recreate_stage_files == a.recreate_stage_files &&
			free_std_dev == a.free_std_dev && fixed_std_dev == a.fixed_std_dev &&
			map_file == a.map_file && bst_file == a.bst_file && bms_file == a.bms_file &&
			seg_file == a.seg_file && comments == a.comments;
	}

	inline void setFilenames(const string& name) {
		map_file = name + ".map";
		bst_file = name + ".bst";
		bms_file = name + ".bms";
		seg_file = name + ".seg";
	}

	UINT16		adjust_mode;			// Network adjustment mode
											// 0 Simultaneous
											// 1 Phased
											// 2 Phased (Block 1)
											// 3 Simulation
	UINT16		inverse_method_msr;		// Inverse method for measurement variances
	UINT16		inverse_method_lsq;		// Inverse method for solution of normal equations
	UINT16		max_iterations;			// Maximum number of iterations
	float		confidence_interval;	// Confidence interval
	UINT16		report_mode;			// Print results only
	UINT16		multi_thread;			// Use multi threading for phased adjustment?
	UINT16		stage;					// Instead of loading all phased adjustment blocks in memory, load only the information required for the current block adjustment and 
	UINT16		scale_normals_to_unity;	// Scale normals to unity prior to inversion
	bool		purge_stage_files;		// Purge memory mapped files from disk upon adjustment completion.
	UINT16		recreate_stage_files;	// Recreate memory mapped files.
	float		iteration_threshold;	// Convergence limit
	double		free_std_dev;			// SD for free stations
	double		fixed_std_dev;			// SD for fixed stations
	string		station_constraints;	// Station constraints.  Comma delimited string.
	string		map_file;				// Station map file
	string		bst_file;				// Binary station file
	string		bms_file;				// Binary measurement file
	string		seg_file;				// Segmentation file
	string		comments;				// General comments about the adjustment, printed to the adj file.
	string		command_line_arguments;
};

// datum and geoid settings
struct output_settings : private boost::equality_comparable<output_settings> {
public:
	output_settings()
		: _datum(DEFAULT_DATUM), _m2s_file(""), _adj_file(""), _xyz_file("")
		, _snx_file(""), _xml_file(""), _cor_file(""), _apu_file("")
		, _adj_stn_iteration(0), _adj_msr_iteration(0), _cmp_msr_iteration(0), _adj_stat_iteration(0)
		, _adj_msr_final(0), _adj_msr_tstat(0), _database_ids(0), _print_ignored_msrs(0), _adj_gnss_units(0)
		, _output_stn_blocks(0), _output_msr_blocks(0), _sort_stn_file_order(0), _sort_adj_msr(0)
		, _init_stn_corrections(0), _msr_to_stn(0), _stn_corr(0), _positional_uncertainty(0)
		, _apu_vcv_units(0), _output_pu_covariances(0)
		, _export_xml_stn_file(0), _export_dna_stn_file(0), _export_snx_file(false)
		, _hz_corr_threshold(0.0), _vt_corr_threshold(0.0)
		, _stn_coord_types("PLHhXYZ"), _angular_type_stn(DMS)
		, _precision_seconds_stn(5), _precision_metres_stn(4), _precision_seconds_msr(4), _precision_metres_msr(4)
		, _angular_type_msr(DMS), _dms_format_msr(SEPARATED)
		
	{}

	bool operator==(const output_settings& o) const {
		return _datum == o._datum && _adj_file == o._adj_file && _xyz_file == o._xyz_file &&
			 _snx_file == o._snx_file && _xml_file == o._xml_file &&
			_cor_file == o._cor_file && _apu_file == o._apu_file && _m2s_file == o._m2s_file &&
			_adj_stn_iteration == o._adj_stn_iteration && _adj_msr_iteration == o._adj_msr_iteration && 
			_cmp_msr_iteration == o._cmp_msr_iteration && _adj_stat_iteration == o._adj_stat_iteration && 
			_output_stn_blocks == o._output_stn_blocks && _output_msr_blocks == o._output_msr_blocks &&
			_sort_stn_file_order == o._sort_stn_file_order && 
			_adj_msr_final == o._adj_msr_final && _adj_msr_tstat == o._adj_msr_tstat &&
			_database_ids == o._database_ids && _print_ignored_msrs == o._print_ignored_msrs &&
			_sort_adj_msr == o._sort_adj_msr &&
			_adj_gnss_units == o._adj_gnss_units &&
			_init_stn_corrections == o._init_stn_corrections && _msr_to_stn == o._msr_to_stn &&
			_output_pu_covariances == o._output_pu_covariances &&
			_apu_vcv_units == o._apu_vcv_units &&
			_hz_corr_threshold == o._hz_corr_threshold && _vt_corr_threshold == o._vt_corr_threshold && 
			_stn_coord_types == o._stn_coord_types && _angular_type_stn == o._angular_type_stn &&
			_stn_corr == o._stn_corr && _positional_uncertainty == o._positional_uncertainty &&
			_precision_metres_stn == o._precision_metres_stn && _precision_seconds_stn == o._precision_seconds_stn && 
			_precision_metres_msr == o._precision_metres_msr && _precision_seconds_msr == o._precision_seconds_msr &&
			_angular_type_msr == o._angular_type_msr && _dms_format_msr == o._dms_format_msr &&
			_export_xml_stn_file == o._export_xml_stn_file &&
			_export_dna_stn_file == o._export_dna_stn_file &&
			_export_snx_file == o._export_snx_file;
	}

	string			_datum;					// Datum switch.  Requires datum.conf and ellipsoid.conf.
	string			_m2s_file;				// Measurement to stations file
	string			_adj_file;				// Adjustment output
	string			_xyz_file;				// Adjusted coordinate output
	string			_snx_file;				// Adjusted coordinate output in SINEX format
	string			_xml_file;				// Estimated station coordinates and full variance matrix in DynaML (DynAdjust XML) format. Uses Y cluster.
	string			_cor_file;				// Corrections to intial stations output
	string			_apu_file;				// Adjusted positional uncertainty output
	UINT16			_adj_stn_iteration;		// Outputs adjusted stations for each block within each iteration
	UINT16			_adj_msr_iteration;		// Outputs adjusted measurements for each block within each iteration
	UINT16			_cmp_msr_iteration;		// Outputs computed measurements for each block within each iteration
	UINT16			_adj_stat_iteration;	// Outputs statistical summary for each block within each iteration
	UINT16			_adj_msr_final;			// Outputs final adjusted measurements
	UINT16			_adj_msr_tstat;			// Outputs tstats for adjusted measurements
	UINT16			_database_ids;			// Output msr id and cluster id
	UINT16			_print_ignored_msrs;	// Output adjusted measurement statistics for ignored measurements
	UINT16			_adj_gnss_units;		// Units of output adjusted GNSS measurements
	UINT16			_output_stn_blocks;		// For phased adjustments, output station information for each segmented block.
	UINT16			_output_msr_blocks;		// For phased adjustments, output adjusted measurement information for each segmented block.
	UINT16			_sort_stn_file_order;	// Outputs stations in original station file sort order
	UINT16			_sort_adj_msr;			// Field by which adjusted measurements are sorted
	UINT16			_init_stn_corrections;	// Output corrections (azimuth, distance, e, n, up) to initial station coordinates
	UINT16			_msr_to_stn;			// Output summary of measurements connected to each station
	UINT16			_stn_corr;				// Output station corrections to station file.
	UINT16			_positional_uncertainty;	// Output covariances between adjusted station coordinates to .apu file.
	UINT16			_apu_vcv_units;			// Units of the VCV elements to be printed to the apu file
	UINT16			_output_pu_covariances;	// Output covariances between adjusted station coordinates to .apu file.
	UINT16			_export_xml_stn_file;	// Create a DynaML stn file
	UINT16			_export_dna_stn_file;	// Create a DNA stn file
	UINT16			_export_snx_file;		// Create a sinex file from the adjustment
	double			_hz_corr_threshold;		// Minimum horizontal threshold for station corrections
	double			_vt_corr_threshold;		// Minimum vertical threshold for station corrections
	string			_stn_coord_types;		// String defining the cooridnate types to be printed for each station
	UINT16			_angular_type_stn;		// Type of angular station coordinates (dms or ddeg)
	UINT16			_precision_seconds_stn;	// Precision of angular station values given in seconds
	UINT16			_precision_metres_stn;	// Precision of linear station values given in seconds
	UINT16			_precision_seconds_msr;	// Precision of angular measurement values given in seconds
	UINT16			_precision_metres_msr;	// Precision of linear measurement values given in seconds
	UINT16			_angular_type_msr;		// Type of angular measurements (dms or ddeg)
	UINT16			_dms_format_msr;		// Format of dms measurements (sep fields or hp notation)
};

// plot settings
struct plot_settings : private boost::equality_comparable<plot_settings> {
public:
	plot_settings()
		: _projection(bestProjection), _gmt_cmd_file(""), _gnuplot_cmd_file(""), _pdf_cmd_file(""), _eps_file_name(""), _pdf_file_name("")
		, _plot_phased_blocks(false), _plot_station_labels(false), _plot_ignored_msrs(false)
		, _plot_alt_name(false), _plot_station_constraints(false)
		, _plot_correction_arrows(false), _plot_correction_labels(false), _compute_corrections(false)
		, _plot_positional_uncertainty(false), _plot_error_ellipses(false)
		, _user_defined_projection(false), _omit_title_block(false), _omit_measurements(false)
		, _use_pdflatex(false), _keep_gen_files(false)
		, _label_font_size(5.0), _msr_line_width(0.15), _correction_scale(1.), _pu_ellipse_scale(1.)
		, _plot_station_centre(""), _bounding_box("")
		, _plot_area_radius(5000.000), _plot_centre_latitude(-999.), _plot_centre_longitude(-999.)
		, _plot_scale(0.), _page_width(0.), _ground_width(0.)
		, _plot_block_number(0), _coasline_resolution(low)
		, _pdf_viewer("Acrobat"), _acrobat_ddename("AcroViewR15")
		, command_line_arguments("")
	{
		_gmt_params.clear();
		_separate_msrs.clear();
		_msr_colours.clear();
	}

	bool operator==(const plot_settings& p) const {
		return _projection == p._projection && _gmt_params == p._gmt_params && _separate_msrs == p._separate_msrs && _msr_colours == p._msr_colours &&
			_gmt_cmd_file == p._gmt_cmd_file && _gnuplot_cmd_file == p._gnuplot_cmd_file && _pdf_cmd_file == p._pdf_cmd_file && _eps_file_name == p._eps_file_name &&
			_pdf_file_name == p._pdf_file_name && _plot_phased_blocks == p._plot_phased_blocks && _use_pdflatex == p._use_pdflatex && 
			_plot_station_labels == p._plot_station_labels && _plot_ignored_msrs == p._plot_ignored_msrs &&
			_plot_alt_name ==p._plot_alt_name && _plot_station_constraints == p._plot_station_constraints &&
			_plot_correction_arrows == p._plot_correction_arrows && _plot_correction_labels == p._plot_correction_labels && 
			_compute_corrections == p._compute_corrections &&
			_plot_positional_uncertainty == p._plot_positional_uncertainty && _plot_error_ellipses == p._plot_error_ellipses && 
			_user_defined_projection == p._user_defined_projection && _omit_title_block == p._omit_title_block &&
			_omit_measurements == p._omit_measurements &&
			_label_font_size == p._label_font_size && _msr_line_width == p._msr_line_width &&
			_correction_scale == p._correction_scale && _pu_ellipse_scale == p._pu_ellipse_scale &&
			_bounding_box == p._bounding_box && _plot_station_centre == p._plot_station_centre && _plot_area_radius == p._plot_area_radius && 
			_plot_centre_latitude == p._plot_centre_latitude && _plot_centre_longitude == p._plot_centre_longitude && _plot_scale == p._plot_scale &&
			_plot_block_number == p._plot_block_number && _keep_gen_files == p._keep_gen_files;
	}

	UINT16					_projection;					// Projection for the plot
															//	0 Allow plot to determine the projection from the data spatial extents
															//	1 World plot
															//	2 Orthographic (globe plot)
															//	3 Mercator
															//	4 Transverse Mercator
															//	5 Albers conic equal-area
															//	6 Lambert conformal
	string					_gmt_cmd_file;					// GMT command file to create the eps
	string					_gnuplot_cmd_file;				// Gnuplot command file to create the eps
	string					_pdf_cmd_file;					// PDF command file to create PDF
	string					_eps_file_name;					// The GMT-created eps file
	string					_pdf_file_name;					// The PDF file generated either by ps2pdf or LaTeX
	bool					_plot_phased_blocks;			// Plot the blocks of a segmented network.  Requires a corresponding segmentation file.
	bool					_plot_station_labels;			// Plots the station labels
	bool					_plot_ignored_msrs;				// Plot ignored measurements
	bool					_plot_alt_name;					// Plot alternate station names
	bool					_plot_station_constraints;		// plots the station constraints
	bool					_plot_correction_arrows;		// Plot arrows representing the direction and magnitude of corrections to the station coordinates.
	bool					_plot_correction_labels;		// Plot correction labels
	bool					_compute_corrections;			// Compute corrections from binary station file
	bool					_plot_positional_uncertainty;	// Plot positional uncertainty
	bool					_plot_error_ellipses;			// Plot error ellipses
	bool					_user_defined_projection;		// If true, the user has specified a projection for the output
	bool					_omit_title_block;				// Don't print a title block and measurements legend
	bool					_omit_measurements;				// Don't print measurements
	bool					_use_pdflatex;					// Uses PdfLaTeX to create the pdf (Good for font substitution, although not implemented yet)
	bool					_keep_gen_files;				// Don't delete command and data files used to generate EPS and PDF plots
	double					_label_font_size;				// Plot font size for labels
	double					_msr_line_width;				// Measurement line width
	double					_correction_scale;				// The amount by which to scale the size of correction arrows
	double					_pu_ellipse_scale;				// The amount by which to scale the size of error ellipses positional uncertainty cirlces
	string					_plot_station_centre;			// Centre the map according to this station
	string					_bounding_box;					// user defined bounding box for plot
	double					_plot_area_radius;				// Set the limits of the plot according to a radius
	double					_plot_centre_latitude;			// Centre the map according to this latitude
	double					_plot_centre_longitude;			// Centre the map according to this longitude
	double					_plot_scale;					// Final plot scale
	double					_page_width;					// page width in centimetres
	double					_ground_width;					// ground width in metres
	UINT32					_plot_block_number;				// Plots this block only
	UINT16					_coasline_resolution;			// ncdf resolution
	string					_pdf_viewer;					// PDF viewer
	string					_acrobat_ddename;				// If viewer is Acrobat, supply necessary dde arguments
	string					command_line_arguments;
	v_string_string_pair	_gmt_params;					// GMT parameters
	vchar					_separate_msrs;					// A char vector of measurement types to be created individually	
	v_string_string_pair	_msr_colours;					// A vector of measurement types and corresponding colours
};

// project settings
struct project_settings : private boost::equality_comparable<project_settings> {

	bool operator==(const project_settings& proj) const {
		return g == proj.g && 
			r == proj.r &&
			n == proj.n &&
			i == proj.i && 
			s == proj.s &&
			a == proj.a &&
			o == proj.o &&
			p == proj.p;
	}

	general_settings	g;
	reftran_settings	r;
	geoid_settings		n;
	import_settings		i;
	segment_settings	s;
	adjust_settings		a;
	output_settings		o;
	plot_settings		p;
	
};


#endif  // DNAOPTIONS_HPP
