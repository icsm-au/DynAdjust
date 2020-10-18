//============================================================================
// Name         : dnaplot.cpp
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

#include <dynadjust/dnaplot/dnaplot.hpp>

namespace dynadjust { 
namespace networkplot {

dna_plot::dna_plot()
	: datum_(DEFAULT_EPSG_U)
	, output_folder_("")
	, network_name_("")
	, stationCount_(0)
	, blockCount_(1)
{
	v_msr_file_.clear();
	v_isl_const_file_.clear();
	v_isl_pts_file_.clear();
	v_jsl_const_file_.clear();
	v_isl_lbl_file_.clear();
	v_stn_cor_file_.clear();

	v_parameterStationList_.clear();

#ifdef _MSC_VER
#if (_MSC_VER < 1900)
	{
		// this function is obsolete in MS VC++ 14.0, VS2015
		// Set scientific format to print two places for the exponent
		_set_output_format(_TWO_DIGIT_EXPONENT);
	}
#endif
#endif
}
	

dna_plot::~dna_plot()
{
	
}

void dna_plot::coutVersion()
{
	string msg;
	fileproc_help_header(&msg);
	cout << msg << endl;
}


_PLOT_STATUS_ dna_plot::CreateSegmentationGraph(plot_settings* plotCriteria, const string& network_name, 
				const string& output_folder, const plotGraphMode& graphMode)
{
	if (!exists(output_folder))
	{
		stringstream ss("CreateSegmentationGraph(): Output path does not exist... \n\n    ");
		ss << output_folder << ".";
		SignalExceptionPlot(ss.str(), 0, NULL);
	}

	output_folder_ = output_folder;
	network_name_ = network_name;

	// create bat file and set gnuplot parameters 
	string gnuplot_cmd_filename("graph_" + network_name + "_eps.bat");
	string gnuplot_cmd_file(output_folder_ + FOLDER_SLASH + gnuplot_cmd_filename);

	string gnuplot_pic_name;
	switch (graphMode)
	{
	case StationsMode:
		gnuplot_pic_name = (network_name + "_graph_stn");
		break;
	case MeasurementsMode:
		gnuplot_pic_name = (network_name + "_graph_msr");
		break;
	}
	
	string gnuplot_eps_file(gnuplot_pic_name + ".eps");
	plotCriteria->_eps_file_name = gnuplot_eps_file;
	
	plotCriteria_ = *plotCriteria;

	switch (graphMode)
	{
	case StationsMode:
		PlotGnuplotDatFileStns();
		break;
	case MeasurementsMode:
		PlotGnuplotDatFileMsrs();
		break;
	}

	try {
		// Create gnuplot batch file.  Throws runtime_error on failure.
		file_opener(gnuplotbat_file_, gnuplot_cmd_file);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	PrintGnuplotBatFile(gnuplot_cmd_file, gnuplot_eps_file, graphMode);
	gnuplotbat_file_.close();

	string pdf_cmd_filename("graph_" + network_name + "_pdf.bat");
	string pdf_cmd_file(output_folder_ + FOLDER_SLASH + pdf_cmd_filename);
	
	PrintPdfCmdfile_Graph(pdf_cmd_file, gnuplot_pic_name);

	plotCriteria->_gnuplot_cmd_file = gnuplot_cmd_filename;
	plotCriteria->_pdf_cmd_file = pdf_cmd_filename;
	

	return PLOT_SUCCESS;
}
	

void dna_plot::PlotGnuplotDatFileStns()
{
	seg_stn_graph_file_ = network_name_ + "-stn.seg.data";
	
	std::ofstream seg_data;
	try {
		// Create gnuplot station segment data file.  Throws runtime_error on failure.
		file_opener(seg_data, formPath<string>(output_folder_, seg_stn_graph_file_));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	stringstream ss(""), st("");
	ss << "\"Max station limit (" << setprecision(0) << blockThreshold_ << ")\" ";
	st << "\"Min inner stns (" << minInnerStns_ << ")\" ";
	seg_data << setw(HEADER_18) << left << "Block" <<
		setw(HEADER_32) << left << ss.str() <<
		setw(HEADER_32) << left << st.str() <<
		setw(HEADER_25) << left << "\"Total block size\"" <<
		setw(HEADER_18) << left << "\"Inner stns\"" <<
		setw(HEADER_18) << left << "\"Junction stns\"" << endl;

	for (UINT32 block=0; block<blockCount_; ++block)
		seg_data << setw(HEADER_18) << left << block+1 << 
			setw(HEADER_32) << left << setprecision(0) << blockThreshold_ <<				// threshold
			setw(HEADER_32) << left << setprecision(0) << minInnerStns_ <<				// threshold
			setw(HEADER_25) << left << v_ISL_.at(block).size() + v_JSL_.at(block).size() <<	// block size
			setw(HEADER_18) << left << v_ISL_.at(block).size() <<		// inner station count
			setw(HEADER_18) << left << v_JSL_.at(block).size() << endl;	// junction station count

	seg_data.close();
}
	

void dna_plot::PlotGnuplotDatFileMsrs()
{
	seg_msr_graph_file_ = network_name_ + "-msr.seg.data";
	
	std::ofstream seg_data;
	try {
		// Create gnuplot measurement segment data file.  Throws runtime_error on failure.
		file_opener(seg_data, formPath<string>(output_folder_, seg_msr_graph_file_));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	UINT16 c;

	// print header
	seg_data << setw(PRINT_VAR_PAD) << left << "Block";
	stringstream ss;
	ss << "\"Total measurements\"";
	seg_data << setw(PRINT_VAR_PAD) << left << ss.str();

	for (c=0; c<_combined_msr_list.size(); c++)
	{		
		if (parsemsrTally_.MeasurementCount(_combined_msr_list.at(c)) == 0)
			continue;
		ss.str("");
		ss << "\"" << measurement_name<char, string>(_combined_msr_list.at(c)) << " (" <<
			parsemsrTally_.MeasurementCount(_combined_msr_list.at(c)) << ")\"";
		seg_data << setw(PRINT_VAR_PAD) << left << ss.str();
	}
	seg_data << endl;

	// Tally up measurement types for each block
	ComputeMeasurementCount();

	// print measurement tally for each block
	UINT32 block;
	for (block=0; block<blockCount_; ++block)
	{
		seg_data << setw(PRINT_VAR_PAD) << left << block + 1;
		seg_data << setw(PRINT_VAR_PAD) << left << v_msr_tally_.at(block).TotalCount();

		for (c=0; c<_combined_msr_list.size(); c++)
		{
			// do any measurements of this type exist at all?
			if (parsemsrTally_.MeasurementCount(_combined_msr_list.at(c)) > 0)
			{
				// yes, so test if this block has such measurements...
				//if (v_msr_tally_.at(block).MeasurementCount(_combined_msr_list.at(c)) == 0)			// no 
				//	seg_data << setw(PRINT_VAR_PAD) << left << "-";
				//else
					seg_data << setw(PRINT_VAR_PAD) << left << v_msr_tally_.at(block).MeasurementCount(_combined_msr_list.at(c));
			}
		}
		seg_data << endl;
	}
	
	seg_data.close();
}
	

void dna_plot::ComputeMeasurementCount()
{
	v_msr_tally_.resize(blockCount_);

	UINT32 block;
	it_vUINT32 _it_msr;

	for (block=0; block<blockCount_; ++block)
	{
		for (_it_msr=v_CML_.at(block).begin(); _it_msr<v_CML_.at(block).end(); _it_msr++)
		{
			// Increment single station measurement counters...
			switch (bmsBinaryRecords_.at(*_it_msr).measType)
			{
			case 'A': // Horizontal angle
				v_msr_tally_.at(block).A++;
				break;
			case 'B': // Geodetic azimuth
				v_msr_tally_.at(block).B++;
				break;
			case 'C': // Chord dist
				v_msr_tally_.at(block).C++;
				break;
			case 'D': // Direction set
				if (bmsBinaryRecords_.at(*_it_msr).measStart == xMeas)
					v_msr_tally_.at(block).D += bmsBinaryRecords_.at(*_it_msr).vectorCount1;
				break;
			case 'E': // Ellipsoid arc
				v_msr_tally_.at(block).E++;
				break;
			case 'G': // GPS Baseline (treat as single-baseline cluster)
				v_msr_tally_.at(block).G ++;
				break;
			case 'X': // GPS Baseline cluster
				if (bmsBinaryRecords_.at(*_it_msr).measStart == xMeas)
				{
					if (bmsBinaryRecords_.at(*_it_msr).vectorCount1 == bmsBinaryRecords_.at(*_it_msr).vectorCount2 + 1)
						v_msr_tally_.at(block).X += (bmsBinaryRecords_.at(*_it_msr).vectorCount1 * 3);
				}					
				break;
			case 'H': // Orthometric height
				v_msr_tally_.at(block).H++;
				break;
			case 'I': // Astronomic latitude
				v_msr_tally_.at(block).I++;
				break;
			case 'J': // Astronomic longitude
				v_msr_tally_.at(block).J++;
				break;
			case 'K': // Astronomic azimuth
				v_msr_tally_.at(block).K++;
				break;
			case 'L': // Level difference
				v_msr_tally_.at(block).L++;
				break;
			case 'M': // MSL arc
				v_msr_tally_.at(block).M++;
				break;
			case 'P': // Geodetic latitude
				v_msr_tally_.at(block).P++;
				break;
			case 'Q': // Geodetic longitude
				v_msr_tally_.at(block).Q++;
				break;
			case 'R': // Ellipsoidal height
				v_msr_tally_.at(block).R++;
				break;
			case 'S': // Slope distance
				v_msr_tally_.at(block).S++;
				break;
			case 'V': // Zenith angle
				v_msr_tally_.at(block).V++;
				break;
			case 'Y': // GPS point cluster
				if (bmsBinaryRecords_.at(*_it_msr).measStart == xMeas)
				{
					if (bmsBinaryRecords_.at(*_it_msr).vectorCount1 == bmsBinaryRecords_.at(*_it_msr).vectorCount2 + 1)
						v_msr_tally_.at(block).Y += (bmsBinaryRecords_.at(*_it_msr).vectorCount1 * 3);
				}					
				break;
			case 'Z': // Vertical angle
				v_msr_tally_.at(block).Z++;
				break;
			default:
				stringstream ss;
				ss << "ComputeMeasurementCount(): Unknown measurement type:  " << bmsBinaryRecords_.at(*_it_msr).measType << endl;
				throw NetPlotException(ss.str(), 0);
			}
		}
	}
}


void dna_plot::PrintGnuplotBatFile(const string& gnuplot_bat_file, const string& epsname, const plotGraphMode& graphMode)
{
	if (output_folder_ != ".")
		gnuplotbat_file_ << "cd '" << output_folder_ << "'" << endl << endl;

	gnuplotbat_file_ << "set terminal postscript eps enhanced color solid colortext" << endl << endl;
	gnuplotbat_file_ << "set output \"" << plotCriteria_._eps_file_name.c_str() << "\"" << endl;

	// histogram style
	gnuplotbat_file_ << "set style fill transparent solid 0.4" << endl;
	gnuplotbat_file_ << "set boxwidth 0.5" << endl;

	UINT32 upperLimit(0), block(0);
	
	switch (graphMode)
	{
	case StationsMode:
		for (block=0; block<blockCount_; ++block)
			if (upperLimit < (v_ISL_.at(block).size() + v_JSL_.at(block).size()))
				upperLimit = static_cast<UINT32>(v_ISL_.at(block).size() + v_JSL_.at(block).size());
		
		upperLimit = max(blockThreshold_, upperLimit);

		gnuplotbat_file_ << "set title \"" << "Station segmentation summary for " << network_name_ << "\" font \"Helvetica,20\" noenhanced" << endl << endl;
		gnuplotbat_file_ << "set key outside center bottom horizontal Left reverse enhanced autotitles samplen 2.5 font \"Helvetica,8\"" << endl;
		gnuplotbat_file_ << "set key width -12 height 0.5" << endl << endl;

		break;
	case MeasurementsMode:
		for (block=0; block<blockCount_; ++block)
			if (upperLimit < v_msr_tally_.at(block).TotalCount())
				upperLimit = v_msr_tally_.at(block).TotalCount();
	
		gnuplotbat_file_ << "set title \"" << "Measurement segmentation summary for " << network_name_ << "\" font \"Helvetica,20\" noenhanced" << endl << endl;
		//gnuplotbat_file_ << "set key outside right top vertical Left reverse enhanced autotitles columnhead box samplen 2.5 font \"Helvetica,8\"" << endl;
		gnuplotbat_file_ << "set key outside center bottom horizontal Left reverse enhanced autotitles columnhead samplen 2.5 font \"Helvetica,8\"" << endl;
		//gnuplotbat_file_ << "set key width -15 height 0" << endl << endl;
		gnuplotbat_file_ << "set key width -15 height 0.5" << endl << endl;

		gnuplotbat_file_ << "set style histogram rowstacked title offset character 0, 0, 0" << endl;
		gnuplotbat_file_ << "set style data histograms" << endl;
		gnuplotbat_file_ << "set datafile missing '-'" << endl;
		gnuplotbat_file_ << "" << endl;
		break;
	}

	upperLimit = upperLimit + upperLimit / 10;

	gnuplotbat_file_ << "set format x '%.0f'" << endl;
	gnuplotbat_file_ << "set format y '%.0f'" << endl;
	gnuplotbat_file_ << "set yrange[0:" << upperLimit << "]" << endl;
	gnuplotbat_file_ << "set auto x" << endl;
	gnuplotbat_file_ << "set ytics scale 0.25 font \"Helvetica,10\"" << endl;

	gnuplotbat_file_ << "set xtics scale 0.25 nomirror" << endl;

	UINT32 fontSize(8);
	if (blockCount_ > 5000)
	{
		fontSize = 5;
		gnuplotbat_file_ << "set xtics 0,500 font \"Helvetica,6\"" << endl << endl;
	}
	else if (blockCount_ > 1000)
	{
		fontSize = 5;
		gnuplotbat_file_ << "set xtics 0,100 font \"Helvetica,6\"" << endl << endl;
	}
	else if (blockCount_ > 500)
	{
		fontSize = 5;
		gnuplotbat_file_ << "set xtics 0,50 font \"Helvetica,5\"" << endl << endl;
	}
	else if (blockCount_ > 100)
	{
		fontSize = 5;
		gnuplotbat_file_ << "set xtics 0,10 font \"Helvetica,6\"" << endl << endl;
	}
	else if (blockCount_ > 50)
	{
		fontSize = 6;
		gnuplotbat_file_ << "set xtics 0,5 font \"Helvetica,8\"" << endl << endl;
	}
	else
		gnuplotbat_file_ << "set xtics 0,1 font \"Helvetica,10\"" << endl << endl;
	

	// x-axis label
	stringstream ss("");
	ss << "Segmented Network Blocks (Total " << fixed << setprecision(0) << blockCount_ << ")";
	gnuplotbat_file_ << "set xlabel '" << ss.str() << "' font \"Helvetica,11\"" << endl;

	switch (graphMode)
	{
	case StationsMode:
		PrintGnuplotBatFileStns(fontSize);
		break;
	case MeasurementsMode:
		PrintGnuplotBatFileMsrs(fontSize);
		break;
	}	
}
	
void dna_plot::PrintGnuplotBatFileStns(const UINT32& fontSize)
{
	stringstream ss("");
	ss << "Station Count (Total " << fixed << setprecision(0) << stationCount_ << ")";
	gnuplotbat_file_ << "set ylabel '" << ss.str() << "' font \"Helvetica,11\"" << endl << endl;

	// http://w3schools.com/tags/ref_colorpicker.asp
	// for converting rgb colours in hex to decimal, see:
	// http://www.yellowpipe.com/yis/tools/hex-to-rgb/color-converter.php
	gnuplotbat_file_ << "set style line 1 lw 0.75 lt 1 pt 7 ps 0.5 lc rgb \"#4169e1\"          # total block size" << endl;	// royalblue
	gnuplotbat_file_ << "set style line 2 lw 2 lt 5 pt 7 ps 0.5 lc rgb \"green\"            # threshold" << endl;			// green
	gnuplotbat_file_ << "set style line 3 lw 2 lt 5 pt 7 ps 0.5 lc rgb \"orange\"            # minimum inner size" << endl;			// orange
	gnuplotbat_file_ << "set style line 4 lw 2 lt 1 pt 7 ps 0.5 lc rgb \"blue\"             # inners" << endl;			// blue
	gnuplotbat_file_ << "set style line 5 lw 2 lt 1 pt 7 ps 0.5 lc rgb \"red\"              # junctions" << endl << endl;		// red

	gnuplotbat_file_ << "plot '" << seg_stn_graph_file_ << "' using 1:4 with boxes ls 1 title columnheader(4), \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:4:(sprintf(\"%.0f\",$4)) with labels font \"Helvetica," << 
		fontSize << "\" center offset 0,0.5 notitle, \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:2 with lines ls 2 title columnheader(2), \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:3 with lines ls 3 title columnheader(3), \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:5 with linespoints ls 4 title columnheader(5), \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:5:(sprintf(\"%.0f\",$5)) with labels tc ls 4 font \"Helvetica," << 
		fontSize << "\" center offset 1,0 notitle, \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:6 with linespoints ls 5 title columnheader(6), \\" << endl;
	gnuplotbat_file_ << "     '" << seg_stn_graph_file_ << "' using 1:6:(sprintf(\"%.0f\",$6)) with labels tc ls 5 font \"Helvetica," << 
		fontSize << "\" center offset 1,0 notitle" << endl << endl;
}
	

void dna_plot::PrintGnuplotBatFileMsrs(const UINT32& fontSize)
{
	stringstream ss("");
	ss << "Measurement Count (Total " << fixed << setprecision(0) << measurementCount_ << ")";
	gnuplotbat_file_ << "set ylabel '" << ss.str() << "' font \"Helvetica,11\"" << endl << endl;

	// http://w3schools.com/tags/ref_colorpicker.asp
	// for converting rgb colours in hex to decimal, see:
	// http://www.yellowpipe.com/yis/tools/hex-to-rgb/color-converter.php
	// for seeing colours, see:
	// http://www.drpeterjones.com/colorcalc/

	UINT32 line(1);
	ss.str("");
	ss << "\"#4169e1\"";
	gnuplotbat_file_ << "set style line " << line++ << " lw 1 lt 1 pt 7 ps 0.5 lc rgb " << left << setw(PRINT_VAR_PAD) << ss.str() << " # total block size" << endl;	// royalblue

	// print measurements for each block
	UINT32 c;
	string colour;
	it_pair_string _it_colour;
	
	sort(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), ComparePairFirst<string>());

	for (c=0; c<_combined_msr_list.size(); c++)
	{
		if (parsemsrTally_.MeasurementCount(_combined_msr_list.at(c)) < 1)
			continue;
		colour = _combined_msr_list.at(c);
		_it_colour = equal_range(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), 
			colour, ComparePairFirst<string>());

		if (_it_colour.first == _it_colour.second)
			colour = "light-gray";
		else
			colour = _it_colour.first->second;
		
		ss.str("");
		ss << "\"" << colour << "\"";
		gnuplotbat_file_ << "set style line " << line++ << " lw 1 lt 1 pt 7 ps 0.5 lc rgb " << left << setw(PRINT_VAR_PAD) << ss.str() << 
			" # \"" << measurement_name<char, string>(_combined_msr_list.at(c)) << "\"" << endl;
		
	}
	gnuplotbat_file_ << endl;
	
	//UINT32 block(0);
	//if (v_msr_tally_.at(block).MeasurementCount(_combined_msr_list.at(c)) == 0)
	//	continue;

	// plot total measurement count
	line = 3;
	UINT32 linestyle(line-1), msrs(0);
	gnuplotbat_file_ << "plot '";
	
	for (c=0; c<_combined_msr_list.size(); c++)
	{
		if (parsemsrTally_.MeasurementCount(_combined_msr_list.at(c)) < 1)
			continue;
		if (msrs++ > 0)
			gnuplotbat_file_ << ", \\" << endl << "     '";
		gnuplotbat_file_ << seg_msr_graph_file_ << "' using " << line++ << ":xtic(1) ls " << linestyle++;
	}

	gnuplotbat_file_ << ", \\" << endl << "     '" << seg_msr_graph_file_ << 
		"' using 0:2:(sprintf(\"%d\",$2)) with labels font \"Helvetica," << 
			fontSize << "\" center offset 0,0.5 notitle" << endl;
	
}
	

_PLOT_STATUS_ dna_plot::CreateGMTPlot(plot_settings* plotCriteria, const string& network_name, const string& output_folder)
{
	if (!exists(output_folder))
	{
		stringstream ss("CreateGMTPlot(): Output path does not exist... \n\n    ");
		ss << output_folder << ".";
		SignalExceptionPlot(ss.str(), 0, NULL);
	}

	output_folder_ = output_folder;
	network_name_ = network_name;
	
	// create bat file and set GMT parameters 
	string gmt_cmd_filename("create_" + network_name + "_eps.bat");
	string gmt_cmd_file(output_folder_ + FOLDER_SLASH + gmt_cmd_filename);
	string gmt_eps_name(network_name + "_plot");
	string gmt_eps_file(gmt_eps_name + ".eps");
	plotCriteria->_eps_file_name = gmt_eps_file;
	
	plotCriteria_ = *plotCriteria;
	
	v_isl_const_file_.clear();
	v_jsl_const_file_.clear();
	v_isl_pts_file_.clear();
	v_isl_lbl_file_.clear();
	v_stn_cor_file_.clear();
	v_stn_err_file_.clear();
	v_stn_apu_file_.clear();
	v_jsl_pts_file_.clear();
	v_jsl_lbl_file_.clear();

	plotConstraints_ = false;

	lowerDeg_ = 1000.0;
	leftDeg_ = 1000.0;
	upperDeg_ = -1000.0;
	rightDeg_ = -1000.0;

	default_limits_ = true;
	
	// check for valid plot limits
	if (!plotCriteria_._bounding_box.empty())
		default_limits_ = false;
	else if (plotCriteria_._plot_area_radius > 0. && 
		plotCriteria_._plot_centre_latitude > -90.00000001 && plotCriteria_._plot_centre_latitude < 90.00000001 && 
		plotCriteria_._plot_centre_longitude > -180.00000001 && plotCriteria_._plot_centre_longitude < 180.00000001)
		default_limits_ = false;
	else if (plotCriteria_._plot_area_radius > 0. && !plotCriteria_._plot_station_centre.empty())
		default_limits_ = false;

	UINT32 block(plotCriteria_._plot_block_number - 1);

	if (plotBlocks_)
	{
		bool oneBlockOnly(plotCriteria_._plot_block_number > 0);
		
		for (block=0; block<blockCount_; ++block)
		{
			PrintStationsDataFileBlock(block);
			if (!plotCriteria_._omit_measurements)
				PrintMeasurementsDatFilesBlock(block);
			if (oneBlockOnly)
				break;
		}		
	}
	else
	{
		v_msr_file_.resize(1);
		// PrintStationsDataFile calculates plot limits if user has 
		// specified centre station or lat/long, or bounding box
		PrintStationsDataFile();
		if (!plotCriteria_._omit_measurements)
			PrintMeasurementsDatFiles();
	}

	try {
		// Create GMT batch file.  Throws runtime_error on failure.
		file_opener(gmtbat_file_, gmt_cmd_file);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

#if defined(_WIN32) || defined(__WIN32__)
	gmtbat_file_ << "@echo off" << endl;
#elif defined(__linux) || defined(sun) || defined(__unix__)
	gmtbat_file_ << "#!/bin/bash" << endl;
#endif
	
	SetGMTParameters();

	// GMT bat file is printed last to reflect the options and dimensions as determined
	// by PrintStationsDataFile and PrintMeasurementsDatFiles
	PrintGMTBatfile(gmt_eps_file, plotCriteria);
	gmtbat_file_.close();

	// Now print stations labels (based on font size determined by PrintGMTPlotBatfile)
	if (plotCriteria_._plot_station_labels)
	{
		if (plotBlocks_)
		{
			if (plotCriteria_._plot_block_number > 0)
				PrintStationLabelsBlock(plotCriteria_._plot_block_number-1);
			else
				for (block=0; block<blockCount_; ++block)
					PrintStationLabelsBlock(block);
		}
		else
			PrintStationLabels();
	}

	if (plotCriteria_._plot_positional_uncertainty)
	{
		if (plotCriteria_._plot_block_number > 0)
			PrintPositionalUncertainty(plotCriteria_._plot_block_number-1);
		else
			for (block=0; block<blockCount_; ++block)
				PrintPositionalUncertainty(block);
	}

	if (plotCriteria_._plot_error_ellipses)
	{
		if (plotCriteria_._plot_block_number > 0)
			PrintErrorEllipses(plotCriteria_._plot_block_number-1);
		else
			for (block=0; block<blockCount_; ++block)
				PrintErrorEllipses(block);
	}

	if (plotCriteria_._plot_correction_arrows)
	{
		if (plotCriteria_._plot_block_number > 0)
			PrintCorrectionArrows(plotCriteria_._plot_block_number-1);
		else
			for (block=0; block<blockCount_; ++block)
				PrintCorrectionArrows(block);
	}

	plotCriteria->_plot_scale = mapScale_;

	string gmt_tex_filename;
	if (plotCriteria_._use_pdflatex)
	{
		gmt_tex_filename = network_name + ".tex";
		string gmt_tex_file(output_folder_ + FOLDER_SLASH + gmt_tex_filename);
		PrintLaTeXPlotfile(gmt_tex_file, gmt_eps_name);
	}

	string pdf_cmd_filename("create_" + network_name + "_pdf.bat");
	string pdf_cmd_file(output_folder_ + FOLDER_SLASH + pdf_cmd_filename);
	string pdf_plot_name;
	if (plotBlocks_)
		pdf_plot_name = network_name + "-phased";
	else
		pdf_plot_name = network_name + "-simult";

	plotCriteria->_gmt_cmd_file = gmt_cmd_filename;
	plotCriteria->_pdf_cmd_file = pdf_cmd_filename;
	
	plotCriteria->_pdf_file_name = PrintPdfCmdfile(pdf_cmd_file, gmt_tex_filename, pdf_plot_name);

	return PLOT_SUCCESS;
}


void dna_plot::PrintGMTBatfile(const string& epsname, plot_settings* plotCriteria)
{
	//
	// At this point, all limit values are in decimal degrees
	//

	if (rightDeg_ < leftDeg_)
	{
		stringstream ss;
		ss << "Right limit cannot be less than left limit." << endl;
		throw NetPlotException(ss.str(), 0);
	}

	if (upperDeg_ < lowerDeg_)
	{
		stringstream ss;
		ss << "Upper limit cannot be less than lower limit." << endl;
		throw NetPlotException(ss.str(), 0);
	}

	// calculate dimensions of data extents
	double dWidth(rightDeg_ - leftDeg_);
	double dHeight(upperDeg_ - lowerDeg_);
	
	// if data is a single point (or a very small area)
	if (dHeight < 0.00005)
		dHeight += seconds15;
	if (dWidth < 0.00005)
		dWidth += seconds15;	
	
	// capture smallest dimension
	double dDimension(min(dWidth, dHeight));
	
	// Determine a buffer to envelope the entire plot, set to
	// 10% of the width/height (whichever is smaller)
	double dBuffer(dDimension * (0.1));

	// calculate latitude at which to place the scale bar
	double dScaleLat(lowerDeg_ - dBuffer / 2.0);

	// calculate centre point of data
	centre_width_ = (rightDeg_ + leftDeg_) / 2.0;
	centre_height_ = (upperDeg_ + lowerDeg_) / 2.0;

	// Has the user specified certain limits, or will the data be used to 
	// define the limits
	if (default_limits_)
	{
		// OK, the spatial extent of the data sets the limits, so
		// add a buffer accordingly
		upperDeg_ += dBuffer;
		lowerDeg_ -= dBuffer;
		leftDeg_ -= dBuffer;
		rightDeg_ += dBuffer;

		if (dDimension > seconds60)
		{
			// round down to nearest 5"
			leftDeg_ = leftDeg_ - fmod(leftDeg_, seconds05);
			lowerDeg_ = lowerDeg_ - fmod(lowerDeg_, seconds05);
			rightDeg_ = rightDeg_ + (seconds05 - fmod(rightDeg_, seconds05));
			upperDeg_ = upperDeg_ + (seconds05 - fmod(upperDeg_, seconds05));
		}
	}
	else
	{
		// Limits have been calculated via user input, so simply calculate
		// calculate latitude at which the scale bar
		// will be placed
		centre_width_ = plotCriteria_._plot_centre_longitude;
		centre_height_ = plotCriteria_._plot_centre_latitude;

		dScaleLat = lowerDeg_ + dBuffer / 2.0;
	}

	// prevent limit values from exceeding 'normal' limits
	if (default_limits_)
	{
		if (lowerDeg_ < -90.)
			lowerDeg_ = -90.;
		if (lowerDeg_ > 90.)
			lowerDeg_ = 90.;
	}

	// set the position of the error ellipse legend
	uncertainty_legend_long_ = leftDeg_ + dBuffer;
	uncertainty_legend_lat_ = dScaleLat;

	if (plotCriteria_._plot_correction_arrows)
	{
		// set the position of the correction arrow legend
		// if error ellipses AND corrections are plotted, then put
		// the corrections legend on the right hand side
		if (plotCriteria_._plot_error_ellipses || 
			plotCriteria_._plot_positional_uncertainty)
			arrow_legend_long_ = rightDeg_ - dBuffer * 2.0;
		else
			arrow_legend_long_ = leftDeg_ + dBuffer / 2.0;
		arrow_legend_lat_ = dScaleLat;
	}

	// Portrait or Landscape?
	// A3 paper width (landscape) is 42cm, and (portrait) is 29.7cm.  Less 
	// nomenclature (~2 cm), this leaves an available width for plotting 
	// of 40cm L or 27.7cm P
	//
	// title block height
	double title_block_height(6);
	double page_width(40.0);		// centimetres

	double avg_data_scale;
	if (dWidth > dHeight)
		avg_data_scale = dHeight / 27.7;
	else
		avg_data_scale = dHeight / 40.;
	
	isLandscape = true;
	if (dWidth < (dHeight + (title_block_height * avg_data_scale)) || !default_limits_)
	{
		page_width = 27.7;			// centimetres
		isLandscape = false;
	}	

	plotCriteria->_page_width = page_width + 2;

	// Scale (for A3 page width)
	double scale;

	// Define fonts
	float annot_font_size_primary(9), label_font_size(11.);
	
	if ((rightDeg_ - leftDeg_) > Degrees(PI) ||			// wider than 180 degrees?
		(upperDeg_ - lowerDeg_) > Degrees(PI_135))		// taller than 135 degrees?
	{
		// Set the appropriate map projection
		if(!plotCriteria_._user_defined_projection)
		{
			if ((upperDeg_ - lowerDeg_) > Degrees(PI_135))		// taller than 135 degrees?
			{
				// Print the entire world on a flat sheet
				plotCriteria_._projection = world;

				// Calculate ground width, which in this case is the circumference of the world
				dDimension = TWO_PI * datum_.GetEllipsoid().GetSemiMajor();		// set distance circumference of a circle
				
				// Centre world map according to data centre
				leftDeg_ = centre_width_ - 180.0;
				rightDeg_ = centre_width_ + 180.0;

				// Normalise limits according to 180 boundary
				if (leftDeg_ < -180.0)
					leftDeg_ += 180.0;
				if (rightDeg_ > 180.0)
					rightDeg_ -= 180.0;
				
				// Set latitude limits
				lowerDeg_ = -90;
				upperDeg_ = 90;
			}
			else
			{
				// Print a globe plot.  Limits are not required
				plotCriteria_._projection = orthographic;
				
				// Calculate ground width between west and east limits (including buffer)
				dDimension = Radians(rightDeg_ - leftDeg_) / TWO_PI * datum_.GetEllipsoid().GetSemiMajor();
			}
		}
		else
			// Calculate ground width between west and east limits (including buffer)
			dDimension = Radians(rightDeg_ - leftDeg_) / TWO_PI * datum_.GetEllipsoid().GetSemiMajor();

		// Calculate scale
		scale = dDimension / page_width * 100;		// metres
	}
	else
	{
		double azimuth;

		// Calculate accurate map width from limits
		dDimension = RobbinsReverse<double>(		// calculate distance (in metres) of map width
			Radians(centre_height_), Radians(leftDeg_), Radians(centre_height_), Radians(rightDeg_),
			&azimuth, datum_.GetEllipsoidRef());

		// Calculate scale
		scale = dDimension / page_width * 100;		// metres

		if (!plotCriteria_._user_defined_projection)
		{
			// default (large regions)
			plotCriteria_._projection = mercator;

			// very high latitudes
			if (fabs(centre_height_) > 80.)
				plotCriteria_._projection = orthographic;
			//
			// high latitudes
			else if (fabs(centre_height_) > 60.)
				plotCriteria_._projection = albersConic;
			// sparse latitude coverage
			else if ((upperDeg_ - lowerDeg_) > Degrees(QUART_PI))	// taller than 45 degrees?
			{
				// tall, narrow plots
				if ((rightDeg_ - leftDeg_) < Degrees(QUART_PI))		// not more than 45 degrees wide?
					plotCriteria_._projection = orthographic;
				// reasonably large area
				else
					plotCriteria_._projection = mercator;
			}
			//
			// Smallish areas
			else if (scale < 750000 && scale > 5000)
				plotCriteria_._projection = transverseMercator;
			//
			// wider than 20 degrees and taller than 20 degrees?
			else if ((rightDeg_ - leftDeg_) > Degrees(PI_20) && (upperDeg_ - lowerDeg_) > Degrees(PI_20))
				plotCriteria_._projection = stereographicConformal;
			// 
			// wide plots
			else if ((rightDeg_ - leftDeg_) > Degrees(THIRD_PI))	// wider than 90 degrees?
				
				plotCriteria_._projection = lambertEqualArea;
		}
	}

	// Update calling app's parameters
	plotCriteria->_projection = plotCriteria_._projection;
	plotCriteria->_ground_width = dDimension;

	// Normalise scale
	NormaliseScale(scale);
	mapScale_ = scale;

	// Calculate scale bar, then round
	double scale_bar_width = dDimension / 3000.;		// convert to kilometres
	NormaliseScaleBar(scale_bar_width);

	// Calculate best graticule width, then round to the best integer
	double graticule_width((rightDeg_ - leftDeg_) / 4.);
	UINT32 graticule_width_precision(12);
	NormaliseGraticule(graticule_width, graticule_width_precision);

	int scale_precision = 0;
	if (scale_bar_width < 1.0)
		scale_precision = 4;

	double line_width(projectSettings_.p._msr_line_width);
	double circle_radius(0.15);
	double circle_radius_2(circle_radius);
	double circle_line_width(0.1);

	//if (scale >= 900000)
	//{
	//	//line_width = 0.05;
	//	circle_radius = 0.05;
	//	circle_line_width = 0.05;
	//}

	// Determine coastline resolution
	string coastResolution("l");
	SelectCoastlineResolution(dDimension, coastResolution, plotCriteria);

	UINT32 block(0), block_index, i, colours(0), precision(10);

	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block = plotCriteria_._plot_block_number - 1;
	
	it_pair_string _it_colour;
	string_string_pair this_msr;

	UINT32 columns(5);
	double symbol_offset(0.5);
	double label_offset(1.0);
	double error_ellipse_scale(uncertainty_legend_length_/largest_uncertainty_);
	UINT32 uncertainty_legend_precision(4);

	for (; block<blockCount_; block++)
	{
		colours = 0;
		if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
			block_index = 0;
		else
			block_index = block;

		if (isLandscape)
			gmtbat_file_ << "gmtset PAGE_ORIENTATION landscape" << endl << endl;
		else
			gmtbat_file_ << "gmtset PAGE_ORIENTATION portrait" << endl << endl;
		
		gmtbat_file_ << "gmtset ANNOT_FONT 0" << endl;	
		gmtbat_file_ << "gmtset ANNOT_FONT_SIZE_PRIMARY " << fixed << setprecision(1) << annot_font_size_primary << "p" << endl;
		gmtbat_file_ << "gmtset LABEL_FONT_SIZE " << fixed << setprecision(1) << label_font_size << "p" << endl;
		if (graticule_width < 60./3600.)
			gmtbat_file_ << "gmtset PLOT_DEGREE_FORMAT ddd:mm:ss.x" << endl;
		else
			gmtbat_file_ << "gmtset PLOT_DEGREE_FORMAT ddd:mm" << endl;
		gmtbat_file_ << endl;

		page_width = plotCriteria->_page_width;

		switch (plotCriteria_._projection)
		{
		case world:
			// World map centered on the dateline
			// pscoast -Rg -JQ4.5i -B60f30g30 -Dc -A5000 -Gblack -P > tmp.ps
			
			// Override coastline resolution
			coastResolution = "i";
			plotCriteria->_coasline_resolution = intermediate;
			circle_radius = 0.02;
			circle_line_width = 0.02;
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 5;

			gmtbat_file_ << "pscoast -Rg" << 
				// Carree Cylindrical equidistant projection, which looks the nicest
				" -JQ" << page_width  << "c -B60g30 -D" << coastResolution << " -A10000 -W0.75p,16/169/243 -G245/245/245 -K > tmp.eps" << endl;
				//
				// Miller's Cylindrical projection, which is neither equal nor conformal. All meridians and parallels are straight lines.
				// " -JJ" << page_width  << "c -B60g30 -D" << coastResolution << " -A10000 -W0.75p,16/169/243 -G245/245/245 -P -K > tmp.eps" << endl;
				//
				// Cylindrical equal-area projection
				//" -JY" << page_width  << "c -B60g30 -D" << coastResolution << " -A10000 -W0.75p,16/169/243 -G245/245/245 -P -K > tmp.eps" << endl;
			break;
		//
		// Orthographic projection
		case orthographic:
				
			// Override coastline resolution
			coastResolution = "h";
			plotCriteria->_coasline_resolution = high;
			circle_radius = 0.05;
			circle_line_width = 0.05;
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 5;

			gmtbat_file_ << "pscoast -Rg -JG" << 
				fixed << setprecision(7) << centre_width_ << "/" <<		// longitude
				fixed << setprecision(7) << centre_height_ << "/" <<		// latitude
				fixed << setprecision(0) << page_width << "c -B30g15 -D" << coastResolution << " -A10000 -W0.75p,16/169/243 -G245/245/245 -P -K > tmp.eps" << endl;
			break;
		//
		// Mercator projection
		case mercator:
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 6;

			gmtbat_file_ << "pscoast -R" << 
			fixed << setprecision(7) << leftDeg_ << "/" <<
			fixed << setprecision(7) << lowerDeg_ << "/" <<
			fixed << setprecision(7) << rightDeg_ << "/" <<
			fixed << setprecision(7) << upperDeg_;

			// example: -Jm1.2e-2i
			gmtbat_file_ << "r -JM" << fixed << setprecision(0) << page_width << "c";
			gmtbat_file_ << " -D" << coastResolution << " -N2/0.25tap -W0.75p,16/169/243 -G255/255/255 -S233/246/255 -Lf" <<
				fixed << setprecision(5) << centre_width_ << "/" << fixed << setprecision(5) << dScaleLat << "/" << 
				fixed << setprecision(5) << centre_height_ << "/" << fixed << setprecision(scale_precision) << scale_bar_width << "k+lKilometres+jt " <<
				"-B" <<
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width << "/" << 
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width;

			if (!isLandscape)
				gmtbat_file_ << " -P";
			gmtbat_file_ << " -K > tmp.eps" << endl;
			
			break;
		//
		// Transverse Mercator projection
		case transverseMercator:
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 6;

			gmtbat_file_ << "pscoast -R" << 
			fixed << setprecision(7) << leftDeg_ << "/" <<
			fixed << setprecision(7) << lowerDeg_ << "/" <<
			fixed << setprecision(7) << rightDeg_ << "/" <<
			fixed << setprecision(7) << upperDeg_;

			// example: -Jt139.9944444/-24.1486111/1:1000000
			//gmtbat_file_ << "r -Jt" << fixed << setprecision(7) << centre_width_ << "/" <<
			//	fixed << setprecision(7) << centre_height_ << "/1:" << fixed << setprecision(0) << scale;
			gmtbat_file_ << "r -JT" << fixed << setprecision(7) << centre_width_ << "/" <<
				fixed << setprecision(0) << page_width << "c";

			gmtbat_file_ << " -D" << coastResolution << " -N2/0.25tap -W0.75p,16/169/243 -G255/255/255 -S233/246/255 -Lf" <<
				fixed << setprecision(5) << centre_width_ << "/" << fixed << setprecision(5) << dScaleLat << "/" << 
				fixed << setprecision(5) << centre_height_ << "/" << fixed << setprecision(scale_precision) << scale_bar_width << "k+lKilometres+jt " <<
				"-B" <<
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width << "/" << 
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width;

			if (!isLandscape)
				gmtbat_file_ << " -P";
			gmtbat_file_ << " -K > tmp.eps" << endl;
			
			break;
		//
		// Albers conic equal-area projection
		case albersConic:
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 6;

			gmtbat_file_ << "pscoast -R" << 
				fixed << setprecision(7) << leftDeg_ << "/" <<
				fixed << setprecision(7) << lowerDeg_ << "/" <<
				fixed << setprecision(7) << rightDeg_ << "/" <<
				fixed << setprecision(7) << upperDeg_;

			// example: -Jb136.5/-36/-18/-36/1:45000000
			//gmtbat_file_ << "r -Jb" << fixed << setprecision(7) << centre_width_ << "/" <<
			//	fixed << setprecision(7) << centre_height_ << "/" <<
			//	fixed << setprecision(7) << upperDeg_ - fabs(dHeight/3.) << "/" <<
			//	fixed << setprecision(7) << lowerDeg_ + fabs(dHeight/3.) << "/" <<				
			//	"1:" << fixed << setprecision(0) << scale;
			gmtbat_file_ << "r -JB" << fixed << setprecision(7) << centre_width_ << "/" <<
				fixed << setprecision(7) << centre_height_ << "/" <<
				fixed << setprecision(7) << upperDeg_ - fabs(dHeight/3.) << "/" <<
				fixed << setprecision(7) << lowerDeg_ + fabs(dHeight/3.) << "/" <<				
				fixed << setprecision(0) << page_width << "c";

			gmtbat_file_ << " -D" << coastResolution << " -N2/0.25tap -W0.75p,16/169/243 -G255/255/255 -S233/246/255 -Lf" <<
				fixed << setprecision(5) << centre_width_ << "/" << fixed << setprecision(5) << dScaleLat << "/" << 
				fixed << setprecision(5) << centre_height_ << "/" << fixed << setprecision(scale_precision) << scale_bar_width << "k+lKilometres+jt " <<
				"-B" <<
				//fixed << setprecision(7) << DmstoDeg(0.3) << "g" << fixed << setprecision(7) << DmstoDeg(0.3) << " -K > tmp.eps" << endl;
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width << "/" << 
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width;

			if (!isLandscape)
				gmtbat_file_ << " -P";
			gmtbat_file_ << " -K > tmp.eps" << endl;
			break;
		//
		// Lambert Azimuthal Equal-Area		
		case lambertEqualArea:		
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 6;

			gmtbat_file_ << "pscoast -R" << 
			fixed << setprecision(7) << leftDeg_ << "/" <<
			fixed << setprecision(7) << lowerDeg_ << "/" <<
			fixed << setprecision(7) << rightDeg_ << "/" <<
			fixed << setprecision(7) << upperDeg_;

			// example: -JA30/-30/4.5i
			gmtbat_file_ << "r -JA" << fixed << setprecision(7) << centre_width_ << "/" <<
				fixed << setprecision(7) << centre_height_ << "/" <<
				fixed << setprecision(0) << page_width << "c";

			gmtbat_file_ << " -D" << coastResolution << " -N2/0.25tap -W0.75p,16/169/243 -G255/255/255 -S233/246/255 -Lf" <<
				fixed << setprecision(5) << centre_width_ << "/" << fixed << setprecision(5) << dScaleLat << "/" << 
				fixed << setprecision(5) << centre_height_ << "/" << fixed << setprecision(scale_precision) << scale_bar_width << "k+lKilometres+jt " <<
				"-B" <<
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width << "/" << 
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width;

			if (!isLandscape)
				gmtbat_file_ << " -P";
			gmtbat_file_ << " -K > tmp.eps" << endl;
			
			break;
		//
		// General stereographic map
		case stereographicConformal:
		default:
			if (plotCriteria_._label_font_size < 0.)
				plotCriteria_._label_font_size = 6;

			gmtbat_file_ << "pscoast -R" << 
			fixed << setprecision(7) << leftDeg_ << "/" <<
			fixed << setprecision(7) << lowerDeg_ << "/" <<
			fixed << setprecision(7) << rightDeg_ << "/" <<
			fixed << setprecision(7) << upperDeg_;

			// example: -R100/-40/160/-10r -JS130/-30/4i
			gmtbat_file_ << "r -JS" << fixed << setprecision(7) << centre_width_ << "/" <<
				fixed << setprecision(7) << centre_height_ << "/" <<
				fixed << setprecision(0) << page_width << "c";

			gmtbat_file_ << " -D" << coastResolution << " -N2/0.25tap -W0.75p,16/169/243 -G255/255/255 -S233/246/255 -Lf" <<
				fixed << setprecision(5) << centre_width_ << "/" << fixed << setprecision(5) << dScaleLat << "/" << 
				fixed << setprecision(5) << centre_height_ << "/" << fixed << setprecision(scale_precision) << scale_bar_width << "k+lKilometres+jt " <<
				"-B" <<
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width << "/" << 
				fixed << setprecision(graticule_width_precision) << graticule_width << "g" << fixed << setprecision(graticule_width_precision) << graticule_width;

			if (!isLandscape)
				gmtbat_file_ << " -P";
			gmtbat_file_ << " -K > tmp.eps" << endl;
			
			break;
		}

			
		sort(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), ComparePairFirst<string>());		

		// Does the user want to print measurements?
		if (!plotCriteria_._omit_measurements)
		{
			// print latitude, longitude and height measurements (large circles) before stations
			for (i=0; i<v_msr_file_.at(block).size(); i++)
			{
				if (v_msr_file_.at(block).at(i).second != "H" &&
					v_msr_file_.at(block).at(i).second != "I" &&
					v_msr_file_.at(block).at(i).second != "J" &&
					v_msr_file_.at(block).at(i).second != "P" &&
					v_msr_file_.at(block).at(i).second != "Q" &&
					v_msr_file_.at(block).at(i).second != "R" &&
					v_msr_file_.at(block).at(i).second != "Y")
					continue;

				circle_radius_2 = circle_radius;

				if (v_msr_file_.at(block).at(i).second == "I" ||		// astronomic latitude
					v_msr_file_.at(block).at(i).second == "P")			//   geodetic latitude
					circle_radius_2 = circle_radius * 3;
				else if (v_msr_file_.at(block).at(i).second == "J" ||	// astronomic longitude
					v_msr_file_.at(block).at(i).second == "Q")			//   geodetic longitude
					circle_radius_2 = circle_radius * 2.4;
				else if (v_msr_file_.at(block).at(i).second == "H" ||	// orthometric height
					v_msr_file_.at(block).at(i).second == "R")			// ellipsoidal height
					circle_radius_2 = circle_radius * 1.7;
				else if (v_msr_file_.at(block).at(i).second == "Y")		// GPS point cluster
					circle_radius_2 = circle_radius * 4;

				_it_colour = equal_range(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), 
					v_msr_file_.at(block).at(i).second, ComparePairFirst<string>());

				if (_it_colour.first == _it_colour.second)
				{
					gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius_2 << " \"" << 
						v_msr_file_.at(block).at(i).first << "\" -W" << setprecision(2) << circle_line_width << 
						"p,darkgray -Glightgray";
					gmtbat_file_ << " -O -K >> tmp.eps" << endl;
				}
				else
				{
					colours++;
					gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius_2 << " \"" << 
						v_msr_file_.at(block).at(i).first << "\" -W" << setprecision(2) << circle_line_width << 
						"p,";

					// Line colour
					if (v_msr_file_.at(block).at(i).second == "Y")
						gmtbat_file_ << "255/0/255";								// magenta, #FF00FF
					else if (v_msr_file_.at(block).at(i).second == "I" ||			// astronomic latitude
						v_msr_file_.at(block).at(i).second == "P")				//   geodetic latitude
						gmtbat_file_ << "252/181/20";							// packer gold, #FCB514
					else if (v_msr_file_.at(block).at(i).second == "J" ||			// astronomic longitude
						v_msr_file_.at(block).at(i).second == "Q")				//   geodetic longitude
						gmtbat_file_ << "255/128/0";								// orange, #FF8000
					else if (v_msr_file_.at(block).at(i).second == "H" ||			// orthometric height
						v_msr_file_.at(block).at(i).second == "R")				// ellipsoidal height
						gmtbat_file_ << "227/54/56";								// rosemadder, #E33638
					else
						gmtbat_file_ << _it_colour.first->second;

					// Fill colour
					gmtbat_file_ << " -G" << _it_colour.first->second  << " -O -K >> tmp.eps" << endl;
				}
			}
		}

		// print stations first to enable measurements to be seen
		gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius << " \"" << 
			v_isl_pts_file_.at(block_index) << "\" -W" << setprecision(2) << circle_line_width << 
			"p,blue -Gwhite -O -K >> tmp.eps" << endl;

		if (plotConstraints_)
			// don't plot line, just fill
			gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius << " \"" << 
				v_isl_const_file_.at(block_index) << "\" -Gblue -O -K >> tmp.eps" << endl;

		if (plotBlocks_)
		{
			gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius << " \"" << 
				v_jsl_pts_file_.at(block_index) << "\" -W" << setprecision(2) << circle_line_width * 2.0 << 
				"p,red -Gwhite -O -K >> tmp.eps" << endl;

			if (plotConstraints_)
				// don't plot line, just fill
				gmtbat_file_ << "psxy -R -J -Skcircle/" << fixed << setprecision(2) << circle_radius << " \"" << 
					v_jsl_const_file_.at(block_index) << "\" -Gred -O -K >> tmp.eps" << endl;
		}

		// Does the user want to print measurements?
		if (!plotCriteria_._omit_measurements)
		{
			// now print lines
			for (i=0; i<v_msr_file_.at(block).size(); i++)
			{
				if (v_msr_file_.at(block).at(i).second == "H")		// already printed
					continue;
				if (v_msr_file_.at(block).at(i).second == "I")		// ''
					continue;
				if (v_msr_file_.at(block).at(i).second == "J")		// ''
					continue;
				if (v_msr_file_.at(block).at(i).second == "P")		// ''
					continue;
				if (v_msr_file_.at(block).at(i).second == "Q")		// ''
					continue;			
				if (v_msr_file_.at(block).at(i).second == "R")		// ''
					continue;
				if (v_msr_file_.at(block).at(i).second == "Y")		// ''
					continue;
					
				_it_colour = equal_range(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), 
					v_msr_file_.at(block).at(i).second, ComparePairFirst<string>());

				if (_it_colour.first == _it_colour.second)
				{
					gmtbat_file_ << "psxy -R -J \"" << v_msr_file_.at(block).at(i).first << 
						"\" -m# -W" << setprecision(2) << line_width << 
						"p,lightgray";
					//if (v_msr_file_.at(block).at(i).second == "Y")		// not a vector measurement, so represent as dashed
					//	gmtbat_file_ << ",6_8:1p";
					gmtbat_file_ << " -O -K >> tmp.eps" << endl;
				}
				else
				{
					colours++;
					gmtbat_file_ << "psxy -R -J \"" << v_msr_file_.at(block).at(i).first << 
						"\" -m# -W" << setprecision(2) << line_width << 
						"p," << _it_colour.first->second;
					//if (v_msr_file_.at(block).at(i).second == "Y")		// not a vector measurement, so represent as dashed
					//	gmtbat_file_ << ",6_8:1p";
					gmtbat_file_ << " -O -K >> tmp.eps" << endl;
				}
			}
		}

		if (plotCriteria_._plot_positional_uncertainty || 
			plotCriteria_._plot_error_ellipses)
		{
			// calculate scale for uncertainty circles, to the end that 
			// largest_uncertainty_ is uncertainty_legend_length_ cm on the A3 page
			//if (largest_uncertainty_ >= 1.0)
			//	uncertainty_legend_precision = 0;
			/*else*/ if ((largest_uncertainty_ / plotCriteria_._pu_ellipse_scale) >= 0.1)
				uncertainty_legend_precision = 1;
			else if ((largest_uncertainty_ / plotCriteria_._pu_ellipse_scale) >= 0.01)
				uncertainty_legend_precision = 2;
			else if ((largest_uncertainty_ / plotCriteria_._pu_ellipse_scale) >= 0.001)
				uncertainty_legend_precision = 3;
			else
				uncertainty_legend_precision = 4;
		}

		if (plotCriteria_._plot_positional_uncertainty)
		{
			// print positional uncertainty
			gmtbat_file_ << "psvelo -R -J \"" << v_stn_apu_file_.at(block_index) << 
				"\" -Sr" << error_ellipse_scale << "/0.95/0c -L -W0.75p,orange -O -K >> tmp.eps" << endl;

			// Add text for error ellipse legend 
			gmtbat_file_ << "echo " << 
				setprecision(precision) << fixed << left << uncertainty_legend_long_ << " " << 
				setprecision(precision) << fixed << left << uncertainty_legend_lat_ << " " << 
				 plotCriteria_._label_font_size <<						// font size
				" 0 0 LM " <<											// font angle, font number, justification
				" 95%% positional uncertainty " <<						// the label
				setprecision(uncertainty_legend_precision) <<			// ''
				fixed << largest_uncertainty_ / plotCriteria_._pu_ellipse_scale << " radius (m)" <<					// radius
				" | ";													// Push to pstext
			gmtbat_file_ << 
				"pstext -R -J -X" << label_font_size * 3. << "p ";		// label offset from centre point
			
			// is positional uncertainty being printed? If so, shift text up
			if (plotCriteria_._plot_error_ellipses)
				gmtbat_file_ << "-Y" << label_font_size * 0.6 << "p ";
			
			gmtbat_file_ << " -O -K >> tmp.eps" << endl;
			
			// Terminate a sequence of GMT plotting commands without producing any plotting output
			gmtbat_file_ << "psxy -R -J -T -X-" << label_font_size * 3. << "p ";

			if (plotCriteria_._plot_error_ellipses)
				gmtbat_file_ << "-Y-" << label_font_size * 0.6 << "p ";
			gmtbat_file_ << " -O -K >> tmp.eps" << endl;
		}
		
		if (plotCriteria_._plot_error_ellipses)
		{
			double ellipse_scale(1.);

			// The error ellipse legend is printed using the largest uncertainty (semi-major) found, unless
			// positional uncertainty is being printed, in which case the error ellipse is printed using
			// half the size.  This is purely to give the impression that error ellipses are 1 sigma (68%) and
			// positional uncertainty is 95%, or close to 2 sigma.
			if (plotCriteria_._plot_positional_uncertainty)
				ellipse_scale = 2.;

			// print error ellipses in red
			gmtbat_file_ << "psvelo -R -J \"" << v_stn_err_file_.at(block_index) << 
				"\" -Sr" << error_ellipse_scale << "/0.95/0c -L -W0.75p,red -O -K >> tmp.eps" << endl;

			// Add text for error ellipse legend 
			gmtbat_file_ << "echo " << 
				setprecision(precision) << fixed << left << uncertainty_legend_long_ << " " << 
				setprecision(precision) << fixed << left << uncertainty_legend_lat_ << " " << 
				 plotCriteria_._label_font_size <<						// font size
				" 0 0 LM " <<											// font angle, font number, justification
				" 1 sigma error ellipse " <<							// the label
				setprecision(uncertainty_legend_precision) <<			// ''
				fixed << largest_uncertainty_ / ellipse_scale / plotCriteria_._pu_ellipse_scale <<		// semi-major
				", " <<				
				fixed << largest_uncertainty_ / 3.0 / ellipse_scale / plotCriteria_._pu_ellipse_scale <<	// semi-minor (make it third the height)
				" (m)" <<
				" | ";													// Push to pstext
			gmtbat_file_ << 
				"pstext -R -J -X" << label_font_size * 3. << "p ";		// label offset from centre point
			
			// is positional uncertainty being printed? If so, shift text down
			if (plotCriteria_._plot_positional_uncertainty)
				gmtbat_file_ << "-Y-" << label_font_size * 0.6 << "p ";
			
			gmtbat_file_ << "-O -K >> tmp.eps" << endl;
			
			// Terminate a sequence of GMT plotting commands without producing any plotting output
			gmtbat_file_ << "psxy -R -J -T -X-" << label_font_size * 3. << "p ";
			if (plotCriteria_._plot_positional_uncertainty)
				gmtbat_file_ << "-Y" << label_font_size * 0.6 << "p ";
			gmtbat_file_ << " -O -K >> tmp.eps" << endl;
		}
		
		if (plotCriteria_._plot_correction_arrows)
		{
			// calculate scale for arrows, to the end that 
			// average_correction_ is 3cm on the A3 page
			if (average_correction_ < PRECISION_1E4)
				average_correction_ = PRECISION_1E4;
			double correction_arrow_scale(arrow_legend_length_/average_correction_);
			UINT32 correction_legend_precision(4);

			//if (average_correction_ >= 1.0)
			//	correction_legend_precision = 0;
			/*else*/ if (average_correction_ >= 0.1)
				correction_legend_precision = 1;
			else if (average_correction_ >= 0.01)
				correction_legend_precision = 2;
			else if (average_correction_ >= 0.001)
				correction_legend_precision = 3;
			else
				correction_legend_precision = 4;


			// print text in black (no arrows)
			if (plotCriteria_._plot_correction_labels)
				gmtbat_file_ << "psvelo -R -J \"" << v_stn_cor_file_.at(block_index) << 
					"\" -Se0.0001/0.95/" << plotCriteria_._label_font_size << "c -L -A0.0001/0.0001/0.0001c -O -K >> tmp.eps" << endl;

			// print arrows (without black outline!)
			gmtbat_file_ << "psvelo -R -J \"" << v_stn_cor_file_.at(block_index) << 
				"\" -Se" << correction_arrow_scale << "/0.95/0c -L -A" << line_width / 2.0 * CM_TO_INCH << "/0.5/0.1c -Gred -W0.0p,red -O -K >> tmp.eps" << endl;

			// Add text below the corrections arrow legend 
			gmtbat_file_ << "echo " << 
				setprecision(precision) << fixed << left << arrow_legend_long_ << " " << 
				setprecision(precision) << fixed << left << arrow_legend_lat_ << " " << 
				plotCriteria_._label_font_size <<						// font size
				" 0 0 LM " <<											// font angle, font number, justification
				setprecision(correction_legend_precision) <<			// the label
				fixed << average_correction_ / plotCriteria_._correction_scale <<	" (m)" <<				// ''
				" | ";													// Push to pstext
			gmtbat_file_ << 
				"pstext -R -J -Y-" << label_font_size << "p ";			// label (vertical) offset below arrow
			gmtbat_file_ << "-O -K >> tmp.eps" << endl;
			
			// Add text above for corrections legend 
			gmtbat_file_ << "echo " << 
				setprecision(precision) << fixed << left << arrow_legend_long_ << " " << 
				setprecision(precision) << fixed << left << arrow_legend_lat_ << " " << 
				plotCriteria_._label_font_size <<						// font size
				" 0 0 LM " <<											// font angle, font number, justification
				"Corrections scale" <<									// the label
				" | ";													// Push to pstext
			gmtbat_file_ << 
				"pstext -R -J -Y" << 2. * label_font_size << "p ";		// label (vertical) offset above arrow
			gmtbat_file_ << "-O -K >> tmp.eps" << endl;
			
			// Terminate a sequence of GMT plotting commands without producing any plotting output
			gmtbat_file_ << "psxy -R -J -T -Y-" << label_font_size << "p -O -K >> tmp.eps" << endl;
		}

		if (plotCriteria_._plot_station_labels)
		{
			gmtbat_file_ << "pstext -R -J \"" << v_isl_lbl_file_.at(block_index);
			//// print white box
			//gmtbat_file_ << "\" -Wwhiteo0.5p,white";
			// print shadow
			gmtbat_file_ << "\" -S" << plotCriteria_._label_font_size/3.0 << "p,white";
			// shift
			gmtbat_file_ << " -X" << plotCriteria_._label_font_size/2.0 + sqrt(plotCriteria_._label_font_size) << "p -O -K >> tmp.eps" << endl;
			if (plotBlocks_)
			{
				gmtbat_file_ << "pstext -R -J \"" << v_jsl_lbl_file_.at(block_index);
				//// print white box
				//gmtbat_file_ << "\" -Wwhiteo0.5p,white";
				// print shadow
				gmtbat_file_ << "\" -S" << plotCriteria_._label_font_size/3.0 << "p,white";
				// finish
				gmtbat_file_ << " -O -K >> tmp.eps" << endl;			
			}
		}

		// Prepare to terminate a sequence of GMT plotting commands without producing 
		// any plotting output (depending on whether station labels were required)
		gmtbat_file_ << "psxy -R -J -T -O";
		

		// Put image back, so that the scale is centred beneath the plot
		if (plotCriteria_._plot_station_labels)
			gmtbat_file_ << " -X-" << plotCriteria_._label_font_size/2.0 + sqrt(plotCriteria_._label_font_size) << "p";

		// No title block?
		if (plotCriteria_._omit_title_block)
		{
			// Terminate a sequence of GMT plotting commands without producing any plotting output
			gmtbat_file_ << " >> tmp.eps" << endl;
		}
		else
		{
			gmtbat_file_ << " -K >> tmp.eps" << endl << endl;
			
			// legend
			gmtbat_file_ << "gmtset HEADER_FONT 1" << endl;
			
			gmtbat_file_ << " " << endl;
			gmtbat_file_ << "echo G 0.25 > legend" << endl;
			gmtbat_file_ << "echo N 1 >> legend" << endl;
			gmtbat_file_ << "echo H " << fixed << setprecision(0) << label_font_size * 1.5 << " - " << network_name_;
			if (blockCount_ > 1)
				gmtbat_file_ << "  (Block " << block + 1 << " of " << blockCount_ << ") ";

			if (!plotCriteria_._plot_station_centre.empty())
				gmtbat_file_ << " centred on " << plotCriteria_._plot_station_centre;

			gmtbat_file_ << " >> legend" << endl;
			gmtbat_file_ << "echo G 0.25 >> legend" << endl;

			// Print stations legend
			gmtbat_file_ << "echo D 0 1p >> legend" << endl;		// horizontal line
			gmtbat_file_ << "echo G 0.25 >> legend" << endl;		// space	

			UINT32 station_count(1);		// Simultaneous or ISL

			if (plotBlocks_)
				station_count++;			// JSL
			
			if (plotConstraints_)
			{
				station_count++;			// Simultaneous or ISL constraint stations
				if (plotBlocks_)
					station_count++;		// JSL constraint stations				
			}
			
			//if (plotCriteria_._plot_correction_arrows)
			//	station_count++;			// Corrections to station coordinates

			gmtbat_file_ << "echo N " << station_count << " >> legend" << endl;
			gmtbat_file_ << "echo V 0 1p >> legend" << endl;		// vertical line
			
			// Simultaneous stations or Phased inner stations
			gmtbat_file_ << 
				"echo S " << fixed << setprecision(1) << symbol_offset << 
				" c 0.2 white " << circle_line_width * 2 << "p,blue " <<
				fixed << setprecision(1) << label_offset;
			if (plotBlocks_)
				gmtbat_file_ << " Free Inner stations ";
			else 
				gmtbat_file_ << " Free Stations";
			gmtbat_file_ << " >> legend" << endl;

			// Simultaneous stations or Phased inner stations
			if (plotBlocks_)
			{
				gmtbat_file_ << 
					"echo S " << fixed << setprecision(1) << symbol_offset << 
					" c 0.2 white " << circle_line_width * 2 << "p,red " <<
					fixed << setprecision(1) << label_offset <<
					" Free Junction stations >> legend" << endl;
			}

			if (plotConstraints_)
			{
				if (plotBlocks_)
				{
					gmtbat_file_ << 
						"echo S " << fixed << setprecision(1) << symbol_offset << 
						" c 0.2 blue " << circle_line_width * 2 << "p,blue " <<
						fixed << setprecision(1) << label_offset <<
						" Constrained inner stations >> legend" << endl;
					gmtbat_file_ << 
						"echo S " << fixed << setprecision(1) << symbol_offset << 
						" c 0.2 red " << circle_line_width * 2 << "p,red " <<
						fixed << setprecision(1) << label_offset <<
						" Constrained junction stations >> legend" << endl;
				}
				else
					gmtbat_file_ << 
						"echo S " << fixed << setprecision(1) << symbol_offset << 
						" c 0.2 blue " << circle_line_width * 2 << "p,blue " <<
						fixed << setprecision(1) << label_offset <<
						" Constraint stations >> legend" << endl;
			}			
			
			title_block_height = 5;

			if (plotCriteria_._plot_correction_arrows)
			{
			//	// Add corrections to end of stations legend
			//	gmtbat_file_ << 
			//		"echo S " << fixed << setprecision(1) << symbol_offset << 
			//		" v 1.5c/0.04/0.5/0.1 red " <<			// arrowlength/linewidth/arrowheadwidth/arrowheadlength
			//		line_width * 2 << "p,red " 
			//		<< fixed << setprecision(1) << label_offset * 1.5 << " Corrections to stations" << " >> legend" << endl;
			}
			// If corrections are not being printed, then print measurements legend
			else
			{
				gmtbat_file_ << "echo G 0.25 >> legend" << endl;		// space
				gmtbat_file_ << "echo D 0 1p >> legend" << endl;		// horizontal line
				gmtbat_file_ << "echo G 0.25 >> legend" << endl;		// space	

				if (v_msr_file_.at(block).size() > columns)
					gmtbat_file_ << "echo N " << columns << " >> legend" << endl;
				else
					gmtbat_file_ << "echo N " << v_msr_file_.at(block).size() << " >> legend" << endl;

				bool bOtherTypes(false);
				
				if (colours < columns)
				{
					symbol_offset = 1.0;
					label_offset = 1.5;
				}

				for (i=0; i<v_msr_file_.at(block).size(); i++)
				{
					_it_colour = equal_range(plotCriteria_._msr_colours.begin(), plotCriteria_._msr_colours.end(), 
						v_msr_file_.at(block).at(i).second, ComparePairFirst<string>());

					if (_it_colour.first != _it_colour.second)
					{
						gmtbat_file_ << "echo S " << fixed << setprecision(1) << symbol_offset;

						circle_radius_2 = circle_radius;

						if (v_msr_file_.at(block).at(i).second == "I" ||		// astronomic latitude
							v_msr_file_.at(block).at(i).second == "P")			//   geodetic latitude
							circle_radius_2 = circle_radius * 2.25;
						else if (v_msr_file_.at(block).at(i).second == "J" ||	// astronomic longitude
							v_msr_file_.at(block).at(i).second == "Q")			//   geodetic longitude
							circle_radius_2 = circle_radius * 2.0;
						else if (v_msr_file_.at(block).at(i).second == "H" ||	// orthometric height
							v_msr_file_.at(block).at(i).second == "R")			// ellipsoidal height
							circle_radius_2 = circle_radius * 1.75;
						else if (v_msr_file_.at(block).at(i).second == "Y")		// GPS point cluster
							circle_radius_2 = circle_radius * 2.5;

						// print a circle or line?
						if (v_msr_file_.at(block).at(i).second == "I" ||		// astronomic latitude
							v_msr_file_.at(block).at(i).second == "P" ||		//   geodetic latitude
							v_msr_file_.at(block).at(i).second == "J" ||		// astronomic longitude
							v_msr_file_.at(block).at(i).second == "Q" ||		//   geodetic longitude
							v_msr_file_.at(block).at(i).second == "H" ||		// orthometric height
							v_msr_file_.at(block).at(i).second == "R" ||		// ellipsoidal height
							v_msr_file_.at(block).at(i).second == "Y")			// GPS point cluster
							gmtbat_file_ << " c " << circle_radius_2 << " " << _it_colour.first->second << " " << circle_line_width * 2;	// circle
						else
							gmtbat_file_ << " - 0.5 " << _it_colour.first->second << " " << line_width * 5;			// line

						gmtbat_file_ << "p,";

						if (v_msr_file_.at(block).at(i).second == "Y")
							gmtbat_file_ << "#FF00FF";								// magenta, #FF00FF
						else if (v_msr_file_.at(block).at(i).second == "I" ||		// astronomic latitude
							v_msr_file_.at(block).at(i).second == "P")				// geodetic latitude
							gmtbat_file_ << "#FCB514";								// packer gold, #FCB514
						else if (v_msr_file_.at(block).at(i).second == "J" ||		// astronomic longitude
							v_msr_file_.at(block).at(i).second == "Q")				// geodetic longitude
							gmtbat_file_ << "#FF8000";								// orange, #FF8000
						else if (v_msr_file_.at(block).at(i).second == "H" ||		// orthometric height
							v_msr_file_.at(block).at(i).second == "R")				// ellipsoidal height
							gmtbat_file_ << "#E33638";								// rosemadder, #E33638
						else
							gmtbat_file_ << _it_colour.first->second;

						gmtbat_file_ << " " << fixed << setprecision(1) << label_offset << " " << 
							measurement_name<char, string>(static_cast<char>(_it_colour.first->first.at(0))) << " >> legend" << endl;
					}
					else
						bOtherTypes = true;
				}

				if (bOtherTypes)
					gmtbat_file_ << 
						"echo S " << fixed << setprecision(1) << symbol_offset << " - 0.5 lightgray " << line_width * 2 << "p,lightgray " 
						<< fixed << setprecision(1) << label_offset << "  All other types" << " >> legend" << endl;			

				title_block_height += floor(((double)colours)/columns) * 0.25;
			
			}

			gmtbat_file_ << "echo G 0.25 >> legend" << endl;		// space
			gmtbat_file_ << "echo D 0 1p >> legend" << endl;		// horizontal line
			gmtbat_file_ << "echo G 0.25 >> legend" << endl;		// space
			gmtbat_file_ << "echo N 5 >> legend" << endl;

			gmtbat_file_ << "echo S 0.01 c 0.01 white 1p,white 1 Geodesy >> legend" << endl;
			gmtbat_file_ << "echo S 0.01 c 0.01 white 1p,white 0 Surveyor-General Victoria >> legend" << endl;
			gmtbat_file_ << "echo S 0.01 c 0.01 white 1p,white 3 GDA2020 >> legend" << endl;
			gmtbat_file_ << "echo S 0.01 c 0.01 white 1p,white 0 " << projectionTypes[plotCriteria_._projection] << " projection >> legend" << endl;
			gmtbat_file_ << "echo S 0.01 c 0.01 white 1p,white 1 Scale 1:" << static_cast<UINT32>(scale) << " \"(A3)\" >> legend" << endl;

			gmtbat_file_ << "echo pslegend -R0/" << fixed << setprecision(1) << page_width << "/0/1.5 -Jx1/1 -O -Dx0/0/" <<
				fixed << setprecision(1) << page_width << "/" << setprecision(1) << title_block_height << "/TL -Y-1.2 -F legend -Sscript2.bat > script1.bat" << endl;
#if defined(_WIN32) || defined(__WIN32__)
			gmtbat_file_ << "call script1.bat" << endl;
			gmtbat_file_ << "call script2.bat >> tmp.eps" << endl;
#elif defined(__linux) || defined(sun) || defined(__unix__)
			gmtbat_file_ << "sh script1.bat" << endl;
			gmtbat_file_ << "sh script2.bat >> tmp.eps" << endl;
#endif
			
			
		}

		if (blockCount_ > 1)
		{
			stringstream ss("");
			string s_epsname(epsname);
			ss << "_block" << block + 1;
			s_epsname = s_epsname.replace(s_epsname.find("."), 1, ss.str() + "."); 
			gmtbat_file_ << "epstool";
			if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
				gmtbat_file_ << " --quiet";
			gmtbat_file_ << " --copy --bbox tmp.eps " << s_epsname << endl;
		}
		else
		{
			gmtbat_file_ << "epstool";
			if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
				gmtbat_file_ << " --quiet";
			gmtbat_file_ << " --copy --bbox tmp.eps " << epsname << endl;
		}

		// clean up
		gmtbat_file_ << DELETE_CMD << " tmp.eps";
		if (!plotCriteria_._omit_title_block)
			gmtbat_file_ << " legend script1.bat script2.bat";
		gmtbat_file_ << endl << endl;

		//////////////////////////////////////////////////////////////////////////////////////////
		// delete data files
		if (!plotCriteria_._keep_gen_files)
		{
			gmtbat_file_ << DELETE_CMD;

			// stations
			gmtbat_file_ << " \"" << v_isl_pts_file_.at(block_index) << "\"";
			gmtbat_file_ << " \"" << v_isl_const_file_.at(block_index) << "\"";

			if (plotBlocks_)
			{
				// junction stations5
				gmtbat_file_ << " \"" << v_jsl_const_file_.at(block_index) << "\"";
				gmtbat_file_ << " \"" << v_jsl_pts_file_.at(block_index) <<  "\"";
			}

			// measurements
			for (i=0; i<v_msr_file_.at(block).size(); i++)
				gmtbat_file_ << " \"" << v_msr_file_.at(block).at(i).first << "\"";
		
			// station labels
			if (plotCriteria_._plot_station_labels)
			{
				gmtbat_file_ << " \"" << v_isl_lbl_file_.at(block_index) << "\"";
			
				if (plotBlocks_)
					gmtbat_file_ << " \"" << v_jsl_lbl_file_.at(block_index) << "\"";
			}

			if (plotCriteria_._plot_positional_uncertainty)
				gmtbat_file_ << " \"" << v_stn_apu_file_.at(block_index) << "\"";

			if (plotCriteria_._plot_error_ellipses)
				gmtbat_file_ << " \"" << v_stn_err_file_.at(block_index) << "\"";

			if (plotCriteria_._plot_correction_arrows)
				gmtbat_file_ << " \"" << v_stn_cor_file_.at(block_index) << "\"";

			gmtbat_file_ << endl;
		}
		//////////////////////////////////////////////////////////////////////////////////////////
		
		gmtbat_file_ << endl;
		
		if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
			break;
	}

