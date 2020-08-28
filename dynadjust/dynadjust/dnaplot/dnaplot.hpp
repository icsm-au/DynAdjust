//============================================================================
// Name         : dnaplot.hpp
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
// Description  : DynAdjust Plot library
//============================================================================

#ifndef DNAPLOT_H_
#define DNAPLOT_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cstdarg>
#include <math.h>

#include <boost/shared_ptr.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <include/config/dnaexports.hpp>
#include <include/config/dnaversion.hpp>
#include <include/config/dnatypes-gui.hpp>

#include <include/exception/dnaexception.hpp>

#include <include/functions/dnatemplatefuncs.hpp>
#include <include/functions/dnatemplatestnmsrfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnaintegermanipfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnatemplategeodesyfuncs.hpp>
#include <include/functions/dnatemplatematrixfuncs.hpp>

#include <include/config/dnaconsts.hpp>
#include <include/config/dnatypes.hpp>

#include <include/parameters/dnaepsg.hpp>
#include <include/parameters/dnadatum.hpp>
#include <include/parameters/dnaprojection.hpp>

#include <include/measurement_types/dnadirection.hpp>
#include <include/measurement_types/dnagpsbaseline.hpp>

#include <include/math/dnamatrix_contiguous.hpp>

#include <include/io/dnaiobst.hpp>
#include <include/io/dnaiobms.hpp>
#include <include/io/dnaiomap.hpp>
#include <include/io/dnaioseg.hpp>
#include <include/io/dnaiopdf.hpp>

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::filesystem;

using namespace dynadjust::measurements;
using namespace dynadjust::exception;
using namespace dynadjust::datum_parameters;
using namespace dynadjust::iostreams;
using namespace dynadjust::math;

namespace dynadjust {
namespace networkplot {

#ifdef _MSC_VER
class DNAPLOT_API dna_plot {
#else
class dna_plot {
#endif
public:
	dna_plot();
	virtual ~dna_plot();

	_PLOT_STATUS_ CreateGMTPlot(plot_settings* plotCriteria, const string& network_name, const string& output_folder);
	_PLOT_STATUS_ CreateSegmentationGraph(plot_settings* plotCriteria, const string& network_name, 
		const string& output_folder, const plotGraphMode& graphMode);

	void LoadNetworkFiles(const project_settings& projectSettings);
	
	static void coutVersion();

private:

	CDnaDatum			datum_;
	CDnaProjection		projection_;
	
	// Gnuplot plotting methods
	void PlotGnuplotDatFileStns();
	void PlotGnuplotDatFileMsrs();
	void PrintGnuplotBatFile(const string& gnuplot_bat_file, const string& epsname, const plotGraphMode& graphMode);
	void PrintGnuplotBatFileStns(const UINT32& fontSize);
	void PrintGnuplotBatFileMsrs(const UINT32& fontSize);

	void LoadBinaryFiles();
	void LoadStationMap();
	void LoadSegmentationFile();
	void LoadPosUncertaintyFile();

	void ComputeStationCorrections();
	void ComputeStationCorrection(it_vstn_t_const _it_stn, stationCorrections_t& stnCor,
		matrix_2d& currentEstimates, matrix_2d& initialEstimates);
	void LoadCorrectionsFile();
	
	void SortandMapStations();
	void ComputeMeasurementCount();

	// GMT plotting methods
	void FormGMTDataFileNames(const UINT32& block=0);
	void SetGMTParameters();
	
	void PrintStationDataFile(ostream& os, it_vstn_t_const _it_stn);
	void PrintStationsDataFile();
	void PrintStationsDataFileBlock(const UINT32& block);
	
	void PrintStationLabel(ostream& os, it_vstn_t_const _it_stn);
	void PrintStationLabels();
	void PrintStationLabelsBlock(const UINT32& block);
	
	void PrintCorrectionArrows(const UINT32& block);
	void PrintPositionalUncertainty(const UINT32& block);
	void PrintErrorEllipses(const UINT32& block);
	
	void PrintMeasurementsDatFiles();
	void PrintMeasurementsDatFilesBlock(const UINT32& block);

	void PrintMeasurementsDatFile(const string& msr_file_name, char msrType, std::ofstream* msr_file_stream);
	void PrintMeasurementsDatFileBlock(const UINT32& block, const string& msr_file_name, char msrType, std::ofstream* msr_file_stream);
	