	gmtbat_file_ << DELETE_CMD << " .gmtdefaults4 .gmtcommands4" << endl << endl << endl;
	
	gmtbat_file_ << endl << "exit /b 0" << endl;
	
}
	
void dna_plot::PrintLaTeXPlotfile(const string& gmt_tex_file, const string& gmt_eps_name)
{
	std::ofstream tex_file;
	try {
		// Create latex file.  Throws runtime_error on failure.
		file_opener(tex_file, gmt_tex_file);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	tex_file << "\\documentclass[12pt]{article} " << endl;
	tex_file << "\\pagestyle{empty} " << endl;
	tex_file << "\\usepackage{ps4pdf} " << endl;
	if (isLandscape)
		tex_file << "\\usepackage[landscape,a3paper]{geometry}" << endl;
	else
		tex_file << "\\usepackage[portrait,a3paper]{geometry}" << endl;
	tex_file << "\\PSforPDF{\\usepackage{graphicx,psfrag}} " << endl;
	tex_file << "% " << endl;
	tex_file << "\\begin{document} " << endl;

	stringstream ss;
	string epsname;

	if (blockCount_ > 1)
	{
		UINT32 block = 0;
		if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
			block = plotCriteria_._plot_block_number - 1;
		
		for (; block<blockCount_; ++block)
		{
			tex_file << "% Block " << block+1 << endl;
			tex_file << "\\PSforPDF[trim=0mm 0mm 0mm 0mm]{ " << endl;

			ss.str("");
			ss << "_block" << block + 1;
			epsname = plotCriteria_._eps_file_name;
			epsname = epsname.replace(epsname.find(".eps"), 4, ss.str());

			tex_file << "\\includegraphics[scale=0.75]{" << epsname << "} " << endl;
			tex_file << "} " << endl;
			tex_file << "% " << endl;

			if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
				break;
		}
	}
	else
	{
		tex_file << "% " << endl;
		tex_file << "\\PSforPDF[trim=0mm 0mm 0mm 0mm]{ " << endl;
		tex_file << "\\includegraphics[scale=0.75]{" << plotCriteria_._eps_file_name.c_str() << "} " << endl;
		tex_file << "} " << endl;
		tex_file << "% " << endl;
	}
	tex_file << "\\end{document} " << endl;

	tex_file.close();
}

string dna_plot::PrintPdfCmdfile(const string& pdf_cmd_file, const string& gmt_tex_filename, const string& picname)
{
	std::ofstream cmd_file;
	try {
		// open PDF batch file.  Throws runtime_error on failure.
		file_opener(cmd_file, pdf_cmd_file);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	string pdfname(picname + ".pdf");
	string pdfname_tmp(picname + ".tmp.pdf");
	
	if (plotCriteria_._plot_block_number > 0)
	{
		stringstream ss_pdf;
		ss_pdf << picname << "-block-" << plotCriteria_._plot_block_number << ".pdf";
		pdfname = ss_pdf.str();
		ss_pdf.str("");
		ss_pdf << picname << "-block-" << plotCriteria_._plot_block_number << ".tmp.pdf";
		pdfname_tmp = ss_pdf.str();
	}

#if defined(_WIN32) || defined(__WIN32__)
	cmd_file << "@echo off" << endl;
#elif defined(__linux) || defined(sun) || defined(__unix__)
	cmd_file << "#!/bin/bash" << endl;
#endif

	UINT32 block(0);
	stringstream ss("");
	string epsname(plotCriteria_._eps_file_name);

	if (plotCriteria_._use_pdflatex)
	{	
		cmd_file << "latex " << gmt_tex_filename;
		if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
			cmd_file << " -interaction=batchmode";
		cmd_file << endl;
		cmd_file << "dvips -Ppdf -R0";
		if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
			cmd_file << " -q*";
		cmd_file << " -o " << picname + ".eps " << findandreplace(gmt_tex_filename, string("tex"), string("dvi")) << endl;

#if defined(_WIN32) || defined(__WIN32__)
		cmd_file << "call ";
#endif
		cmd_file << "ps2pdf ";
		
		if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
			cmd_file << "-q ";
		cmd_file << picname + ".eps " << pdfname_tmp << endl;
		cmd_file << DELETE_CMD << " " << picname + ".eps" << endl;
		cmd_file << DELETE_CMD << " " << gmt_tex_filename << endl;
		cmd_file << DELETE_CMD << " *.dvi" << endl;
		cmd_file << DELETE_CMD << " *.log" << endl;
		cmd_file << DELETE_CMD << " *.aux" << endl;
		
		if (blockCount_ > 1)
		{	
			block = 0;
			if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
				block = plotCriteria_._plot_block_number - 1;
			
			for (; block<blockCount_; ++block)
			{
				ss.str("");
				ss << "_block" << block + 1;
				epsname = plotCriteria_._eps_file_name;
				epsname = epsname.replace(epsname.find("."), 1, ss.str() + ".");
				cmd_file << DELETE_CMD << " " << epsname << endl;

				if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
					break;
			}
		}
		else
			cmd_file << DELETE_CMD << " " << epsname << endl;
	}
	else
	{
		if (blockCount_ > 1)
		{	
			block = 0;
			if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
				block = plotCriteria_._plot_block_number - 1;

			for (; block<blockCount_; block++)
			{
				ss.str("");
				ss << "_block" << block + 1;
				epsname = plotCriteria_._eps_file_name;
				epsname = epsname.replace(epsname.find("."), 1, ss.str() + ".");

#if defined(_WIN32) || defined(__WIN32__)
				cmd_file << "call ";
#endif
				cmd_file << "ps2pdf -dEPSCrop " << epsname <<
					" " << picname << ss.str() << ".pdf" << endl;
				cmd_file << DELETE_CMD << " " << epsname << endl;

				if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
					break;
			}

			cmd_file << "pdftk ";

			if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
				block = plotCriteria_._plot_block_number - 1;
			else
				block = 0;

			for (; block<blockCount_; block++)
			{
				ss.str("");
				ss << "_block" << block + 1;
				cmd_file << picname << ss.str() << ".pdf ";

				if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
					break;
			}

			cmd_file << "cat output " << pdfname_tmp << " dont_ask" << endl;
			cmd_file << DELETE_CMD << " ";
			
			if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
				block = plotCriteria_._plot_block_number - 1;
			else
				block = 0;

			for (; block<blockCount_; block++)
			{
				ss.str("");
				ss << "_block" << block + 1;
				cmd_file << "*" << ss.str() << ".pdf ";
				//cmd_file << "*" << ss.str() << "*.d ";

				if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
					break;
			}			
			cmd_file << endl;
		}
		else
		{
#if defined(_WIN32) || defined(__WIN32__)
			cmd_file << "call ";
#endif
			cmd_file << "ps2pdf -dEPSCrop " << plotCriteria_._eps_file_name.c_str() << " " << pdfname_tmp << endl;
			cmd_file << DELETE_CMD << " " << plotCriteria_._eps_file_name.c_str() << endl;
		}
	}
	
	// close PDF file
	dna_io_pdf pdf;
	string close_cmd(pdf.form_pdf_close_string(pdfname, projectSettings_.p._acrobat_ddename));
	if (close_cmd != "")
		cmd_file << close_cmd << endl;

	// copy temp PDF file to final PDF file
	cmd_file << COPY_CMD << " " << pdfname_tmp << " " << pdfname;
	if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
	{
		// Force output to file instead of stdout
		cmd_file << " > tmp.tmp" << endl;
		cmd_file << DELETE_CMD << " tmp.tmp" << endl;
	}
	else
		cmd_file << endl;

	// delete temp file
	cmd_file << DELETE_CMD << " " << pdfname_tmp << endl;

#if defined(_WIN32) || defined(__WIN32__)
	// open in Acrobat
	string open_cmd(pdf.form_pdf_open_string(pdfname, projectSettings_.p._acrobat_ddename));
	
	if (open_cmd != "")
		cmd_file << open_cmd << endl;
	else
		cmd_file << "start " << pdfname << endl;
#endif
	
	cmd_file << endl << "exit /b 0" << endl;

	cmd_file.close();

	return pdfname;
}
	

void dna_plot::PrintPdfCmdfile_Graph(const string& pdf_cmd_file, const string& picname)
{
	std::ofstream cmd_file;
	try {
		// open PDF batch file.  Throws runtime_error on failure.
		file_opener(cmd_file, pdf_cmd_file);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	string pdfname(picname + ".pdf");
	string pdfname_tmp(picname + ".tmp.pdf");

#if defined(_WIN32) || defined(__WIN32__)
	cmd_file << "@echo off" << endl;
	cmd_file << "call ";
#elif defined(__linux) || defined(sun) || defined(__unix__)
	cmd_file << "#!/bin/bash" << endl;
#endif
	
	cmd_file << "ps2pdf -dEPSCrop " << plotCriteria_._eps_file_name.c_str() << " " << pdfname_tmp << endl;
	cmd_file << DELETE_CMD << " " << plotCriteria_._eps_file_name.c_str() << endl;
	
	if (!plotCriteria_._keep_gen_files)
	{
		if (exists(seg_msr_graph_file_))
			cmd_file << DELETE_CMD << " " << seg_msr_graph_file_ << endl;
		if (exists(seg_stn_graph_file_))
			cmd_file << DELETE_CMD << " " << seg_stn_graph_file_ << endl;
	}

#if defined(_WIN32) || defined(__WIN32__)
	cmd_file << "call ";
#endif
	
	// close PDF file
	dna_io_pdf pdf;
	string close_cmd(pdf.form_pdf_close_string(pdfname, projectSettings_.p._acrobat_ddename));
	if (close_cmd != "")
		cmd_file << close_cmd << endl;

	// copy temp PDF file to final PDF file
	cmd_file << COPY_CMD << " " << pdfname_tmp << " " << pdfname;
	if (projectSettings_.g.verbose == 0 && projectSettings_.g.quiet == 0)
	{
		// Force output to file instead of stdout
		cmd_file << " > tmp.tmp" << endl;
		cmd_file << DELETE_CMD << " tmp.tmp" << endl;
	}
	else
		cmd_file << endl;

	// delete temp file
	cmd_file << DELETE_CMD << " " << pdfname_tmp << endl;

	#if defined(_WIN32) || defined(__WIN32__)
	// open in Acrobat
	string open_cmd(pdf.form_pdf_open_string(pdfname, projectSettings_.p._acrobat_ddename));
	if (open_cmd != "")
		cmd_file << open_cmd << endl;
	else
		cmd_file << "start " << pdfname << endl;
#endif
	
	cmd_file << endl << "exit /b 0" << endl;

	cmd_file.close();
}
	

void dna_plot::DetermineBoundingBox()
{
	if (plotCriteria_._bounding_box.empty())
		return;
	
	if (GetFields(const_cast<char*>(plotCriteria_._bounding_box.c_str()), ',', false, "ffff", 
		&upperDeg_, &leftDeg_, &lowerDeg_, &rightDeg_ ) < 4)
		return;
	
	upperDeg_ = DmstoDeg(upperDeg_);
	rightDeg_ = DmstoDeg(rightDeg_);
	lowerDeg_ = DmstoDeg(lowerDeg_);
	leftDeg_ = DmstoDeg(leftDeg_);

	plotCriteria_._plot_centre_longitude = (rightDeg_ + leftDeg_) / 2.0;
	plotCriteria_._plot_centre_latitude = (upperDeg_ + lowerDeg_) / 2.0;

	default_limits_ = false;
}
	

void dna_plot::CalculateLimitsFromStation()
{
	it_pair_string_vUINT32 _it_stnmap;
	_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), plotCriteria_._plot_station_centre, StationNameIDCompareName());

	if (_it_stnmap.first == _it_stnmap.second)
	{
		stringstream ss;
		ss.str("");
		ss << plotCriteria_._plot_station_centre << " is not in the list of network stations.";
		SignalExceptionPlot(ss.str(), 0, NULL);
	}

	plotCriteria_._plot_centre_latitude = bstBinaryRecords_.at(_it_stnmap.first->second).initialLatitude;
	plotCriteria_._plot_centre_longitude = bstBinaryRecords_.at(_it_stnmap.first->second).initialLongitude;
	
	CalculateLimitsFromPoint();
}
	

void dna_plot::CalculateLimitsFromPoint()
{	
	double temp;
	
	CDnaEllipsoid e(DEFAULT_EPSG_U);

	VincentyDirect<double>(Radians<double>(plotCriteria_._plot_centre_latitude), Radians<double>(plotCriteria_._plot_centre_longitude),
		Radians<double>(0.), plotCriteria_._plot_area_radius, &upperDeg_, &temp, &e);
	upperDeg_ = Degrees(upperDeg_);
	
	VincentyDirect<double>(Radians<double>(plotCriteria_._plot_centre_latitude), Radians<double>(plotCriteria_._plot_centre_longitude),
		Radians<double>(90.), plotCriteria_._plot_area_radius, &temp, &rightDeg_, &e);
	rightDeg_ = Degrees(rightDeg_);
	
	VincentyDirect<double>(Radians<double>(plotCriteria_._plot_centre_latitude), Radians<double>(plotCriteria_._plot_centre_longitude), 
		Radians<double>(180.), plotCriteria_._plot_area_radius, &lowerDeg_, &temp, &e);
	lowerDeg_ = Degrees(lowerDeg_);
	
	VincentyDirect<double>(Radians<double>(plotCriteria_._plot_centre_latitude), Radians<double>(plotCriteria_._plot_centre_longitude),
		Radians<double>(270.), plotCriteria_._plot_area_radius, &temp, &leftDeg_, &e);
	leftDeg_ = Degrees(leftDeg_);
}
	

void dna_plot::FormGMTDataFileNames(const UINT32& block)
{
	string firstPartISL(network_name_), firstPartSTN(network_name_);

	if (plotBlocks_)
	{
		stringstream firstPart(network_name_);
		string firstPartJSL;
		
		// add isl and jsl filenames for this block
		firstPart << "_block" << block + 1;
		firstPartISL = firstPart.str() + "_isl_";
		firstPartJSL = firstPart.str() + "_jsl_";
		firstPartSTN = firstPart.str() + "_stn_";

		v_jsl_pts_file_.push_back(firstPartJSL + ".stn.d");
		v_jsl_const_file_.push_back(firstPartJSL + ".const.d");

		if (plotCriteria_._plot_station_labels)
			v_jsl_lbl_file_.push_back(firstPartJSL + ".stn.lbl");
	}

	v_isl_pts_file_.push_back(firstPartISL + ".stn.d");
	v_isl_const_file_.push_back(firstPartISL + ".const.d");
	
	if (plotCriteria_._plot_station_labels)
		v_isl_lbl_file_.push_back(firstPartISL + ".stn.lbl");

	if (plotCriteria_._plot_positional_uncertainty)
		v_stn_apu_file_.push_back(firstPartSTN + ".apu.d");
	
	if (plotCriteria_._plot_error_ellipses)
		v_stn_err_file_.push_back(firstPartSTN + ".err.d");
	
	if (plotCriteria_._plot_correction_arrows)
		v_stn_cor_file_.push_back(firstPartSTN + ".cor.d");
}

void dna_plot::PrintStationsDataFileBlock(const UINT32& block)
{
	// Form file names for this block
	FormGMTDataFileNames(block);
	
	std::ofstream isl_pts, isl_const, jsl_pts, jsl_const;

	UINT32 block_index(block);
	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block_index = 0;	// one and only block

	try {
		// Create stations data files for ISL/JSL.  Throws runtime_error on failure.
		file_opener(isl_pts, formPath<string>(output_folder_, v_isl_pts_file_.at(block_index)));
		file_opener(jsl_pts, formPath<string>(output_folder_, v_jsl_pts_file_.at(block_index)));
		// Create constraint stations data files for ISL/JSL.  Throws runtime_error on failure.
		file_opener(isl_const, formPath<string>(output_folder_, v_isl_const_file_.at(block_index)));
		file_opener(jsl_const, formPath<string>(output_folder_, v_jsl_const_file_.at(block_index)));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	// print isl coordinates
	it_vstn_t_const _it_stn;
	for (it_vUINT32 _it_isl=v_ISL_.at(block).begin();
		_it_isl<v_ISL_.at(block).end(); 
		++_it_isl)
	{
		// Advance iterator
		_it_stn = bstBinaryRecords_.begin() + *_it_isl;

		// Constrained?
		if (_it_stn->stationConst[0] == 'C' || 
			_it_stn->stationConst[1] == 'C' || 
			_it_stn->stationConst[2] == 'C')
		{
			// Constrained
			plotConstraints_ = true;
			PrintStationDataFile(isl_const, _it_stn);
		}
		else
			// Free
			PrintStationDataFile(isl_pts, _it_stn);
	}

	isl_pts.close();
	isl_const.close();

	// print jsl coordinates
	it_vUINT32 _it_jsl(v_JSL_.at(block).begin());
	for (it_vUINT32 _it_jsl=v_JSL_.at(block).begin();
		_it_jsl<v_JSL_.at(block).end(); 
		++_it_jsl)
	{
		// Advance iterator
		_it_stn = bstBinaryRecords_.begin() + *_it_jsl;

		// Constrained?
		if (_it_stn->stationConst[0] == 'C' || 
			_it_stn->stationConst[1] == 'C' || 
			_it_stn->stationConst[2] == 'C')
		{
			// Constrained
			plotConstraints_ = true;
			PrintStationDataFile(jsl_const, _it_stn);
		}
		else
			// Free
			PrintStationDataFile(jsl_pts, _it_stn);
	}

	jsl_pts.close();
	jsl_const.close();
}
	
void dna_plot::PrintStationsDataFile()
{
	// Form file names
	FormGMTDataFileNames();
	
	std::ofstream stn_pts, stn_const;
	try {
		// Create stations data file.  Throws runtime_error on failure.
		file_opener(stn_pts, formPath<string>(output_folder_, v_isl_pts_file_.at(0)));
		// Create constraint stations data file.  Throws runtime_error on failure.
		file_opener(stn_const, formPath<string>(output_folder_, v_isl_const_file_.at(0)));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	stringstream ss;

	for (it_vstn_t_const _it_stn(bstBinaryRecords_.begin());
		_it_stn<bstBinaryRecords_.end(); 
		++_it_stn)
	{
		// Unused Station - don't print.
		if (_it_stn->unusedStation == 1)
			continue;

		// Constrained?
		if (_it_stn->stationConst[0] == 'C' || 
			_it_stn->stationConst[1] == 'C' || 
			_it_stn->stationConst[2] == 'C')
		{
			// Constrained
			plotConstraints_ = true;
			PrintStationDataFile(stn_const, _it_stn);
		}
		else
			// Free
			PrintStationDataFile(stn_pts, _it_stn);
	}

	stn_pts.close();
	stn_const.close();

	if (!default_limits_)
	{
		if (!plotCriteria_._bounding_box.empty())
			DetermineBoundingBox();
		else if (plotCriteria_._plot_station_centre.empty())
			CalculateLimitsFromPoint();
		else
			CalculateLimitsFromStation();
	}
}
	

void dna_plot::PrintStationDataFile(ostream& os, it_vstn_t_const _it_stn)
{
	if (default_limits_) 
	{
		if (_it_stn->initialLatitude < lowerDeg_) 
			lowerDeg_ = _it_stn->initialLatitude;
		if (_it_stn->initialLatitude > upperDeg_)
			upperDeg_ = _it_stn->initialLatitude;
		if (_it_stn->initialLongitude < leftDeg_) 
			leftDeg_ = _it_stn->initialLongitude;
		if (_it_stn->initialLongitude > rightDeg_) 
			rightDeg_ = _it_stn->initialLongitude;
	}

	// print longitude and latitude
	os << setprecision(10) << fixed << _it_stn->initialLongitude << "  " <<
		_it_stn->initialLatitude << endl;
}

void dna_plot::PrintStationLabel(ostream& os, it_vstn_t_const _it_stn)
{
	if (_it_stn->unusedStation)
		return;

	// Print longitude, latitude and label
	os << setprecision(10) << fixed << _it_stn->initialLongitude << "  " <<
		_it_stn->initialLatitude;
	os << "  " << plotCriteria_._label_font_size <<		// font size
		" 0 0 LM ";										// font angle, font number, justification

	// Print the label, default is station name (i.e. 200100350)
	string label(_it_stn->stationName);
	
	// plot alternate name?
	if (plotCriteria_._plot_alt_name)
	{
		label = _it_stn->description;		// description (i.e. Acheron PM 35)
		if (trimstr(label).empty())
			label = "_";
	}
	os << label;			

	// Plot constraints?
	if (plotCriteria_._plot_station_constraints)
		if (_it_stn->stationConst[0] == 'C' || 
			_it_stn->stationConst[1] == 'C' || 
			_it_stn->stationConst[2] == 'C')
				os << " (" << _it_stn->stationConst << ")";
	os << endl;
}

void dna_plot::PrintStationLabels()
{
	std::ofstream stn_lbl;
	try {
		// Create stations data file.  Throws runtime_error on failure.
		file_opener(stn_lbl, formPath<string>(output_folder_, v_isl_lbl_file_.at(0)));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	// initialise a station list and sort on longitude
	vUINT32 stnList(bstBinaryRecords_.size());
	it_vUINT32 _it_stn(stnList.begin());

	// initialise vector with 0,1,2,...,n-2,n-1,n
	initialiseIncrementingIntegerVector<UINT32>(stnList, static_cast<UINT32>(bstBinaryRecords_.size()));

	// sort stations on longitude so labels to the left are placed last
	CompareStnLongitude<station_t, UINT32> stnorderCompareFunc(&bstBinaryRecords_, false);
	sort(stnList.begin(), stnList.end(), stnorderCompareFunc);

	it_vstn_t _it_bstn;

	for (_it_stn=stnList.begin(); _it_stn<stnList.end(); _it_stn++)
	{
		_it_bstn = bstBinaryRecords_.begin() + *_it_stn;
		
		// print the label
		PrintStationLabel(stn_lbl, _it_bstn);
	}

	stn_lbl.close();
}
	

void dna_plot::PrintStationLabelsBlock(const UINT32& block)
{	
	UINT32 block_index(block);
	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block_index = 0;
	
	std::ofstream isl_lbl;
	try {
		// Create stations data file.  Throws runtime_error on failure.
		file_opener(isl_lbl, formPath<string>(output_folder_, v_isl_lbl_file_.at(block_index)));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	// sort ISLs on longitude so labels to the left are placed last
	CompareStnLongitude<station_t, UINT32> stnorderCompareFunc(&bstBinaryRecords_, false);
	sort(v_ISL_.at(block).begin(), v_ISL_.at(block).end(), stnorderCompareFunc);

	it_vstn_t _it_bstn;

	for (it_vUINT32 _it_isl(v_ISL_.at(block).begin());
		_it_isl<v_ISL_.at(block).end(); 
		++_it_isl)
	{
		_it_bstn = bstBinaryRecords_.begin() + *_it_isl;
		
		// print the label
		PrintStationLabel(isl_lbl, _it_bstn);
	}

	isl_lbl.close();
	
	// return to former sort order
	sort(v_ISL_.at(block).begin(), v_ISL_.at(block).end());


	std::ofstream jsl_lbl;
	try {
		// Create stations data file.  Throws runtime_error on failure.
		file_opener(jsl_lbl, formPath<string>(output_folder_, v_jsl_lbl_file_.at(block_index)));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	// sort JSLs on longitude so labels to the left are placed last
	sort(v_JSL_.at(block).begin(), v_JSL_.at(block).end(), stnorderCompareFunc);

	for (it_vUINT32 _it_jsl(v_JSL_.at(block).begin());
		_it_jsl<v_JSL_.at(block).end(); 
		++_it_jsl)
	{
		_it_bstn = bstBinaryRecords_.begin() + *_it_jsl;
		
		// print the label
		PrintStationLabel(jsl_lbl, _it_bstn);	
	}
	
	jsl_lbl.close();

	// return to former sort order
	sort(v_JSL_.at(block).begin(), v_JSL_.at(block).end());
}
	

void dna_plot::PrintPositionalUncertainty(const UINT32& block)
{
	UINT32 block_index(block);
	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block_index = 0;
	
	std::ofstream stn_apu;
	try {
		// Create apu file.  Throws runtime_error on failure.
		file_opener(stn_apu, v_stn_apu_file_.at(block_index));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	it_pair_string_vUINT32 _it_stnmap;
	it_vstn_t_const _it_stn(bstBinaryRecords_.begin());
	UINT32 precision(10);

	it_vstnPU_t _it_pu;

	// Print uncertainty legend, using average xx length.  See LoadPosUncertaintyFile().
	stn_apu << 
		setw(MSR) << setprecision(precision) << fixed << left << uncertainty_legend_long_ << "  " <<
		setw(MSR) << setprecision(precision) << fixed << left << uncertainty_legend_lat_ << "  " << 
		" 0 0 " <<
		setw(MSR) << setprecision(4) << scientific << right << largest_uncertainty_  << 		// semi-major
		setw(MSR) << setprecision(4) << scientific << right << largest_uncertainty_ <<			// semi-minor
		setw(MSR) << setprecision(2) << fixed << right << "45.0" << endl;						// orientation from e-axis
	
	for (_it_pu=v_stn_pu_.begin();
		_it_pu!=v_stn_pu_.end();
		++_it_pu)
	{
		_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), _it_pu->_station, StationNameIDCompareName());

		// Not in the list? (Unlikely since adjust output will not contain stations not in the map, but test anyway)
		if (_it_stnmap.first == _it_stnmap.second)
			continue;
		
		// no need to plot constrained stations - the uncertainties will be zero!!!	
		// Horizontal only!!!
		if (bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[0] == 'C' &&
			bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[1] == 'C')
			continue;

		if (plotBlocks_)
			// If this station is not in this block, don't print
			if (!binary_search(v_parameterStationList_.at(block).begin(), v_parameterStationList_.at(block).end(),
				_it_stnmap.first->second))
				continue;

		stn_apu << 
			setw(MSR) << setprecision(precision) << fixed << left << _it_pu->_longitude << "  " <<
			setw(MSR) << setprecision(precision) << fixed << left << _it_pu->_latitude << "  " << 
			" 0 0 " <<
			setw(MSR) << setprecision(4) << scientific << right << _it_pu->_hzPosU * plotCriteria_._pu_ellipse_scale <<
			setw(MSR) << setprecision(4) << scientific << right << _it_pu->_hzPosU * plotCriteria_._pu_ellipse_scale <<
			setw(MSR) << setprecision(1) << fixed << right << "0." << endl;	
	}
	
	stn_apu.close();
}
	

void dna_plot::PrintErrorEllipses(const UINT32& block)
{	
	UINT32 block_index(block);
	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block_index = 0;
	
	std::ofstream stn_err;
	try {
		// Create error ellipse file.  Throws runtime_error on failure.
		file_opener(stn_err, v_stn_err_file_.at(block_index));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	it_pair_string_vUINT32 _it_stnmap;
	it_vstn_t_const _it_stn(bstBinaryRecords_.begin());
	UINT32 precision(10);

	it_vstnPU_t _it_pu;

	double ellipse_scale(1.);

	// The error ellipse legend is printed using the largest uncertainty (semi-major) found, unless
	// positional uncertainty is being printed, in which case the error ellipse is printed using
	// half the size.  This is purely to give the impression that error ellipses are 1 sigma (68%) and
	// positional uncertainty is 95%, or close to 2 sigma.
	if (plotCriteria_._plot_positional_uncertainty)
		ellipse_scale = 2.;

	// Print uncertainty legend, using average xx length.  See LoadPosUncertaintyFile().
	stn_err << 
		setw(MSR) << setprecision(precision) << fixed << left << uncertainty_legend_long_ << "  " <<
		setw(MSR) << setprecision(precision) << fixed << left << uncertainty_legend_lat_ << "  " << 
		" 0 0 " <<
		setw(MSR) << setprecision(4) << scientific << right << 
		largest_uncertainty_ / ellipse_scale << 					// semi-major
		setw(MSR) << setprecision(4) << scientific << right << 
		largest_uncertainty_ / 3.0 / ellipse_scale <<				// semi-minor
		setw(MSR) << setprecision(2) << fixed << right <<
		"45.0" << endl;												// orientation from e-axis
	
	double angle;

	for (_it_pu=v_stn_pu_.begin();
		_it_pu!=v_stn_pu_.end();
		++_it_pu)
	{
		_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), _it_pu->_station, StationNameIDCompareName());

		// Not in the list? (Unlikely since adjust output will not contain stations not in the map, but test anyway)
		if (_it_stnmap.first == _it_stnmap.second)
			continue;
		
		// no need to plot constrained stations - the uncertainties will be zero!!!	
		// Horizontal only!!!
		if (bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[0] == 'C' &&
			bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[1] == 'C')
			continue;

		if (plotBlocks_)
			// If this station is not in this block, don't print
			if (!binary_search(v_parameterStationList_.at(block).begin(), v_parameterStationList_.at(block).end(),
				_it_stnmap.first->second))
				continue;

		// reduce error ellipse orientation in terms of
		// positive anti-clockwise direction from e-axis
		if (_it_pu->_orientation > 90.)
			angle = -(_it_pu->_orientation - 90.);
		else
			angle = DegtoDms(90. - DmstoDeg(_it_pu->_orientation));

		stn_err << 
			setw(MSR) << setprecision(precision) << fixed << left << _it_pu->_longitude << "  " <<
			setw(MSR) << setprecision(precision) << fixed << left << _it_pu->_latitude << "  " << 
			" 0 0 " <<
			setw(MSR) << setprecision(4) << scientific << right << _it_pu->_semimMajor * plotCriteria_._pu_ellipse_scale <<
			setw(MSR) << setprecision(4) << scientific << right << _it_pu->_semimMinor * plotCriteria_._pu_ellipse_scale <<
			setw(MSR) << setprecision(4) << fixed << right << angle << endl;	
	}
	
	stn_err.close();
}
	

void dna_plot::PrintCorrectionArrows(const UINT32& block)
{
	UINT32 block_index(block);
	if (plotBlocks_ && plotCriteria_._plot_block_number > 0)
		block_index = 0;
	
	std::ofstream stn_cor;
	try {
		// Create corrections file.  Throws runtime_error on failure.
		file_opener(stn_cor, v_stn_cor_file_.at(block_index));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	it_pair_string_vUINT32 _it_stnmap;
	it_vstn_t_const _it_stn(bstBinaryRecords_.begin());
	UINT32 precision(10);

	it_vstnCor_t _it_cor;

	// Print correction legend, using average correction length.  See LoadCorrectionsFile().
	stn_cor << 
		setw(MSR) << setprecision(precision) << fixed << left << arrow_legend_long_ << "  " <<
		setw(MSR) << setprecision(precision) << fixed << left << arrow_legend_lat_ << "  " << 
		setw(MSR) << setprecision(4) << scientific << right << average_correction_ <<
		setw(MSR) << setprecision(2) << fixed << right << 0.0 <<
		" 0 0 0 " << endl;
	
	for (_it_cor=v_stn_corrs_.begin();
		_it_cor!=v_stn_corrs_.end();
		++_it_cor)
	{
		// Find the id of this station
		_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), _it_cor->_station, StationNameIDCompareName());

		// Not in the list? (Unlikely since adjust will not output stations that are not 
		// in the map, but test anyway)
		if (_it_stnmap.first == _it_stnmap.second)
			continue;
		
		// no need to plot constrained stations - these won't have moved!	
		// Horizontal only!!!
		if (bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[0] == 'C' &&
			bstBinaryRecords_.at(_it_stnmap.first->second).stationConst[1] == 'C')
			continue;

		if (plotBlocks_)
			// If this station is not in this block, don't print
			if (!binary_search(v_parameterStationList_.at(block).begin(), v_parameterStationList_.at(block).end(),
				_it_stnmap.first->second))
				continue;

		stn_cor << 
			setw(MSR) << setprecision(precision) << fixed << left << bstBinaryRecords_.at(_it_stnmap.first->second).initialLongitude << "  " <<
			setw(MSR) << setprecision(precision) << fixed << left << bstBinaryRecords_.at(_it_stnmap.first->second).initialLatitude << "  " << 
			setw(MSR) << setprecision(4) << scientific << right << _it_cor->_east * plotCriteria_._correction_scale <<
			setw(MSR) << setprecision(4) << scientific << right << _it_cor->_north * plotCriteria_._correction_scale <<
			" 0 0 0 ";

		if (plotCriteria_._plot_correction_labels)
			stn_cor << 
				setw(HEIGHT) << setprecision(3) << fixed << right << _it_cor->_east << "e," <<
				setprecision(3) << fixed << _it_cor->_north << "n   ";
		stn_cor << endl;	
	}
	
	stn_cor.close();
}
	

// Plots the specified measurement types to individual files
// All other measurement types (_combined_msr_list) are printed to a single file
// This enables colours to be specified for individual measurement types
void dna_plot::PrintMeasurementsDatFilesBlock(const UINT32& block)
{
	v_msr_file_.at(block).clear();

	string type;
	stringstream ss, filename;

	std::ofstream msr_file_stream;
	
	for (UINT16 c=0; c<plotCriteria_._separate_msrs.size(); c++)
	{
		// no measurement for this type?
		if (parsemsrTally_.MeasurementCount(plotCriteria_._separate_msrs.at(c)) == 0)
			continue;

		ss.str("");
		ss << plotCriteria_._separate_msrs.at(c);
		type = ss.str();
		filename.str("");
		filename << network_name_ << "_msr_block" << block + 1 << "_" << plotCriteria_._separate_msrs.at(c) << ".d";
		v_msr_def_file_.push_back(filename.str());
		v_msr_file_.at(block).push_back(string_string_pair(v_msr_def_file_.back(), type));

		try {
			// Create msr data file.  Throws runtime_error on failure.
			file_opener(msr_file_stream, formPath<string>(output_folder_, v_msr_def_file_.back()));
		}
		catch (const runtime_error& e) {
			SignalExceptionPlot(e.what(), 0, NULL);
		}

		PrintMeasurementsDatFileBlock(block, v_msr_def_file_.back(), plotCriteria_._separate_msrs.at(c), &msr_file_stream);

		msr_file_stream.close();

		// remove this measurement type from the global list
		it_vchar_range = equal_range(_combined_msr_list.begin(), _combined_msr_list.end(), 
			plotCriteria_._separate_msrs.at(c));

		if (it_vchar_range.first != it_vchar_range.second)
		{
			// found - remove it from the list.  The remaining elements will be printed to a single file
			_combined_msr_list.erase(it_vchar_range.first);
		}
	}

	vector<char>::const_iterator _it_type;
	bool printOtherTypes(false);
	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); ++_it_type)
	{
		if (parsemsrTally_.MeasurementCount(*_it_type) > 0)
		{
			printOtherTypes = true;
			break;
		}
	}

	// don't bother printing this file if there are no 'other' measurements
	if (!printOtherTypes)
		return;

	// now print remaining measurement types to a single file
	filename.str("");
	filename << network_name_ << "_msr_block" << block + 1 << "_";
	ss.str("");
	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); _it_type++)
		ss << *_it_type;
	filename << ss.str() << ".d";
	v_msr_all_file_.push_back(filename.str());
	v_msr_file_.at(block).insert(v_msr_file_.at(block).begin(), string_string_pair(v_msr_all_file_.back(), ss.str()));

	try {
		// Create msr data file.  Throws runtime_error on failure.
		file_opener(msr_file_stream, formPath<string>(output_folder_, v_msr_all_file_.back()));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); _it_type++)
		PrintMeasurementsDatFileBlock(block, v_msr_all_file_.back(), *_it_type, &msr_file_stream);

	msr_file_stream.close();

}


// Plots the specified measurement types to individual files
// All other measurement types (_combined_msr_list) are printed to a single file
// This enables colours to be specified for individual measurement types
void dna_plot::PrintMeasurementsDatFiles()
{
	_combined_msr_list.clear();
	_combined_msr_list.push_back('A'); // Horizontal angle
	_combined_msr_list.push_back('B'); // Geodetic azimuth
	_combined_msr_list.push_back('C'); // Chord dist
	_combined_msr_list.push_back('D'); // Direction set
	_combined_msr_list.push_back('E'); // Ellipsoid arc
	_combined_msr_list.push_back('G'); // GPS Baseline (treat as single-baseline cluster)
	_combined_msr_list.push_back('H'); // Orthometric height
	_combined_msr_list.push_back('I'); // Astronomic latitude
	_combined_msr_list.push_back('J'); // Astronomic longitude
	_combined_msr_list.push_back('K'); // Astronomic azimuth
	_combined_msr_list.push_back('L'); // Level difference
	_combined_msr_list.push_back('M'); // MSL arc
	_combined_msr_list.push_back('P'); // Geodetic latitude
	_combined_msr_list.push_back('Q'); // Geodetic longitude
	_combined_msr_list.push_back('R'); // Ellipsoidal height
	_combined_msr_list.push_back('S'); // Slope distance
	_combined_msr_list.push_back('V'); // Zenith angle
	_combined_msr_list.push_back('X'); // GPS Baseline cluster
	_combined_msr_list.push_back('Y'); // GPS point cluster
	_combined_msr_list.push_back('Z'); // Vertical angle

	//vector<char>::iterator _it_type(_combined_msr_list.end());
	
	string msr_file_name;
	v_msr_file_.at(0).clear();

	YClusterStations_.clear();

	string type;
	stringstream ss;

	std::ofstream msr_file_stream;
	
	for (UINT16 c=0; c<plotCriteria_._separate_msrs.size(); c++)
	{
		// no measurement for this type?
		if (parsemsrTally_.MeasurementCount(plotCriteria_._separate_msrs.at(c)) == 0)
			continue;

		ss.str("");
		ss << plotCriteria_._separate_msrs.at(c);
		type = ss.str();
		msr_file_name = network_name_ + "_msr_" + plotCriteria_._separate_msrs.at(c) + ".d";
		v_msr_file_.at(0).push_back(string_string_pair(msr_file_name, type));

		try {
			// Create msr data file.  Throws runtime_error on failure.
			file_opener(msr_file_stream, formPath<string>(output_folder_, msr_file_name));
		}
		catch (const runtime_error& e) {
			SignalExceptionPlot(e.what(), 0, NULL);
		}

		PrintMeasurementsDatFile(msr_file_name, plotCriteria_._separate_msrs.at(c), &msr_file_stream);

		msr_file_stream.close();

		// remove this measurement type from the global list
		it_vchar_range = equal_range(_combined_msr_list.begin(), _combined_msr_list.end(), 
			plotCriteria_._separate_msrs.at(c));

		if (it_vchar_range.first != it_vchar_range.second)
		{
			// found - remove it from the list.  The remaining elements will be printed to a single file
			_combined_msr_list.erase(it_vchar_range.first);
		}
	}

	vector<char>::const_iterator _it_type;
	bool printOtherTypes(false);
	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); ++_it_type)
	{
		if (parsemsrTally_.MeasurementCount(*_it_type) > 0)
		{
			printOtherTypes = true;
			break;
		}
	}

	// don't bother printing this file if there are no 'other' measurements
	if (!printOtherTypes)
		return;

	// now print remaining measurement types to a single file
	msr_file_name = network_name_ + "_msr_";
	ss.str("");
	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); ++_it_type)
		ss << *_it_type;
	type = ss.str();
	msr_file_name.append(type).append(".d");
	v_msr_file_.at(0).insert(v_msr_file_.at(0).begin(), string_string_pair(msr_file_name, type));

	try {
		// Create msr data file.  Throws runtime_error on failure.
		file_opener(msr_file_stream, formPath<string>(output_folder_, msr_file_name));
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	for (_it_type=_combined_msr_list.begin(); _it_type!=_combined_msr_list.end(); _it_type++)
		PrintMeasurementsDatFile(msr_file_name, *_it_type, &msr_file_stream);

	msr_file_stream.close();

}