	void PrintGMTBatfile(const string& epsname, plot_settings* plotCriteria); 
	void PrintLaTeXPlotfile(const string& gmt_tex_file, const string& epsname);
	string PrintPdfCmdfile(const string& pdf_cmd_file, const string& gmt_tex_filename, const string& pic_name);
	void PrintPdfCmdfile_Graph(const string& pdf_cmd_file, const string& pic_name);

	//void PrintGMTPlotCoords(vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, ostream& osStn, ostream& osStn2, ostream& osMsr); 
	//void PrintGMTPlotCoords_D(vdnaStnPtr* vStations, vector<CDnaDirection>* vDirections, ostream& osMsr); 
	//void PrintGMTPlotCoords_GX(vdnaStnPtr* vStations, vector<CDnaGpsBaseline>* vBaselines, ostream& osMsr); 

	void CalculateLimitsFromStation();
	void CalculateLimitsFromPoint();
	void DetermineBoundingBox();

	void SignalExceptionPlot(const string& msg, const int& line_no, const char *streamType, ...);

	void ReduceStationCoordinates(station_t* stationRecord);

	void NormaliseScale(double& scale);
	void NormaliseScaleBar(double& scale_bar_width);
	void NormaliseGraticule(double& graticule_width, UINT32& graticule_width_precision);
	void SelectCoastlineResolution(const double& dDimension, string& coastResolution, plot_settings* plotCriteria);

	bool WithinLimits(const double& latitude, const double& longitude);

	void BuildParameterStationList();

	vvUINT32				v_parameterStationList_;

	std::ofstream			gmtbat_file_;
	std::ofstream			gnuplotbat_file_;

	MsrTally				parsemsrTally_;		// total network tally
	vector<MsrTally>		v_msr_tally_;		// per block tally

	vstn_t					bstBinaryRecords_;
	vmsr_t					bmsBinaryRecords_;
	string					output_folder_;
	string					network_name_;
	v_string_uint32_pair	stnsMap_;		// Station Name Map sorted on name (string)
	_PLOT_STATUS_			plotStatus_;
	plot_settings			plotCriteria_;
	project_settings		projectSettings_;
	
	double					lowerDeg_;
	double					leftDeg_;
	double					upperDeg_;
	double					rightDeg_;
	double					centre_width_;
	double					centre_height_;

	double					mapScale_;
	double					arrow_legend_long_;
	double					arrow_legend_lat_;
	double					average_correction_;
	double					arrow_legend_length_;

	double					uncertainty_legend_long_;
	double					uncertainty_legend_lat_;
	double					largest_uncertainty_;
	double					uncertainty_legend_length_;
	
	_it_pair_vchar			it_vchar_range;
	it_pair_string_vUINT32	it_stnmap_range;
	vchar					_combined_msr_list;

	string					seg_stn_graph_file_;
	string					seg_msr_graph_file_;

	vstring					v_isl_pts_file_;
	vstring					v_isl_const_file_;
	vstring					v_isl_lbl_file_;
	vstring					v_stn_cor_file_;
	vstring					v_stn_apu_file_;
	vstring					v_stn_err_file_;
	vstring					v_jsl_pts_file_;
	vstring					v_jsl_const_file_;
	vstring					v_jsl_lbl_file_;
	vstring					v_msr_def_file_;
	vstring					v_msr_all_file_;
	
	vv_string_string_pair	v_msr_file_;

	vstnCor_t				v_stn_corrs_;
	vstnPU_t				v_stn_pu_;

	bool					isLandscape;
	bool					default_limits_;
	bool					plotConstraints_;

	// segmentation block variables
	bool					plotBlocks_;
	UINT32					stationCount_;
	UINT32					measurementCount_;
	UINT32					blockCount_;
	UINT32					blockThreshold_;
	UINT32					minInnerStns_;
	vUINT32					YClusterStations_;
	vvUINT32				v_ISL_;				// Inner stations
	vvUINT32				v_JSL_;				// Junction stations
	vvUINT32				v_CML_;				// Measurements

	vUINT32				v_measurementCount_;		// number of raw measurements and constrained stations
	vUINT32				v_unknownsCount_;			// number of all stations (constrained and free)
};


}	// namespace mathcomp
}	// namespace dynadjust



#endif /* DNAPLOT_H_ */