bool dna_plot::WithinLimits(const double& latitude, const double& longitude)
{
	if (latitude >= lowerDeg_ && latitude <= upperDeg_ &&
		longitude >= leftDeg_ && longitude <= rightDeg_)
		return true;
	else
		return false;
}
	

void dna_plot::PrintMeasurementsDatFileBlock(const UINT32& block, const string& msr_file_name, char msrType, std::ofstream* msr_file_stream)
{
	it_vstn_t _it_stn(bstBinaryRecords_.begin());
	
	UINT32 precision(10);
	bool FirstisWithinLimits, SecondisWithinLimits, ThirdisWithinLimits;

	stringstream ss;
	
	it_vUINT32 _it_block_msr(v_CML_.at(block).begin());
	vUINT32 msrIndices;
	it_vmsr_t _it_msr;

	for (_it_block_msr=v_CML_.at(block).begin(); _it_block_msr<v_CML_.at(block).end(); _it_block_msr++)
	{
		_it_msr = bmsBinaryRecords_.begin() + (*_it_block_msr);

		if (msrType != _it_msr->measType)
			continue;

		if (_it_msr->ignore && !plotCriteria_._plot_ignored_msrs)
			continue;

		ss.str("");
		ss.clear();
		FirstisWithinLimits = SecondisWithinLimits = ThirdisWithinLimits = false;
		switch (msrType)
		{
		case 'A':	// Horizontal angles
			// Print 3 - 1 - 2
			//
			// Third
			ss << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station3).initialLongitude;
			ss << "  ";
			ss << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station3).initialLatitude;
			ss << endl;

			// test if point is within limits of custom defined box
			if (!default_limits_)
				ThirdisWithinLimits = WithinLimits(bstBinaryRecords_.at(_it_msr->station3).initialLatitude, bstBinaryRecords_.at(_it_msr->station3).initialLongitude);

		case 'B': // Geodetic azimuth
		case 'D': // Direction set
		case 'G': // GPS Baseline
		case 'K': // Astronomic azimuth
		case 'C': // Chord dist
		case 'E': // Ellipsoid arc
		case 'M': // MSL arc
		case 'S': // Slope distance
		case 'L': // Level difference
		case 'V': // Zenith angle
		case 'X': // GPS Baseline cluster
		case 'Z': // Vertical angle
			// Get the measurement indices involved in this measurement
			GetMsrIndices<UINT32>(bmsBinaryRecords_, *_it_block_msr, msrIndices);

			for (it_vUINT32 msr=msrIndices.begin(); msr!=msrIndices.end(); ++msr)
			{
				if (bmsBinaryRecords_.at(*msr).measStart > zMeas)
					continue;

				if (!default_limits_)
				{
					FirstisWithinLimits = WithinLimits(
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLatitude, 
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLongitude);
					SecondisWithinLimits = WithinLimits(
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station2).initialLatitude, 
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station2).initialLongitude);

					if (!FirstisWithinLimits && !SecondisWithinLimits && !ThirdisWithinLimits)
						continue;
				}

				// Third.  If case 'A' was reached before case 'B', ss will be non-empty
				(*msr_file_stream) << ss.str();
			
				// First
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLongitude;
				(*msr_file_stream) << "  ";
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLatitude;
				(*msr_file_stream) << endl;
			
				// Second
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station2).initialLongitude;
				(*msr_file_stream) << "  ";
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station2).initialLatitude;
				(*msr_file_stream) << endl << "#" << endl;
			}
			break;
		
		// single station
		case 'H': // Orthometric height
		case 'I': // Astronomic latitude
		case 'J': // Astronomic longitude
		case 'P': // Geodetic latitude
		case 'Q': // Geodetic longitude
		case 'R': // Ellipsoidal height
			if (_it_msr->measStart > zMeas)
				continue;

			if (!default_limits_)
				if (!WithinLimits(
					bstBinaryRecords_.at(_it_msr->station1).initialLatitude, 
					bstBinaryRecords_.at(_it_msr->station1).initialLongitude))
					continue;

			// First
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLongitude;
			(*msr_file_stream) << "  ";
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLatitude;
			(*msr_file_stream) << endl;
			
			break;

		case 'Y': // GPS point cluster
			
			// Get the measurement indices involved in this measurement
			GetMsrIndices<UINT32>(bmsBinaryRecords_, *_it_block_msr, msrIndices);

			for (it_vUINT32 msr=msrIndices.begin(); msr!=msrIndices.end(); ++msr)
			{
				if (bmsBinaryRecords_.at(*msr).measStart > zMeas)
					continue;

				if (!default_limits_)
					if (!WithinLimits(
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLatitude, 
						bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLongitude))
						continue;

				// First
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLongitude;
				(*msr_file_stream) << "  ";
				(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(bmsBinaryRecords_.at(*msr).station1).initialLatitude;
				(*msr_file_stream) << endl << "#" << endl;
			}
		}
	}

}
	

void dna_plot::PrintMeasurementsDatFile(const string& msr_file_name, char msrType, std::ofstream* msr_file_stream)
{
	it_vstn_t _it_stn(bstBinaryRecords_.begin());
	it_vmsr_t _it_msr(bmsBinaryRecords_.begin());
	
	UINT32 precision(10);
	bool FirstisWithinLimits, SecondisWithinLimits, ThirdisWithinLimits;

	stringstream ss;
	
	for (_it_msr=bmsBinaryRecords_.begin(); _it_msr!=bmsBinaryRecords_.end(); _it_msr++)
	{
		if (msrType != _it_msr->measType)
			continue;
		
		if (_it_msr->ignore && !plotCriteria_._plot_ignored_msrs)
			continue;
		
		ss.str("");
		ss.clear();
		FirstisWithinLimits = SecondisWithinLimits = ThirdisWithinLimits = false;
		switch (msrType)
		{
		case 'A':	// Horizontal angles
			// Print 3 - 1 - 2
			//
			// Third
			ss << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station3).initialLongitude;
			ss << "  ";
			ss << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station3).initialLatitude;
			ss << endl;

			// test if point is within limits of custom defined box
			if (!default_limits_)
				ThirdisWithinLimits = WithinLimits(bstBinaryRecords_.at(_it_msr->station3).initialLatitude, bstBinaryRecords_.at(_it_msr->station3).initialLongitude);

		case 'G': // GPS Baseline
		case 'X': // GPS Baseline cluster
		
			// The following prevents GPS measurements from printing three times (i.e. once per X, Y, Z)
			if (_it_msr->measStart > xMeas)
				continue;

		case 'B': // Geodetic azimuth
		case 'D': // Direction set
		case 'K': // Astronomic azimuth
		case 'C': // Chord dist
		case 'E': // Ellipsoid arc
		case 'M': // MSL arc
		case 'S': // Slope distance
		case 'L': // Level difference
		case 'V': // Zenith angle
		case 'Z': // Vertical angle

			if (!default_limits_)
			{
				FirstisWithinLimits = WithinLimits(bstBinaryRecords_.at(_it_msr->station1).initialLatitude, bstBinaryRecords_.at(_it_msr->station1).initialLongitude);
				SecondisWithinLimits = WithinLimits(bstBinaryRecords_.at(_it_msr->station2).initialLatitude, bstBinaryRecords_.at(_it_msr->station2).initialLongitude);

				if (!FirstisWithinLimits && !SecondisWithinLimits && !ThirdisWithinLimits)
					continue;
			}

			// Third 
			// If this measurement is an angle, then ss will contain data for third station
			// Otherwise, ss will be blank
			(*msr_file_stream) << ss.str();
			
			// First
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLongitude;
			(*msr_file_stream) << "  ";
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLatitude;
			(*msr_file_stream) << endl;
			
			// Second
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station2).initialLongitude;
			(*msr_file_stream) << "  ";
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station2).initialLatitude;
			(*msr_file_stream) << endl << "#" << endl;
			break;
		
		// single station
		case 'H': // Orthometric height
		case 'I': // Astronomic latitude
		case 'J': // Astronomic longitude
		case 'P': // Geodetic latitude
		case 'Q': // Geodetic longitude
		case 'R': // Ellipsoidal height
			
			if (!default_limits_)
			{
				FirstisWithinLimits = WithinLimits(bstBinaryRecords_.at(_it_msr->station1).initialLatitude, bstBinaryRecords_.at(_it_msr->station1).initialLongitude);
				if (!FirstisWithinLimits)
					continue;
			}

			// First
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLongitude;
			(*msr_file_stream) << "  ";
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLatitude;
			(*msr_file_stream) << endl;
			
			break;

		case 'Y': // GPS point cluster
			
			// The following prevents GPS measurements from printing three times (i.e. once per X, Y, Z)
			if (_it_msr->measStart > xMeas)
				continue;

			//// capture first station in the cluster
			//if (_it_tmp == bmsBinaryRecords_.end())
			//{
			//	_it_tmp = _it_msr;
			//	continue;
			//}

			//// Is this a new measurement cluster?
			//if (_it_tmp->clusterID != _it_msr->clusterID)
			//{
			//	_it_tmp = _it_msr;
			//	continue;
			//}

			if (binary_search(YClusterStations_.begin(), YClusterStations_.end(), _it_msr->station1))
				// already have this one
				continue;
			
			YClusterStations_.push_back(_it_msr->station1);
			sort(YClusterStations_.begin(), YClusterStations_.end());

			// First
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLongitude;
			(*msr_file_stream) << "  ";
			(*msr_file_stream) << setprecision(precision) << fixed << bstBinaryRecords_.at(_it_msr->station1).initialLatitude;
			(*msr_file_stream) << endl << "#" << endl;

		}
	}

}
	

void dna_plot::SetGMTParameters()
{
	it_string_pair _it_vpstr(plotCriteria_._gmt_params.begin());

	try {
		for (; _it_vpstr!=plotCriteria_._gmt_params.end(); _it_vpstr++)
			gmtbat_file_ << "gmtset " << _it_vpstr->first << " " << _it_vpstr->second << endl;
		gmtbat_file_ << endl;
	}
	catch (const ios_base::failure& f) {
		SignalExceptionPlot(static_cast<string>(f.what()), 0, "o", &gmtbat_file_);	
	}
	
}
	

void dna_plot::LoadNetworkFiles(const project_settings& projectSettings)
{
	projectSettings_ = projectSettings;

	_combined_msr_list.clear();
	MsrTally::FillMsrList(_combined_msr_list);

	blockCount_ = 1;
	plotBlocks_ = projectSettings.p._plot_phased_blocks;
	LoadBinaryFiles();
	LoadStationMap();
	if (plotBlocks_)
		LoadSegmentationFile();

	// Printing uncertainties?
	if (projectSettings.p._plot_error_ellipses || 
		projectSettings.p._plot_positional_uncertainty)
	{
		uncertainty_legend_length_ = 0.5;
		LoadPosUncertaintyFile();
	}
	
	// Printing corrections?
	if (projectSettings.p._plot_correction_arrows)
	{
		arrow_legend_length_ = 3.;
		if (projectSettings.p._compute_corrections)
			ComputeStationCorrections();
		else
			LoadCorrectionsFile();
	}

	if (plotBlocks_)
		// Parameter station list is needed for correction arrows
		BuildParameterStationList();
}


void dna_plot::LoadBinaryFiles()
{
	binary_file_meta_t	bst_meta_, bms_meta_;
	try {
		// Load binary stations data.  Throws runtime_error on failure.
		dna_io_bst bst;
		stationCount_ = bst.load_bst_file(projectSettings_.i.bst_file, &bstBinaryRecords_, bst_meta_);

		// Load binary stations data.  Throws runtime_error on failure.
		dna_io_bms bms;
		bms.load_bms_file(projectSettings_.i.bms_file, &bmsBinaryRecords_, bms_meta_);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}
	
	for_each(
		bstBinaryRecords_.begin(), 
		bstBinaryRecords_.end(), 
		[this] (station_t& stn) {			// use lambda expression
			ReduceStationCoordinates(&stn);
		}
	);

	// Create measurement tally to determine the measurement
	// types that have been supplied
	for_each(
		bmsBinaryRecords_.begin(), 
		bmsBinaryRecords_.end(), 
		[this] (measurement_t& msr) {			// use lambda expression
			
			switch (msr.measType)
			{
			case 'A': // Horizontal angle
				parsemsrTally_.A++;
				break;
			case 'B': // Geodetic azimuth
				parsemsrTally_.B++;
				break;
			case 'C': // Chord dist
				parsemsrTally_.C++;
				break;
			case 'D': // Direction set
				if (msr.vectorCount1 > 0)
					parsemsrTally_.D += msr.vectorCount1 - 1;
				break;
			case 'E': // Ellipsoid arc
				parsemsrTally_.E++;
				break;
			case 'G': // GPS Baseline
				parsemsrTally_.G++;
				break;
			case 'H': // Orthometric height
				parsemsrTally_.H++;
				break;
			case 'I': // Astronomic latitude
				parsemsrTally_.I++;
				break;
			case 'J': // Astronomic longitude
				parsemsrTally_.J++;
				break;
			case 'K': // Astronomic azimuth
				parsemsrTally_.K++;
				break;
			case 'L': // Level difference
				parsemsrTally_.L++;
				break;
			case 'M': // MSL arc
				parsemsrTally_.M++;
				break;
			case 'P': // Geodetic latitude
				parsemsrTally_.P++;
				break;
			case 'Q': // Geodetic longitude
				parsemsrTally_.Q++;
				break;
			case 'R': // Ellipsoidal height
				parsemsrTally_.R++;
				break;
			case 'S': // Slope distance
				parsemsrTally_.S++;
				break;
			case 'V': // Zenith angle
				parsemsrTally_.V++;
				break;
			case 'X': // GPS Baseline cluster
				if (msr.measStart == xMeas)
				{
					if (msr.vectorCount1 == msr.vectorCount2 + 1)
						parsemsrTally_.X += (msr.vectorCount1 * 3);
				}
				break;
			case 'Y': // GPS point cluster
				if (msr.measStart == xMeas)
				{
					if (msr.vectorCount1 == msr.vectorCount2 + 1)
						parsemsrTally_.Y += (msr.vectorCount1 * 3);
				}
				break;
			case 'Z': // Vertical angle
				parsemsrTally_.Z++;
				break;
			}
		}
	);

	measurementCount_ = parsemsrTally_.TotalCount();	

}


void dna_plot::LoadStationMap()
{
	try {
		// Load station map.  Throws runtime_error on failure.
		dna_io_map map;
		map.load_map_file(projectSettings_.i.map_file, &stnsMap_);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}
}
	

void dna_plot::SortandMapStations()
{
	UINT32 stnCount(static_cast<UINT32>(bstBinaryRecords_.size()));
	stnsMap_.clear();
	stnsMap_.reserve(stnCount);

	// Create the Station-Name / ID map
	string_uint32_pair stnID;
	for (UINT32 stnIndex=0; stnIndex<stnCount; stnIndex++)
	{
		stnID.first = bstBinaryRecords_.at(stnIndex).stationName;
		stnID.second = stnIndex;
		stnsMap_.push_back(stnID);
	}

	// sort on station name (i.e. first of the pair)
	sort(stnsMap_.begin(), stnsMap_.end(), StationNameIDCompareName());

	if (stnsMap_.size() < stnCount)
		SignalExceptionPlot("SortandMapStations(): Could not allocate sufficient memory for the Station map.", 0, NULL);
}
	

void dna_plot::LoadSegmentationFile()
{
	vUINT32 v_ContiguousNetList, v_parameterStationCount;
	try {
		// Load segmentation file.  Throws runtime_error on failure.
		dna_io_seg seg;
		seg.load_seg_file(projectSettings_.s.seg_file, 
			blockCount_, blockThreshold_, minInnerStns_,
			v_ISL_, v_JSL_, v_CML_,
			true, &bmsBinaryRecords_,
			&v_measurementCount_, &v_unknownsCount_, &v_ContiguousNetList,
			&v_parameterStationCount);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	if (projectSettings_.p._plot_block_number > blockCount_)
	{
		stringstream ss;
		ss << 
			"Specified block number (" << projectSettings_.p._plot_block_number << 
			") exceeds the total number of blocks (" <<
			blockCount_ << ")." << endl;
		SignalExceptionPlot(ss.str(), 0, NULL);	
	}

	v_msr_file_.resize(blockCount_);
}
	

// The positional uncertainty file is produced by adjust.
void dna_plot::LoadPosUncertaintyFile()
{
	// Load corrections file generated by adjust
	std::ifstream apu_file;
	try {
		// Load apu file.  Throws runtime_error on failure.
		file_opener(apu_file, projectSettings_.o._apu_file, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	bool fullCovarianceMatrix(false);
	UINT32 block(0), stn(0), stn_cov, blockstnCount;
	string strLine;
	stationPosUncertainty_t posUnc;

	v_stn_pu_.resize(blockCount_);
	largest_uncertainty_ = 0.0;

	char line[PRINT_LINE_LENGTH];
	bool dataBlocks(false);
	it_pair_string_vUINT32 _it_stnmap;

	APU_UNITS_UI vcv_units;
	matrix_2d vcv_cart(3, 3), vcv_local(3, 3);

	try {
		
		// read header line
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		//            DYNADJUST POSITIONAL UNCERTAINTY...
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// Version
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// Build
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// File created
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// File name
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// PU Confidence interval

		apu_file.getline(line, PRINT_LINE_LENGTH);		// Stations printed in blocks
		strLine = trimstr(string(line));		
		if (!iequals(strLine.substr(0, 16), "Stations printed"))
		{
			stringstream ss;
			ss << "LoadPosUncertaintyFile(): " << projectSettings_.o._apu_file << " is corrupt." << endl;
			ss << "  Expected to find 'Stations printed in blocks' field." << endl;
			SignalExceptionPlot(ss.str(), 0, "i", &apu_file);
		}
		
		if ((dataBlocks = yesno_uint<bool, string>(strLine.substr(PRINT_VAR_PAD))))
			v_stn_pu_.resize(bstBinaryRecords_.size());
		else
			v_stn_pu_.reserve(bstBinaryRecords_.size());

		apu_file.getline(line, PRINT_LINE_LENGTH);		// Variance matrix units
		strLine = trimstr(string(line));
		if (iequals(strLine.substr(PRINT_VAR_PAD, 3), "XYZ"))
			vcv_units = XYZ_apu_ui;
		else
			vcv_units = ENU_apu_ui;
		

		apu_file.getline(line, PRINT_LINE_LENGTH);		// Full covariance matrix
		strLine = trimstr(string(line));
		if (!iequals(strLine.substr(0, 15), "Full covariance"))
		{
			stringstream ss;
			ss << "LoadPosUncertaintyFile(): " << projectSettings_.o._apu_file << " is corrupt." << endl;
			ss << "  Expected to find 'Full covariance matrix' field." << endl;
			SignalExceptionPlot(ss.str(), 0, "i", &apu_file);
		}
		
		fullCovarianceMatrix = yesno_uint<bool, string>(strLine.substr(PRINT_VAR_PAD));

		// If covariances are printed, then the data will appear in blocks
		if (fullCovarianceMatrix)
			dataBlocks = true;
		
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// Positional uncertainty of ...
		apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
		
		for (block=0; block<blockCount_; ++block)
		{
			if (dataBlocks)
			{
				blockstnCount = static_cast<UINT32>(v_ISL_.at(block).size() + v_JSL_.at(block).size());

				apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
				apu_file.getline(line, PRINT_LINE_LENGTH);		// Block #

				strLine = trimstr(string(line));
				
				if (!iequals(strLine.substr(0, 5), "Block"))
				{
					stringstream ss;
					ss << "LoadPosUncertaintyFile(): " << projectSettings_.o._apu_file << " is corrupt." << endl;
					ss << "  Expected to read Block data, but found " << strLine << endl;
					SignalExceptionPlot(ss.str(), 0, "i", &apu_file);
				}

				if (LongFromString<UINT32>(strLine.substr(6)) != block + 1)
				{
					stringstream ss;
					ss << "LoadPosUncertaintyFile(): " << projectSettings_.o._apu_file << " is corrupt." << endl;
					ss << "  Expected to read data for block " << strLine.substr(6) << ", but found " << strLine << endl;
					SignalExceptionPlot(ss.str(), 0, "i", &apu_file);
				}
			}
			else
			{
				blockstnCount = stationCount_;
				apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
			}
			
			apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// Station
			apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
			
			for (stn=0; stn<blockstnCount;  ++stn)
			{
				apu_file.getline(line, PRINT_LINE_LENGTH);		// Now the data...

				strLine = trimstr(string(line));
				if (strLine.length() < STATION)
				{
					if (apu_file.eof())
						break;				
					continue;
				}

				posUnc._station =	  trimstr(strLine.substr(0, STATION));
				posUnc._latitude =	  DmstoDeg(DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2, LAT_EAST))));
				posUnc._longitude =   DmstoDeg(DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST, LON_NORTH))));
				posUnc._hzPosU =	  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH, STAT)));
				posUnc._vtPosU =	  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT, STAT)));
				posUnc._semimMajor =  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT, PREC)));
				posUnc._semimMinor =  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC, PREC)));
				posUnc._orientation = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC, PREC)));
				posUnc._xx =		  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC+PREC, MSR)));
				posUnc._xy =		  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC+PREC+MSR, MSR)));
				posUnc._xz =		  DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC+PREC+MSR+MSR, MSR)));

				// get yy from next row of variance matrix
				apu_file.getline(line, PRINT_LINE_LENGTH);
				strLine = string(line);
				posUnc._yy = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC+PREC+MSR, MSR)));
				posUnc._yz = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+LAT_EAST+LON_NORTH+STAT+STAT+PREC+PREC+PREC+MSR+MSR, MSR)));

				// Get reference of this station in bstBinaryRecords.
				if (dataBlocks || vcv_units == XYZ_apu_ui)
				{
					_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), posUnc._station, StationNameIDCompareName());

					// Not in the list? (Unlikely since adjust output will not contain stations not in the map, but test anyway)
					if (_it_stnmap.first == _it_stnmap.second)
						continue;
				}

				// Calculate vcv in local reference frame
				if (vcv_units == XYZ_apu_ui)
				{
					// get yy from next row of variance matrix
					apu_file.getline(line, PRINT_LINE_LENGTH);
					strLine = trimstr(string(line));
					posUnc._zz = DoubleFromString<double>(trimstr(strLine));

					vcv_cart.put(0, 0, posUnc._xx);
					vcv_cart.put(0, 1, posUnc._xy);
					vcv_cart.put(0, 2, posUnc._xz);
					vcv_cart.put(1, 1, posUnc._yy);
					vcv_cart.put(1, 2, posUnc._yz);
					vcv_cart.put(2, 2, posUnc._zz);

					vcv_cart.filllower();

					// Calculate correlations in local reference frame
					PropagateVariances_LocalCart<double>(vcv_cart, vcv_local, 
						bstBinaryRecords_.at(_it_stnmap.first->second).currentLatitude, 
						bstBinaryRecords_.at(_it_stnmap.first->second).currentLongitude, false);

					posUnc._xx = vcv_local.get(0, 0);
					posUnc._xy = vcv_local.get(0, 1);
					posUnc._xz = vcv_local.get(0, 2);
					posUnc._yy = vcv_local.get(1, 1);
					posUnc._yz = vcv_local.get(1, 2);
					posUnc._zz = vcv_local.get(2, 2);
				}
				else
					// ENU - don't need up component - ignore last (height) row 
					// of variance matrix
					apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------

				// Data in blocks?  Set the vector element accordingly, otherwise add to the vector
				if (dataBlocks)
					v_stn_pu_.at(_it_stnmap.first->second) = posUnc;
				else
					v_stn_pu_.push_back(posUnc);

				if (fabs(posUnc._semimMajor) > largest_uncertainty_)
					largest_uncertainty_ = posUnc._semimMajor;

				if (!fullCovarianceMatrix)
					continue;
				
				// skip over covariances
				for (stn_cov=stn+1; stn_cov<blockstnCount;  ++stn_cov)
				{
					// ignore covariance block
					apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
					apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
					apu_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
				}
			}
		}
	}
	catch (const ios_base::failure& f) {
		if (!apu_file.eof())
		{
			stringstream ss;
			ss << "LoadPosUncertaintyFile(): An error was encountered when reading " << projectSettings_.o._apu_file << "." << endl << "  " << f.what();
			SignalExceptionPlot(ss.str(), 0, "i", &apu_file);
		}
	}

	apu_file.close();
}

void dna_plot::ComputeStationCorrection(it_vstn_t_const _it_stn, stationCorrections_t& stnCor,
	matrix_2d& currentEstimates, matrix_2d& initialEstimates)
{
	// Compute XYZ for current estimates
	GeoToCart<double>(
		_it_stn->currentLatitude,
		_it_stn->currentLongitude,
		_it_stn->currentHeight,
		currentEstimates.getelementref(0, 0),
		currentEstimates.getelementref(1, 0),
		currentEstimates.getelementref(2, 0),
		datum_.GetEllipsoidRef());

	// Compute XYZ for initial estimates
	GeoToCart<double>(
		Radians(_it_stn->initialLatitude),
		Radians(_it_stn->initialLongitude),
		_it_stn->initialHeight + _it_stn->geoidSep,
		initialEstimates.getelementref(0, 0),
		initialEstimates.getelementref(1, 0),
		initialEstimates.getelementref(2, 0),
		datum_.GetEllipsoidRef());

	// compute vertical angle
	stnCor._vAngle = VerticalAngle<double>(
		initialEstimates.get(0, 0),		// X1
		initialEstimates.get(1, 0),		// Y1
		initialEstimates.get(2, 0),		// Z1
		currentEstimates.get(0, 0),		// X2
		currentEstimates.get(1, 0),		// Y2
		currentEstimates.get(2, 0),		// Z2
		_it_stn->currentLatitude,
		_it_stn->currentLongitude,
		_it_stn->currentLatitude,
		_it_stn->currentLongitude,
		0., 0.,
		&stnCor._east, &stnCor._north, &stnCor._up);

	if (fabs(stnCor._east) < PRECISION_1E5)
		if (fabs(stnCor._north) < PRECISION_1E5)
			if (fabs(stnCor._up) < PRECISION_1E5)
				stnCor._vAngle = 0.;
	
	// compute horizontal correction
	stnCor._hDistance = magnitude(stnCor._east, stnCor._north);
			
	// calculate azimuth
	stnCor._azimuth = Direction<double>(
		initialEstimates.get(0, 0),		// X1
		initialEstimates.get(1, 0),		// Y1
		initialEstimates.get(2, 0),		// Z1
		currentEstimates.get(0, 0),		// X2
		currentEstimates.get(1, 0),		// Y2
		currentEstimates.get(2, 0),		// Z2
		_it_stn->currentLatitude,
		_it_stn->currentLongitude,
		&stnCor._east, &stnCor._north);

	if (fabs(stnCor._east) < PRECISION_1E5)
		if (fabs(stnCor._north) < PRECISION_1E5)
			stnCor._azimuth = 0.;

	// calculate distances
	stnCor._sDistance = magnitude<double>(
		initialEstimates.get(0, 0),		// X1
		initialEstimates.get(1, 0),		// Y1
		initialEstimates.get(2, 0),		// Z1
		currentEstimates.get(0, 0),		// X2
		currentEstimates.get(1, 0),		// Y2
		currentEstimates.get(2, 0));	// Z2
}
	

void dna_plot::ComputeStationCorrections()
{
	// reserve enough memory for each station
	v_stn_corrs_.reserve(bstBinaryRecords_.size());

	stationCorrections_t stnCor;
	matrix_2d currentEstimates(3, 1), initialEstimates(3, 1);

	average_correction_ = 0.0;
	UINT32 correction_count(0);

	// compute corrections for every station in the network
	for (it_vstn_t_const _it_stn=bstBinaryRecords_.begin();
		_it_stn!=bstBinaryRecords_.end(); 
		++_it_stn)
	{
		// Constrained?  No correction!
		if (_it_stn->stationConst[0] == 'C' && 
			_it_stn->stationConst[1] == 'C' && 
			_it_stn->stationConst[2] == 'C')
		{
			// push a zero correction
			v_stn_corrs_.push_back(stationCorrections(_it_stn->stationName));
			continue;
		}

		stnCor._station = _it_stn->stationName;
		ComputeStationCorrection(_it_stn, stnCor,
			currentEstimates, initialEstimates);

		average_correction_ += stnCor._hDistance;
		correction_count++;

		v_stn_corrs_.push_back(stnCor);
	}

	// determine average correction length
	average_correction_ /= correction_count;
}
	
void dna_plot::BuildParameterStationList()
{
	if (!plotBlocks_)
		return;

	v_parameterStationList_.resize(blockCount_);

	vUINT32 vStns;
	it_vUINT32_const _it_stn;

	for (UINT32 block(0); block!=blockCount_; ++block)
	{
		// Form combined stations list
		v_parameterStationList_.at(block) = v_ISL_.at(block);
		v_parameterStationList_.at(block).insert(v_parameterStationList_.at(block).end(),
			v_JSL_.at(block).begin(), v_JSL_.at(block).end());
		sort(v_parameterStationList_.at(block).begin(), v_parameterStationList_.at(block).end());

	}
}

// The station corrections file is produced by adjust.  Note that corrections for *all* stations may not 
// be printed, since the user has the option to restrict the output of station corrections using
// horizontal and vertical thresholds.
// Also, the coorctions file may contain a list of stations printed in blocks, or in a contiguous list.
// These options create complexities!
//
// The best way to deal with station corrections is to ignore the cor file and to recalculate corrections
// from the original coordinates in the bst file.
void dna_plot::LoadCorrectionsFile()
{
	// Load corrections file generated by adjust
	std::ifstream cor_file;
	try {
		// Load corrections file.  Throws runtime_error on failure.
		file_opener(cor_file, projectSettings_.o._cor_file, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		SignalExceptionPlot(e.what(), 0, NULL);
	}

	UINT32 correction_count(0);
	UINT32 block(0), stn(0), blockstnCount, corFileBlockCount(blockCount_);
	string strLine;
	stationCorrections_t stnCor;

	average_correction_ = 0.0;

	char line[PRINT_LINE_LENGTH];
	bool dataBlocks(false);
	it_pair_string_vUINT32 _it_stnmap;

	try {
		
		// read header line
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		//            DYNADJUST CORRECT...
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// Version
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// Build
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// File created
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// File name
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		cor_file.getline(line, PRINT_LINE_LENGTH);		// Stations printed in blocks
		
		// Get the yes/no string and convert to bool
		strLine = trimstr(string(line));
		if (!iequals(strLine.substr(0, 16), "Stations printed"))
		{
			stringstream ss;
			// TODO - make use of Boost current function
			// http://www.boost.org/doc/libs/1_58_0/boost/current_function.hpp
			// and if required, print the filename and line number using __FILE__ and __LINE__
			// https://stackoverflow.com/questions/15305310/predefined-macros-for-function-name-func
			ss << "LoadCorrectionsFile(): " << projectSettings_.o._cor_file << " is corrupt." << endl;
			ss << "  Expected to find 'Stations printed in blocks' field." << endl;
			SignalExceptionPlot(ss.str(), 0, "i", &cor_file);
		}

		if ((dataBlocks = yesno_uint<bool, string>(strLine.substr(PRINT_VAR_PAD))))		// Data printed in blocks?
			v_stn_corrs_.resize(bstBinaryRecords_.size());
		else
		{
			v_stn_corrs_.reserve(bstBinaryRecords_.size());
			corFileBlockCount = 1;
		}

		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// Corrections to Stations
		cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------
		
		for (block=0; block<corFileBlockCount; ++block)
		{
			if (dataBlocks)
			{
				blockstnCount = static_cast<UINT32>(v_ISL_.at(block).size() + v_JSL_.at(block).size());

				cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
				cor_file.getline(line, PRINT_LINE_LENGTH);		// Block #

				strLine = trimstr(string(line));
				
				if (!iequals(strLine.substr(0, 5), "Block"))
				{
					stringstream ss;
					ss << "LoadCorrectionsFile(): " << projectSettings_.o._cor_file << " is corrupt." << endl;
					ss << "  Expected to read Block data, but found " << strLine << endl;
					SignalExceptionPlot(ss.str(), 0, "i", &cor_file);
				}

				if (LongFromString<UINT32>(strLine.substr(6)) != block + 1)
				{
					stringstream ss;
					ss << "LoadCorrectionsFile(): " << projectSettings_.o._cor_file << " is corrupt." << endl;
					ss << "  Expected to read data for block " << strLine.substr(6) << ", but found " << strLine << endl;
					SignalExceptionPlot(ss.str(), 0, "i", &cor_file);
				}
			}
			else
			{
				blockstnCount = stationCount_;
				cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// 
			}
			
			cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// Station
			cor_file.ignore(PRINT_LINE_LENGTH, '\n');		// ------------------------

			for (stn=0; stn<blockstnCount;  ++stn)
			{
				cor_file.getline(line, PRINT_LINE_LENGTH);		// Now the data...

				strLine = trimstr(string(line));
				if (strLine.length() < STATION)
				{
					if (cor_file.eof())
						break;				
					continue;
				}

				//// print...
				//// station and constraint
				//os << setw(STATION) << left << bstBinaryRecords_.at(stn).stationName << setw(PAD2) << " ";
				//// data
				//os << setw(MSR) << right << FormatDmsString(RadtoDms(azimuth), 4, true, false) << 
				//	setw(MSR) << right << FormatDmsString(RadtoDms(vertical_angle), 4, true, false) << 
				//	setw(MSR) << setprecision(PRECISION_MTR_STN) << fixed << right << slope_distance << 
				//	setw(MSR) << setprecision(PRECISION_MTR_STN) << fixed << right << horiz_distance << 
				//	setw(HEIGHT) << setprecision(PRECISION_MTR_STN) << fixed << right << local_12e << 
				//	setw(HEIGHT) << setprecision(PRECISION_MTR_STN) << fixed << right << local_12n << 
				//	setw(HEIGHT) << setprecision(PRECISION_MTR_STN) << fixed << right << local_12up << endl;

				stnCor._station   = trimstr(strLine.substr(0, STATION));
				stnCor._azimuth   = ParseDmsString<double>(trimstr(strLine.substr(STATION+PAD2, MSR)), " ");
				stnCor._vAngle	  = ParseDmsString<double>(trimstr(strLine.substr(STATION+PAD2+MSR, MSR)), " ");
				stnCor._sDistance = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+MSR+MSR, MSR)));
				stnCor._hDistance = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+MSR+MSR+MSR, MSR)));
				stnCor._east      = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+MSR+MSR+MSR+MSR, HEIGHT)));
				stnCor._north     = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+MSR+MSR+MSR+MSR+HEIGHT, HEIGHT)));
				stnCor._up        = DoubleFromString<double>(trimstr(strLine.substr(STATION+PAD2+MSR+MSR+MSR+MSR+HEIGHT+HEIGHT, HEIGHT)));


				// Get reference of this station in bstBinaryRecords.
				if (dataBlocks)
				{
					_it_stnmap = equal_range(stnsMap_.begin(), stnsMap_.end(), stnCor._station, StationNameIDCompareName());

					// Not in the list? (Unlikely since adjust output will not contain stations not in the map, but test anyway)
					if (_it_stnmap.first == _it_stnmap.second)
						continue;
		
					v_stn_corrs_.at(_it_stnmap.first->second) = stnCor;
				}
				else
					v_stn_corrs_.push_back(stnCor);

				average_correction_ += stnCor._hDistance;
				correction_count++;
			}
		}
	}
	catch (const ios_base::failure& f) {
		if (!cor_file.eof())
		{
			stringstream ss;
			ss << "LoadCorrectionsFile(): An error was encountered when reading " << projectSettings_.o._cor_file << "." << endl << "  " << f.what();
			SignalExceptionPlot(ss.str(), 0, "i", &cor_file);
		}
	}

	// determine average correction length
	average_correction_ /= correction_count;

	cor_file.close();
}
	

// Name:				SignalExceptionPlot
// Purpose:				Closes all files (if file pointers are passed in) and throws runtime_error
// Called by:			Any
// Calls:				runtime_error()
void dna_plot::SignalExceptionPlot(const string& msg, const int& line_no, const char *streamType, ...)
{
	plotStatus_ = PLOT_EXCEPTION_RAISED;

	if (streamType == NULL)
		throw NetPlotException(msg, line_no);

	std::ofstream* ofsDynaML;
	std::ifstream* ifsbinaryFile;

	va_list argptr; 
	va_start(argptr, streamType);

	while (*streamType != '\0')
	{
		//ifstream
		if (*streamType == 'i' )
		{
			ifsbinaryFile = va_arg(argptr, std::ifstream*);
			ifsbinaryFile->close();
		}
		//ofstream
		if (*streamType == 'o' )
		{
			ofsDynaML = va_arg(argptr, std::ofstream*);
			ofsDynaML->close();
		}
		streamType++;
	}

	va_end(argptr);

	throw NetPlotException(msg, line_no);
}


//void dna_plot::PrintGMTPlotCoords(vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, ostream& osStn, ostream& osStn2, ostream& osMsr)
//{
//	_it_vdnastnptr _it_stn(vStations->begin());
//	vdnaMsrPtr::iterator _it_msr(vMeasurements->begin());
//	
//	// Print station coords
//	for (; _it_stn!=vStations->end(); _it_stn++)
//		_it_stn->get()->coutStationData(osStn, osStn2, GMT_OUT);
//
//	//v_string_uint32_pair::iterator _it_stnmap(stnsMap_.begin());
//
//	UINT32 precision;
//	_COORD_TYPE_ ctType;
//
//	vector<CDnaDirection>* vdirns;
//	vector<CDnaGpsBaseline>* vgpsBsls;
//
//	for (; _it_msr!=vMeasurements->end(); _it_msr++)
//	{
//		switch (_it_msr->get()->GetTypeC())
//		{
//		case 'A':	// Horizontal angles
//			// Print 3 - 1 - 2
//			//
//			// Third
//			it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//				_it_msr->get()->GetTarget2(), StationNameIDCompareName());
//
//			if (it_stnmap_range.first == it_stnmap_range.second)
//			{
//				cout << _it_msr->get()->GetTarget2() << " is not in the list of network stations.";
//				continue;
//			}
//			precision = 3;
//			if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//				precision = 10;
//
//			osMsr << setprecision(precision) << fixed << 
//				(ctType == LLH_type_i ? 
//					Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()) :
//					vStations->at(it_stnmap_range.first->second)->GetYAxis());
//			osMsr << "  ";
//			osMsr << setprecision(precision) << fixed << 
//				(ctType == LLH_type_i ? 
//					Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()) :
//					vStations->at(it_stnmap_range.first->second)->GetXAxis());
//			osMsr << endl;
//		case 'B': // Geodetic azimuth
//		case 'K': // Astronomic azimuth
//		case 'C': // Chord dist
//		case 'E': // Ellipsoid arc
//		case 'M': // MSL arc
//		case 'S': // Slope distance
//		case 'L': // Level difference
//		case 'V': // Zenith angle
//		case 'Z': // Vertical angle
//			// First
//			it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//				_it_msr->get()->GetFirst(), StationNameIDCompareName());
//
//			if (it_stnmap_range.first == it_stnmap_range.second)
//			{
//				cout << _it_msr->get()->GetFirst() << " is not in the list of network stations.";
//				continue;
//			}
//			precision = 3;
//			if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//				precision = 10;
//
//			osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//			osMsr << "  ";
//			osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//			osMsr << endl;
//			
//			// Second
//			it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//				_it_msr->get()->GetTarget(), StationNameIDCompareName());
//
//			if (it_stnmap_range.first == it_stnmap_range.second)
//			{
//				cout << _it_msr->get()->GetTarget() << " is not in the list of network stations.";
//				continue;
//			}
//			precision = 3;
//			if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//				precision = 10;
//
//			osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//			osMsr << "  ";
//			osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//			osMsr << endl << "#" << endl;
//			break;
//		case 'D': // Direction set
//			vdirns = _it_msr->get()->GetDirections_ptr();
//			PrintGMTPlotCoords_D(vStations, vdirns, osMsr);
//			break;
//		case 'G': // GPS Baseline
//		case 'X': // GPS Baseline cluster
//			vgpsBsls = _it_msr->get()->GetBaselines_ptr();
//			PrintGMTPlotCoords_GX(vStations, vgpsBsls, osMsr);
//			break;
//		}
//	}
//}
//
//
//void dna_plot::PrintGMTPlotCoords_D(vdnaStnPtr* vStations, vector<CDnaDirection>* vDirections, ostream& osMsr)
//{
//	vector<CDnaDirection>::iterator _it_dirn(vDirections->begin());
//	//v_string_uint32_pair::iterator _it_stnmap(stnsMap_.begin());
//
//	UINT32 precision;
//	_COORD_TYPE_ ctType;
//
//	for (; _it_dirn!=vDirections->end(); _it_dirn++)
//	{
//		// First
//		it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//			_it_dirn->GetFirst(), StationNameIDCompareName());
//
//		if (it_stnmap_range.first == it_stnmap_range.second)
//		{
//			cout << _it_dirn->GetFirst() << " is not in the list of network stations.";
//			continue;
//		}
//		precision = 3;
//		if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//			precision = 10;
//
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//		osMsr << "  ";
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//		osMsr << endl;
//
//		// Second
//		it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//			_it_dirn->GetTarget(), StationNameIDCompareName());
//
//		if (it_stnmap_range.first == it_stnmap_range.second)
//		{
//			cout << _it_dirn->GetTarget() << " is not in the list of network stations.";
//			continue;
//		}
//		precision = 3;
//		if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//			precision = 10;
//
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//		osMsr << "  ";
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//		osMsr << endl << "#" << endl;
//	}
//}
//
//
//void dna_plot::PrintGMTPlotCoords_GX(vdnaStnPtr* vStations, vector<CDnaGpsBaseline>* vGPSBaselines, ostream& osMsr)
//{
//	vector<CDnaGpsBaseline>::iterator _it_bsl(vGPSBaselines->begin());
//	//v_string_uint32_pair::iterator _it_stnmap(stnsMap_.begin());
//
//	UINT32 precision;
//	_COORD_TYPE_ ctType;
//
//	for (; _it_bsl!=vGPSBaselines->end(); _it_bsl++)
//	{
//		// First
//		it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//			_it_bsl->GetFirst(), StationNameIDCompareName());
//
//		if (it_stnmap_range.first == it_stnmap_range.second)
//		{
//			cout << _it_bsl->GetFirst() << " is not in the list of network stations.";
//			continue;
//		}
//		precision = 3;
//		if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//			precision = 10;
//
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//		osMsr << "  ";
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//		osMsr << endl;
//
//		// Second
//		it_stnmap_range = equal_range(stnsMap_.begin(), stnsMap_.end(), 
//			_it_bsl->GetTarget(), StationNameIDCompareName());
//
//		if (it_stnmap_range.first == it_stnmap_range.second)
//		{
//			cout << _it_bsl->GetTarget() << " is not in the list of network stations.";
//			continue;
//		}
//		precision = 3;
//		if ((ctType = vStations->at(it_stnmap_range.first->second)->GetMyCoordTypeC()) == LLH_type_i)
//			precision = 10;
//
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetYAxis()): vStations->at(it_stnmap_range.first->second)->GetYAxis());
//		osMsr << "  ";
//		osMsr << setprecision(precision) << fixed << (ctType == LLH_type_i ? Degrees(vStations->at(it_stnmap_range.first->second)->GetXAxis()): vStations->at(it_stnmap_range.first->second)->GetXAxis());
//		osMsr << endl << "#" << endl;
//	}
//}



void dna_plot::ReduceStationCoordinates(station_t* stationRecord)
{
	stationRecord->initialLatitude = Degrees(stationRecord->initialLatitude);
	stationRecord->initialLongitude = Degrees(stationRecord->initialLongitude);
}

void dna_plot::NormaliseScale(double& scale)
{
	if (scale > 5000000. && scale < 10000000.)
		scale = 5000000.;
	else if (scale > 1250000.)
		scale = 1500000.;
	else if (scale > 1000000.)
		scale = 1250000.;
	else if (scale > 750000.)
		scale = 1000000.;
	else if (scale > 500000.)
		scale = 750000.;
	else if (scale > 350000.)
		scale = 500000.;
	else if (scale > 250000.)
		scale = 350000.;
	else if (scale > 200000.)
		scale = 250000.;
	else if (scale > 150000.)
		scale = 200000.;
	else if (scale > 100000.)
		scale = 150000.;
	else if (scale > 75000.)
		scale = 100000.;
	else if (scale > 50000.)
		scale = 75000.;
	else if (scale > 35000.)
		scale = 50000.;
	else if (scale > 25000.)
		scale = 35000.;
	else if (scale > 10000.)
		scale = 25000.;
	else if (scale > 7500.)
		scale = 10000.;
	else if (scale > 5000.)
		scale = 7500.;
	else if (scale > 2500.)
		scale = 5000.;
	else if (scale > 1000.)
		scale = 2500.;
	else if (scale > 500.)
		scale = 1000.;
	else
		scale = 500.;
}

void dna_plot::NormaliseScaleBar(double& scale_bar_width)
{
	if (scale_bar_width > 1.)
		scale_bar_width = floor(scale_bar_width);

	if (scale_bar_width > 10000)
		scale_bar_width -= (int)scale_bar_width % 10000;
	else if (scale_bar_width > 1000)
		scale_bar_width -= (int)scale_bar_width % 1000;
	else if (scale_bar_width > 100)
		scale_bar_width -= (int)scale_bar_width % 100;
	else if (scale_bar_width > 10)
		scale_bar_width -= (int)scale_bar_width % 10;
	else if (scale_bar_width > 5)
		scale_bar_width -= (int)scale_bar_width % 5;
	else if (scale_bar_width > 1)
		scale_bar_width -= (int)scale_bar_width % 1;
	else if (scale_bar_width > 0.5)
		scale_bar_width = 0.5;
	else if (scale_bar_width > 0.25)
		scale_bar_width = 0.25;
	
	else if (scale_bar_width > 0.1)
		scale_bar_width = 0.1;
	else if (scale_bar_width > 0.075)
		scale_bar_width = 0.075;
	else if (scale_bar_width > 0.05)
		scale_bar_width = 0.05;
	else if (scale_bar_width > 0.025)
		scale_bar_width = 0.025;
	
	else if (scale_bar_width > 0.01)
		scale_bar_width = 0.01;
	else if (scale_bar_width > 0.0075)
		scale_bar_width = 0.0075;
	else if (scale_bar_width > 0.005)
		scale_bar_width = 0.005;
	else if (scale_bar_width > 0.0025)
		scale_bar_width = 0.0025;
	
	else if (scale_bar_width > 0.0001)
		scale_bar_width = 0.0001;
	else if (scale_bar_width > 0.00075)
		scale_bar_width = 0.00075;
	else if (scale_bar_width > 0.0005)
		scale_bar_width = 0.0005;
	else if (scale_bar_width > 0.00025)
		scale_bar_width = 0.00025;
	
}


void dna_plot::NormaliseGraticule(double& graticule_width, UINT32& graticule_width_precision)
{
	double graticule = graticule_width * 3600.0;	// convert to seconds
	if (graticule > 1.)
		graticule = floor(graticule);
	else
		graticule_width_precision = 16;

	if (graticule > 7200)			// 2 degrees
		graticule -= (int)graticule % 7200;
	else if (graticule > 3600)		// 1 degrees
		graticule -= (int)graticule % 3600;
	else if (graticule > 1800)		// 30 minutes
		graticule -= (int)graticule % 1800;
	else if (graticule > 900)		// 15 minutes
		graticule -= (int)graticule % 900;
	else if (graticule > 600)		// 10 minutes
		graticule -= (int)graticule % 600;
	else if (graticule > 300)		// 5 minutes
		graticule -= (int)graticule % 300;
	else if (graticule > 60)		// 1 minute
		graticule -= (int)graticule % 60;
	else if (graticule > 30)		// 30 seconds
		graticule -= (int)graticule % 30;
	else if (graticule > 15)		// 15 seconds
		graticule -= (int)graticule % 15;
	else if (graticule > 10)		// 10 seconds
		graticule -= (int)graticule % 10;
	else if (graticule > 5)			// 5 seconds
		graticule -= (int)graticule % 5;
	else if (graticule > 1)			// 5 seconds
		graticule -= (int)graticule % 1;
	else if (graticule > 0.5)		// .5 seconds
		graticule = 0.5;
	else if (graticule > 0.25)		// .25 seconds
		graticule = 0.25;


	// convert to degrees
	graticule_width = graticule / 3600.;
}

void dna_plot::SelectCoastlineResolution(const double& dDimension, string& coastResolution, plot_settings* plotCriteria)
{
	plotCriteria->_coasline_resolution = low;
	coastResolution = "l";
	
	if (dDimension < 900000)
	{
		plotCriteria->_coasline_resolution = full;
		coastResolution = "f";
		return;
	}
	if (dDimension < 2000000)
	{
		coastResolution = "h";
		plotCriteria->_coasline_resolution = high;
		return;
	}
	if (dDimension < 9000000)
	{
		plotCriteria->_coasline_resolution = intermediate;
		coastResolution = "i";
	}	
}

}	// namespace mathcomp
}	// namespace dynadjust
