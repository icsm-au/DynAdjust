//============================================================================
// Name         : dnageoid.cpp
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
// Description  : AusGeoid Grid File (NTv2) Interpolation library
//============================================================================


#include <dynadjust/dnageoid/dnageoid.hpp>

namespace dynadjust { namespace geoidinterpolation {

dna_geoid_interpolation::dna_geoid_interpolation()
	: m_isReading(false)
	, m_isWriting(false)
	, m_pGridfile(0)
	, m_dPercentComplete(0.0)
	, m_iBytesRead(0)
	, m_Grid_Success(ERR_TRANS_SUCCESS)
{
	
}

dna_geoid_interpolation::dna_geoid_interpolation(const dna_geoid_interpolation& orig)
{
	*m_pGridfile = *orig.m_pGridfile;
	m_dPercentComplete = orig.m_dPercentComplete;
	m_iBytesRead = orig.m_iBytesRead;
	m_Grid_Success = orig.m_Grid_Success;
	m_isReading = orig.m_isReading;
	m_isWriting = orig.m_isWriting;
}
	

dna_geoid_interpolation::~dna_geoid_interpolation()
{
	ClearGridFileMemory();
}


void dna_geoid_interpolation::ApplyAusGeoidGrid(geoid_point *agCoord, const int& method)
{
	double dlat, dlon;
	
	dlat = m_pGridfile->dLat = agCoord->cVar.dLatitude * DEG_TO_SEC;
	dlon = m_pGridfile->dLong = agCoord->cVar.dLongitude * -DEG_TO_SEC;

	// Outside the extents of the grid
	if ((agCoord->cVar.IO_Status = FindSubGrid()) < 0)
	{
		if (m_fileMode)
			throw NetGeoidException("", agCoord->cVar.IO_Status);
		else
			throw NetGeoidException(ErrorString(agCoord->cVar.IO_Status, m_inputCoordinates), agCoord->cVar.IO_Status);
	}
	
	// Interpolate values
	if (method == BILINEAR)
	{
		if ((agCoord->cVar.IO_Status = InterpolateNvalue_BiLinear(dlat, dlon, agCoord)) != ERR_TRANS_SUCCESS)
			throw NetGeoidException(ErrorString(agCoord->cVar.IO_Status), agCoord->cVar.IO_Status);
	}
	else if (method == BICUBIC)
	{
		if ((agCoord->cVar.IO_Status = InterpolateNvalue_BiCubic(dlat, dlon, agCoord)) != ERR_TRANS_SUCCESS)
			throw NetGeoidException(ErrorString(agCoord->cVar.IO_Status), agCoord->cVar.IO_Status);
	}
	else
		throw NetGeoidException(ErrorString(ERR_INTERPOLATION_TYPE), ERR_INTERPOLATION_TYPE);
}

string dna_geoid_interpolation::ReturnBadStationRecords()
{
	stringstream ssPoints;

	it_vstn_t stn_it;

	for (stn_it=bstBadPoints_.begin(); stn_it!=bstBadPoints_.end(); ++stn_it)
	{
		ssPoints << fixed << setprecision(9) <<
			"   " <<
			stn_it->stationName << ", " <<
			setw(14) << FormatDmsString(RadtoDms(stn_it->currentLatitude), 7, true, false) << ", " <<
			setw(14) << FormatDmsString(RadtoDms(stn_it->currentLongitude), 7, true, false) << endl;
	}

	return ssPoints.str();
}

void dna_geoid_interpolation::PopulateStationRecords(const int& method, bool convertHeights)
{
	geoid_point agCoord;
	double dlat, dlon;
	it_vstn_t stn_it;

	m_pointsInterpolated = m_pointsNotInterpolated = 0;
	bstBadPoints_.clear();

	for (stn_it=bstBinaryRecords_.begin(); stn_it!=bstBinaryRecords_.end(); ++stn_it)
	{
		dlat = m_pGridfile->dLat = stn_it->currentLatitude * RAD_TO_SEC;
		dlon = m_pGridfile->dLong = stn_it->currentLongitude * -RAD_TO_SEC;
		
		agCoord.cVar.IO_Status = FindSubGrid();
		switch (agCoord.cVar.IO_Status)
		{
		case ERR_PT_OUTSIDE_GRID:		// 13
		case ERR_FINDSUBGRID_OUTSIDE:	// -1
			++m_pointsNotInterpolated;
			bstBadPoints_.push_back(*stn_it);
			continue;					// this point is outside the grid file extents
		case ERR_TRANS_SUCCESS:
			break;						// success
		default:						// any error
			throw NetGeoidException(ErrorString(agCoord.cVar.IO_Status), agCoord.cVar.IO_Status);
		}
		
		// Interpolate values
		if (method == BILINEAR)
		{
			if ((agCoord.cVar.IO_Status = InterpolateNvalue_BiLinear(dlat, dlon, &agCoord)) != ERR_TRANS_SUCCESS)
				throw NetGeoidException(ErrorString(agCoord.cVar.IO_Status), agCoord.cVar.IO_Status);
		}
		else if (method == BICUBIC)
		{
			if ((agCoord.cVar.IO_Status = InterpolateNvalue_BiCubic(dlat, dlon, &agCoord)) != ERR_TRANS_SUCCESS)
				throw NetGeoidException(ErrorString(agCoord.cVar.IO_Status), agCoord.cVar.IO_Status);
		}
		else
			throw NetGeoidException(ErrorString(ERR_INTERPOLATION_TYPE), ERR_INTERPOLATION_TYPE);

		++m_pointsInterpolated;

		stn_it->geoidSep = static_cast<float>(agCoord.gVar.dN_value);
		// deflection values in the grid file are in seconds, so convert to radians
		stn_it->meridianDef = SecondstoRadians(static_cast<float>(agCoord.gVar.dDefl_meridian));
		stn_it->verticalDef = SecondstoRadians(static_cast<float>(agCoord.gVar.dDefl_primev));

		if (convertHeights)
			if (stn_it->suppliedHeightRefFrame == ORTHOMETRIC_type_i)
				stn_it->currentHeight = stn_it->initialHeight + stn_it->geoidSep;
	}
}


void dna_geoid_interpolation::Version(char* version)
{
	string msg;
	fileproc_help_header(&msg);
	sprintf(version, "%s", msg.c_str());
}

void dna_geoid_interpolation::ProcessCsvFile(std::ifstream* f_in, std::ofstream* f_out, std::ofstream* f_dnageo, const int& method, const int& intEllipsoidtoOrtho, const int& intDmsFlag)
{
	m_fileMode = true;

	char cPoint[50];
	double latitude, longitude, original_height;

	// calculate file length
	f_in->seekg(0, ios::end);		// set file pointer to end
	streamoff lFileLen = f_in->tellg();
	f_in->seekg(0, ios::beg);		// reset file pointer to beginning

	char cBuf[MAX_RECORD_LENGTH];

	geoid_point apPt;

	bool iSuccess;
	int fieldCount;
	bool bheight_data;
	string sBuf;

	// Print header
	*f_out << left << setw(11) << "Point" << "," <<
		setw(16) << right << "Latitude" << "," <<
		setw(16) << right << "Longitude" << "," <<
		setw(9) << right << "Orig. Ht" << ",";
	if (intEllipsoidtoOrtho == 1)
		*f_out << setw(9) << right << trimstr(string(m_pGridfile->chSystem_t)) <<	",";	// "New Ht" 
	else
		*f_out << setw(9) << right << trimstr(string(m_pGridfile->chSystem_f)) <<	",";	// "New Ht" 
	*f_out << setw(9) << right << "N value" << "," <<
		setw(9) << right << "D.Merid" << "," <<
		setw(9) << right << "D.PrimeV" << endl;

	while (!f_in->eof())			// while EOF not found
	{
		try {
			f_in->getline(cBuf, MAX_RECORD_LENGTH);
		}
		catch (...) {
			if (f_in->eof())
				return;
		}

		m_iBytesRead = (int)f_in->tellg();
		m_dPercentComplete = (double)m_iBytesRead / (double)lFileLen * 100.0;

		sBuf = cBuf;

		latitude = UNRELIABLE+1;
		longitude = UNRELIABLE+1;
		original_height = 0;

		// Is the line empty or comprised of only blank spaces?
		if (trimstr(sBuf).empty())
			continue;

		try {
			// Get records and check for sufficient fields or blank fields
			if ((fieldCount = GetFields(cBuf, ',', false, "sfff", cPoint, &latitude, &longitude, &original_height)) < 3)
			{
				// This point may be reached if the point id is missing, which is not a failure
				// in itself, so check if the coordinates are valid.
				if (latitude < -90.0 || latitude > 90.0 || 
					longitude < -180.0 || longitude > 180.0)
				{
					// Coordinates are invalid
					*f_out << left << "ERROR (" << ERR_NUM_CSV_FIELDS << "): " <<
						ErrorString(ERR_NUM_CSV_FIELDS, sBuf) << endl;
					continue;
				}
			}
		}
		catch (...) {
			*f_out << left << "ERROR (" << ERR_READING_DATA << "): " <<
				ErrorString(ERR_READING_DATA, cBuf) << endl;
			continue;
		}

		if (fabs(latitude) > UNRELIABLE || fabs(longitude) > UNRELIABLE)
		{
			// a comma was found, but no data
			*f_out << left << "ERROR (" << ERR_NUM_CSV_FIELDS << "): " <<
				ErrorString(ERR_NUM_CSV_FIELDS, sBuf) << endl;
			continue;
		}

		if (fabs(latitude) > 90.0 || fabs(longitude) > 180.0)
		{
			// latitude or longitude invalid
			*f_out << left << "ERROR (" << ERR_INVALID_INPUT << "): " <<
				ErrorString(ERR_INVALID_INPUT, sBuf) << endl;
			continue;
		}

		if (fieldCount < 4)
		{
			bheight_data = false;
			original_height = 0;
		}
		else
			bheight_data = true;

		if (intDmsFlag == DMS)
			apPt.cVar.dLatitude = DmstoDeg(latitude);
		else
			apPt.cVar.dLatitude = latitude;

		if (intDmsFlag == DMS)
			apPt.cVar.dLongitude = DmstoDeg(longitude);
		else
			apPt.cVar.dLongitude = longitude;

		apPt.cVar.dHeight = original_height;
		apPt.cVar.IO_Status = ERR_TRANS_SUCCESS;

		original_height = apPt.cVar.dHeight;
		
		iSuccess = false;
		try {
			ApplyAusGeoidGrid(&apPt, method);
			iSuccess = true;
		}
		catch (const NetGeoidException& e) {
			stringstream s(e.what());
			iSuccess = false;
		}

		if (iSuccess)
		{
			++m_pointsInterpolated;

			if (intEllipsoidtoOrtho == 1)
				apPt.cVar.dHeight -= apPt.gVar.dN_value;		// H = h - N
			else
				apPt.cVar.dHeight += apPt.gVar.dN_value;		// h = H + N
		}
		else
		{
			++m_pointsNotInterpolated;
			apPt.cVar.dHeight = -UNRELIABLE;

			// Format error message using input coordinates
			stringstream ss;
			strcpy(cBuf, sBuf.c_str());
			char inputLatitude[50], inputLongitude[50];
			GetFields(cBuf, ',', false, "sss", cPoint, inputLatitude, inputLongitude);
			ss << trimstr(string(inputLatitude)) << ", " << trimstr(string(inputLongitude));

			// print dat
			*f_out << left << "ERROR (" << apPt.cVar.IO_Status << "): " <<
				ErrorString(apPt.cVar.IO_Status, ss.str()) << endl;
			continue;
		}

		// print csv 
		// const char* const CSV_DDEG = "%-11s,%16.9f,%16.9f,%9.3f\n";
		*f_out << left << setw(11) << cPoint << "," <<
			setw(16) << setprecision(9) << fixed << right << latitude << "," <<
			setw(16) << setprecision(9) << fixed << right << longitude << ",";
		
		
		if (bheight_data)
		{
			// A height was supplied...
			// Print the 'original' height
			*f_out << setw(9) << setprecision(3) << fixed << right << original_height << ",";
			// Print the 'transformed' height
			*f_out << setw(9) << setprecision(3) << fixed << right << apPt.cVar.dHeight << ",";
		}
		else
		{
			// no height was supplied, so print blanks for original height and derived heights
			*f_out << setw(9) << right << " " << ",";
			*f_out << setw(9) << right << " " << ",";
		}			

		// As per Nick Brown's request, print the N value regardless of input height
		*f_out << setw(9) << setprecision(3) << fixed << right << apPt.gVar.dN_value << ",";

		// Print the deflection values
		*f_out << setw(9) << setprecision(3) << fixed << right << apPt.gVar.dDefl_meridian << "," <<
			setw(9) << setprecision(3) << fixed << right << apPt.gVar.dDefl_primev << endl;

		// write to dna geo file
		if (m_exportDNAGeoidFile)
			PrintDNA1GeoidRecord(*f_dnageo, 
				cPoint, apPt.gVar.dN_value, apPt.gVar.dDefl_meridian, apPt.gVar.dDefl_primev);
	}		
}
	

void dna_geoid_interpolation::ProcessDatFile(std::ifstream* f_in, std::ofstream* f_out, std::ofstream* f_dnageo, const int& method, const int& intEllipsoidtoOrtho, const int& intDmsFlag)
{
	m_fileMode = true;

	// calculate file length
	f_in->seekg(0, ios::end);		// set file pointer to end
	streamoff lFileLen = f_in->tellg();
	f_in->seekg(0, ios::beg);		// reset file pointer to beginning

	char cBuf[MAX_RECORD_LENGTH];
	geoid_point apPt;
	bool iSuccess;
	bool bheight_data;
	string strBuf, station;
	
	double original_height;

	// Print header
	*f_out << left << setw(11) << "Point" <<
		setw(16) << right << "Latitude" <<
		setw(16) << right << "Longitude" <<
		setw(9) << right << "Orig. Ht";
	if (intEllipsoidtoOrtho == 1)
		*f_out << setw(9) << right << trimstr(string(m_pGridfile->chSystem_t));		// "New Ht" 
	else
		*f_out << setw(9) << right << trimstr(string(m_pGridfile->chSystem_f));		// "New Ht" 
	*f_out << setw(9) << right << "N value" <<
		setw(9) << right << "D.Merid" <<
		setw(9) << right << "D.PrimeV" << endl;

	do
	{
		try {
			f_in->getline(cBuf, MAX_RECORD_LENGTH);
		}
		catch (...) {
			if (f_in->eof())
				return;
		}

		m_iBytesRead = (int)f_in->tellg();
		m_dPercentComplete = (double)m_iBytesRead / (double)lFileLen * 100.0;

		strBuf = cBuf;

		// Is the line empty or comprised of only blank spaces?
		if (trimstr(strBuf).empty())
			continue;

		// Is the line too short to contain valid data?
		if (strBuf.length() < 31)		// 31 = 11 + 16 + 4 (4 is the minimum number of
		{								//                   units required for longitude)
			// print dat
			*f_out << left << "ERROR (" << ERR_LINE_TOO_SHORT << "): " <<
				ErrorString(ERR_LINE_TOO_SHORT, strBuf) << endl;
			continue;					
		}
		
		stringstream ssInput;
		ssInput << trimstr(strBuf.substr(11, 16)) << ", " 
			<< trimstr(strBuf.substr(27, 16));
		SetInputCoordinates(ssInput.str());

		// capture latitude 
		if (intDmsFlag == DMS)
			apPt.cVar.dLatitude = FromDmsString<double>(trimstr(strBuf.substr(11, 16)));
		else
			apPt.cVar.dLatitude = DoubleFromString<double>(trimstr(strBuf.substr(11, 16)));		

		// capture longitude
		// are there characters after the permitted longitude field?
		if (strBuf.length() > 43)
		{
			bheight_data = true;
			if (intDmsFlag == DMS)
				apPt.cVar.dLongitude = FromDmsString<double>(trimstr(strBuf.substr(27, 16)));
			else
				apPt.cVar.dLongitude = DoubleFromString<double>(trimstr(strBuf.substr(27, 16)));
				
		}
		else
		{
			bheight_data = false;
			if (intDmsFlag == DMS)
				apPt.cVar.dLongitude = FromDmsString<double>(trimstr(strBuf.substr(27)));
			else
				apPt.cVar.dLongitude = DoubleFromString<double>(trimstr(strBuf.substr(27)));
			apPt.cVar.dHeight = 0.;
		}

		// capture height
		// are there characters after the permitted height field?
		if (bheight_data)
		{
			string height_str;
			if (strBuf.length() > 52)
				height_str = trimstr(strBuf.substr(43, 9));
			else
				height_str = trimstr(strBuf.substr(43));

			if (height_str.empty())
				bheight_data = false;
			else
				apPt.cVar.dHeight = DoubleFromString<double>(height_str);
		}
		
		apPt.cVar.IO_Status = ERR_TRANS_SUCCESS;

		original_height = apPt.cVar.dHeight;
		
		iSuccess = false;
		try {
			ApplyAusGeoidGrid(&apPt, method);
			iSuccess = true;
		}
		catch (const NetGeoidException& e) {
			stringstream s(e.what());
			iSuccess = false;
		}

		if (iSuccess)
		{	
			++m_pointsInterpolated;

			if (intEllipsoidtoOrtho == 1)
				apPt.cVar.dHeight -= apPt.gVar.dN_value;		// H = h - N
			else
				apPt.cVar.dHeight += apPt.gVar.dN_value;		// h = H + N
		}
		else
		{
			++m_pointsNotInterpolated;
			apPt.cVar.dHeight = -UNRELIABLE;

			// Format error message using input coordinates
			stringstream ss;
			ss << trimstr(strBuf.substr(11, 16)) << ", " << trimstr(strBuf.substr(27, 16));

			// print dat
			*f_out << left << "ERROR (" << apPt.cVar.IO_Status << "): " <<
				ErrorString(apPt.cVar.IO_Status, ss.str()) << endl;
			continue;
		}

		if (intDmsFlag == DMS)
		{
			apPt.cVar.dLatitude = DegtoDms<double>(apPt.cVar.dLatitude);
			apPt.cVar.dLongitude = DegtoDms<double>(apPt.cVar.dLongitude);
		}

		station = string(cBuf).substr(0, 11);
		// print dat
		*f_out << left << setw(11) << station <<
			setw(16) << setprecision(9) << fixed << right << apPt.cVar.dLatitude <<
			setw(16) << setprecision(9) << fixed << right << apPt.cVar.dLongitude;
		
		if (bheight_data)
		{
			// A height was supplied...
			// Print the 'original' height
			*f_out << setw(9) << setprecision(3) << fixed << right << original_height;
			// Print the 'transformed' height
			*f_out << setw(9) << setprecision(3) << fixed << right << apPt.cVar.dHeight;
		}
		else
		{
			// no height was supplied, so print blanks for original height and derived heights
			*f_out << setw(9) << right << " ";
			*f_out << setw(9) << right << " ";
		}
		
		// As per Nick Brown's request, print the N value regardless of input height
		*f_out << setw(9) << setprecision(3) << fixed << right << apPt.gVar.dN_value;

		// Print the deflection values
		*f_out << setw(9) << setprecision(3) << fixed << right << apPt.gVar.dDefl_meridian <<
			setw(9) << setprecision(3) << fixed << right << apPt.gVar.dDefl_primev << endl;

		// write to dna geo file
		if (m_exportDNAGeoidFile)
			PrintDNA1GeoidRecord(*f_dnageo, 
				station, apPt.gVar.dN_value, apPt.gVar.dDefl_meridian, apPt.gVar.dDefl_primev);
	}
	while (!f_in->eof());			// while EOF not found
}
	

void dna_geoid_interpolation::FileTransformation(const char* fileIn, const char* fileOut, const int& method, 
	const int& intEllipsoidtoOrtho, const int& intDmsFlag, bool exportDnaGeoidFile, const char* geoFile)
{
	if (!m_pGridfile)
		throw NetGeoidException(ErrorString(ERR_GRID_UNOPENED), ERR_GRID_UNOPENED);
	
	SetByteOffset();

	m_pointsInterpolated = m_pointsNotInterpolated = 0;
	
	//const char* ext = strrchr(trimstr<string>(fileIn).c_str(), '.');
	const char* ext = strrchr(fileIn, '.');
	int iF_inType(TYPE_ASC);
	
	if (ext != NULL)
		iF_inType = DetermineFileType(ext+1);
	
	if (iF_inType < 0)
		throw NetGeoidException(ErrorString(ERR_INFILE_TYPE), ERR_INFILE_TYPE);

	std::ifstream f_in;
	try {
		// open input file.  Throws runtime_error on failure.
		file_opener(f_in, trimstr<string>(fileIn).c_str(), ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_INFILE_READ) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_INFILE_READ);
	}

	std::ofstream f_out;
	try {
		// Create input file.  Throws runtime_error on failure.
		file_opener(f_out, trimstr<string>(fileOut).c_str());
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}

	m_exportDNAGeoidFile = exportDnaGeoidFile;

	// Export data to DNA geoid file?
	std::ofstream f_dnageo;
	if (m_exportDNAGeoidFile)
	{
		string geofileOut;
		if (geoFile != NULL)
			geofileOut = geoFile;
		else
		{
			geofileOut = trimstr<string>(fileOut);

			string searchstr(ext);
			string replacestr(".geo");
		
			size_t pos = 0;

			if ((pos = geofileOut.find(searchstr, pos)) != string::npos)
				geofileOut.replace(pos, replacestr.size(), replacestr);
			else
				geofileOut += ".geo";
		}
		
		try {
			// Create geoid file.  Throws runtime_error on failure.
			file_opener(f_dnageo, geofileOut);
		}
		catch (const runtime_error& e) {
			ClearGridFileMemory();
			stringstream ss;
			ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
				"  " << e.what();
			throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
		}
	}

	// As per Nick Brown's request, print Grid file version to file
	f_out << "Derived ";
	if (intEllipsoidtoOrtho == 1)
		f_out << trimstr(string(m_pGridfile->chSystem_t));
	else
		f_out << trimstr(string(m_pGridfile->chSystem_f));
	f_out << " values obtained from " << left << m_pGridfile->filename << ", Version " << trimstr(string(m_pGridfile->chVersion)) << endl;

	switch (iF_inType)
	{
		case TYPE_DAT:		// DAT
			ProcessDatFile(&f_in, &f_out, &f_dnageo, method, intEllipsoidtoOrtho, intDmsFlag);
			break;
		case TYPE_CSV:		// CSV
			ProcessCsvFile(&f_in, &f_out, &f_dnageo, method, intEllipsoidtoOrtho, intDmsFlag);
			break;
		default:
			throw NetGeoidException(ErrorString(ERR_INFILE_TYPE), ERR_INFILE_TYPE);
	}

	f_in.close();
	f_out.close();
	if (m_exportDNAGeoidFile)
		f_dnageo.close();
}
	
void dna_geoid_interpolation::PopulateBinaryStationFile(const string& bstnFile, const int& method, 
	bool convertHeights, bool exportDnaGeoidFile, const char* geoFile)
{
	if (!m_pGridfile)
		throw NetGeoidException(ErrorString(ERR_GRID_UNOPENED), ERR_GRID_UNOPENED);

	SetByteOffset();
	
	LoadBinaryStationFile(bstnFile);
	
	PopulateStationRecords(method, convertHeights);

	WriteBinaryStationFile(bstnFile);

	if (exportDnaGeoidFile)
	{
		if (geoFile == NULL)
		{
			string geofileOut(bstnFile);

			// Export data to DNA geoid file?
			string searchstr(".bst"), replacestr(".geo");
			size_t pos = 0;

			if ((pos = geofileOut.find(searchstr, pos)) != string::npos)
				geofileOut.replace(pos, replacestr.size(), replacestr);
			else
				geofileOut += ".geo";
	
			WriteDNA1GeoidFile(geofileOut);
		}
		else
			WriteDNA1GeoidFile(geoFile);
	}
}

void dna_geoid_interpolation::LoadBinaryStationFile(const string& bstnfileName)
{
	try {
		// Load binary stations data.  Throws runtime_error on failure.
		dna_io_bst bst;
		bst.load_bst_file(bstnfileName, &bstBinaryRecords_, bst_meta_);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_INFILE_READ) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_INFILE_READ);
	}
}


void dna_geoid_interpolation::WriteBinaryStationFile(const string& bstnfileName)
{
	sprintf(bst_meta_.modifiedBy, "%s", __BINARY_NAME__);
	
	try {
		// write binary stations data.  Throws runtime_error on failure.
		dna_io_bst bst;
		bst.write_bst_file(bstnfileName, &bstBinaryRecords_, bst_meta_);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << endl <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}
}


void dna_geoid_interpolation::WriteDNA1GeoidFile(const string& geofileName)
{
	std::ofstream f_dnageo;
	try {
		// Create geoid file.  Throws runtime_error on failure.
		file_opener(f_dnageo, geofileName);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}

	// write to dna geo file
	try {
		// Write header line
		dnaproj_header(f_dnageo, "DNA geoid file");

		f_dnageo << endl;

		it_vstn_t _it_stn(bstBinaryRecords_.begin());
		
		for (; _it_stn!=bstBinaryRecords_.end(); ++_it_stn)
		{
			// Were values interpolated?
			if (fabs(_it_stn->geoidSep) <= PRECISION_1E4)
				continue;
			if (fabs(_it_stn->verticalDef) <= E4_SEC_DEFLECTION)
				continue;
			if (fabs(_it_stn->meridianDef) <= E4_SEC_DEFLECTION)
				continue;

			// Print data to geoid file
			PrintDNA1GeoidRecord(f_dnageo, 
				_it_stn->stationName, _it_stn->geoidSep, 
				Seconds(_it_stn->meridianDef), 
				Seconds(_it_stn->verticalDef));
		}
	}
	catch (const ios_base::failure& f) {
		stringstream ss;
		ss << "WriteDNA1GeoidFile(): An error was encountered when writing to  " << geofileName << "." << endl << f.what();
		throw NetGeoidException(ss.str(), ERR_INFILE_READ);
	}

	// close
	f_dnageo.close();
}
	

void dna_geoid_interpolation::PrintDNA1GeoidRecord(std::ofstream& f_out, const string& station, const double& nValue, const double& meridianDef, const double& verticalDef)
{
	f_out << setw(41) << left << station <<
		setw(9) << right << fixed << setprecision(3) << nValue <<
		setw(19) << right << setprecision(3) << meridianDef <<
		setw(10) << right << setprecision(3) << verticalDef << endl;
}
	

/////////////////////////////////////////////////////////////////////////
// ClearGridFileMemory: Closes an open grid file pointer and deletes memory 
//				created for sub grid header struct and array.
/////////////////////////////////////////////////////////////////////////
// On Entry:	A valid file pointer, and memory for the sub grid header
//				struct and array may or may not exist.
// On Exit:		An open grid file pointer is closed and any memory 
//				created for sub grid header struct and array is deleted.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::ClearGridFileMemory()
{
	if (m_pGfileptr)	// is file pointer to distortion grid file open?
	{
		if (m_pGfileptr.is_open())
			m_pGfileptr.close();
	}

	if (m_pGridfile)	// check for existence of allcoated memory for n_file_par struct
	{
		if (m_pGridfile->ptrIndex)		// check for existence of allcoated memory for n_gridfileindex struct
		{
			delete[] m_pGridfile->ptrIndex;
			m_pGridfile->ptrIndex = 0;
		}

		delete m_pGridfile;
		m_pGridfile = 0;
	}
}
		

/////////////////////////////////////////////////////////////////////////
// BiLinearTransformation: External wrapper for ApplyAusGeoidGrid
/////////////////////////////////////////////////////////////////////////
// On Entry:  A pointer to the grid file is held by m_pGfileptr. The coordinates
//			  for the point to be transformed are passed by apPoint. These values
//			  must be geographic coordinates in decimal degrees. The pointer to
//			  grid file must point to a valid file before this procedure is called.
// On Exit:   The transformed point is returned by cThePoint.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::BiLinearTransformation(geoid_point* apPoint)
{
	m_fileMode = false;

	// initialise success
	apPoint->cVar.IO_Status = ERR_TRANS_SUCCESS;
	if (m_pGridfile != NULL)
		ApplyAusGeoidGrid(apPoint, 0);	// throws on failure
	else
		throw NetGeoidException(ErrorString(ERR_GRID_UNOPENED), ERR_GRID_UNOPENED);

	if (apPoint->cVar.iHeightSystem == 1)
	{
		apPoint->cVar.dHeight -= apPoint->gVar.dN_value;
		apPoint->cVar.iHeightSystem = 0;
	}
	else
	{
		apPoint->cVar.dHeight += apPoint->gVar.dN_value;
		apPoint->cVar.iHeightSystem = 1;
	}
}
	

/////////////////////////////////////////////////////////////////////////
// BiCubuicTransformation: External wrapper for ApplyAusGeoidGrid
/////////////////////////////////////////////////////////////////////////
// On Entry:  A pointer to the grid file is held by m_pGfileptr. The coordinates
//			  for the point to be transformed are passed by apPoint. These values
//			  must be geographic coordinates in decimal degrees. The pointer to
//			  grid file must point to a valid file before this procedure is called.
// On Exit:   The transformed point is returned by cThePoint.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::BiCubicTransformation(geoid_point* apPoint)
{
	m_fileMode = false;

	// initialise success
	apPoint->cVar.IO_Status = ERR_TRANS_SUCCESS;
	if (m_pGridfile != NULL)		// has grid file been opened?
		ApplyAusGeoidGrid(apPoint, 1);
	else
		apPoint->cVar.IO_Status = ERR_GRID_UNOPENED;
	
	if (apPoint->cVar.iHeightSystem == 1)
	{
		apPoint->cVar.dHeight -= apPoint->gVar.dN_value;
		apPoint->cVar.iHeightSystem = 0;
	}
	else
	{
		apPoint->cVar.dHeight += apPoint->gVar.dN_value;
		apPoint->cVar.iHeightSystem = 1;
	}
}
	

/////////////////////////////////////////////////////////////////////////
// CreateGridIndex: External wrapper for Opengridfile
/////////////////////////////////////////////////////////////////////////
// On Entry:  The grid file name and file type is held by filename and filetype.
// On Exit:   The grid file is opened and all header information is read 
//			  and returned by fGridFile. If the grid file does not meet the
//			  standard NTv2 grid file format, or contains values that are un-
//			  recognised, OpenGridFile returns fGridfile with a values
//			  corresponding to the nature of the error and its file location.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::CreateGridIndex(const char* fileName, const char* fileType)
{
	// grid file not opened?
	if (m_pGridfile == NULL)		
	{
		if ((m_Grid_Success = OpenGridFile(fileName, fileType, 
			m_pGridfile, false)) != ERR_TRANS_SUCCESS)	// an error occurred?
		{
			ClearGridFileMemory();
			throw NetGeoidException(ErrorString(m_Grid_Success, fileName), m_Grid_Success);
		}
	}
	// a new filename or filetype?
	else if (!iequals(m_pGridfile->filename, fileName))
	{
		ClearGridFileMemory();
		if ((m_Grid_Success = OpenGridFile(fileName, fileType, 
			m_pGridfile, false)) != ERR_TRANS_SUCCESS)	// an error occurred?
		{
			ClearGridFileMemory();
			throw NetGeoidException(ErrorString(m_Grid_Success, fileName), m_Grid_Success);
		}
	}

	// Validate the grid file
	int filePos(0), i;
	double dNum;
	char cBuf[DATA_RECORD];
	char ident_rec[9];

	int gridType = DetermineFileType(m_pGridfile->filetype);

	try {

		// skip through the sub grids, reading either the end of file record or a sub grid
		// The following loop throws on any failure to read
		for (i=0; i<m_pGridfile->iNumsubgrids; ++i)
		{
			if (gridType == TYPE_ASC)		// ascii
			{
				filePos = m_pGridfile->ptrIndex[i].iGridPos + m_pGridfile->ptrIndex[i].lGscount * 42;
				m_pGfileptr.seekg(filePos);
				m_pGfileptr.getline(cBuf, DATA_RECORD);
			}
			else							// binary
			{
				filePos = m_pGridfile->ptrIndex[i].iGridPos + (m_pGridfile->ptrIndex[i].lGscount * 4 * sizeof(float));
				m_pGfileptr.seekg(filePos);

				// read the record tag
				m_pGfileptr.read(reinterpret_cast<char *>(ident_rec), IDENT_BUF);

				if (i == m_pGridfile->iNumsubgrids-1)
				{
					if (strncmp("END     ", ident_rec, IDENT_BUF) != 0)
						throw NetGeoidException(ErrorString(ERR_GRID_CORRUPT, m_pGridfile->filename), ERR_GRID_CORRUPT);
					// last subgrid - read the EOF value, being (double) 3.33E+32
					m_pGfileptr.read(reinterpret_cast<char *>(&dNum), sizeof(double));
				}
				else
					// not the last subgrid - read the SUB_NAME
					m_pGfileptr.read(reinterpret_cast<char *>(ident_rec), IDENT_BUF);
			}
		}
	}
	catch (...) {
		throw NetGeoidException(ErrorString(ERR_GRID_CORRUPT, m_pGridfile->filename), ERR_GRID_CORRUPT);
	}
}
	

/////////////////////////////////////////////////////////////////////////
// ReportGridVersion: Retrieves the current grid file's version.
/////////////////////////////////////////////////////////////////////////
// On Entry: The full file path of the grid file and its type is passed with a 
//			 pointer to a file_par struct.
// On Exit:  The file is opened and stored in file_par struct.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::ReportGridVersion(char* version)
{
	if (m_pGridfile == NULL)
		strcpy(version, "None");
	else
		strcpy(version, m_pGridfile->chVersion);
}


/////////////////////////////////////////////////////////////////////////
// ReportGridProperties: Retrieves the fundamental parameters from a grid file.
/////////////////////////////////////////////////////////////////////////
// On Entry: The full file path of the grid file and its type is passed with a 
//			 pointer to a file_par struct.
// On Exit:  The file is opened and stored in file_par struct.
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::ReportGridProperties(const char* fileName, const char* fileType, n_file_par* pGridfile)
{
	if ((m_Grid_Success = OpenGridFile(fileName, fileType, pGridfile)) != ERR_TRANS_SUCCESS)	// an error occurred?
		throw NetGeoidException(ErrorString(m_Grid_Success), m_Grid_Success);
}


// use a temporary n_file_par object, therefore retain the existing grid file in memory
void dna_geoid_interpolation::CreateNTv2File(const char* datFile, const n_file_par* grid)
{	
	m_dPercentComplete = 1.0;
	m_iBytesRead = 0;

	// temporary grid file object
	n_file_par GridfileTmp(*grid);

	// Grid files created from AusGeoid dat files are created with only 1 sub-grid
	if (!GridfileTmp.ptrIndex)
		GridfileTmp.ptrIndex = new n_gridfileindex[1];

	std::ifstream f_in;
	try {
		// open dat file.  Throws runtime_error on failure.
		file_opener(f_in, datFile, ios::in, ascii, true);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_INFILE_READ) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_INFILE_READ);
	}

	std::ofstream f_out;
	try {
		// open binary gsb output.  Throws runtime_error on failure.
		file_opener(f_out, grid->filename, ios::out | ios::binary, binary, false);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}

	// calculate file length
	f_in.seekg(0, ios::end);		// set file pointer to end
	streamoff lFileLen = f_in.tellg();
	f_in.seekg(0, ios::beg);		// reset file pointer to beginning
	
	char szLine[MAX_RECORD_LENGTH];

	int lat_deg, lat_min, lon_deg, lon_min;
	float n_value, lat_sec, lon_sec, dDefl_meridian, dDefl_primev;
	char c_northsouth, c_eastwest;
	UINT32 lNodeCount = 0, lNodeRead = 0;

	// Read the first line and test for a comment line
	f_in.getline(szLine, MAX_RECORD_LENGTH);

	if (strncmp(szLine, "GEO", 3) == 0)
		f_in.seekg(0, ios::beg);		// put file pointer back to beginning

	// AusGeoid file prints upper lat to lower lat.  NTv2 file format prints lower lat to upper lat
	// So, read data to the end of the file to determine grid file limits and record count, then
	// store data in array.  Use this loop to determine limits and grid node increments
	double lat, lon, min_lat(90.), max_lat(-90.), min_lon(180.), max_lon(-180.);
	double init_lat, lat_inc, lon_inc;

	cout << endl << "+ Reading contents of WINTER DAT file... ";

	m_isReading = true;
	m_isWriting = false;

	try {
		while (!f_in.eof())
		{
			f_in.getline(szLine, MAX_RECORD_LENGTH);
			
			m_iBytesRead = (int)f_in.tellg();
			m_dPercentComplete = (double)(m_iBytesRead) / (double)lFileLen * 100.0;
			
			if (strlen(szLine) < 10)
				continue;

			// determine limits
			ScanNodeLocations(szLine, &lat, &lon, lNodeRead);
			if (lat < min_lat)
				min_lat = lat;
			if (lat > max_lat)
				max_lat = lat;
			if (lon < min_lon)
				min_lon = lon;
			if (lon > max_lon)
				max_lon = lon;

			// calculate latitude separation
			if (lNodeRead == 1)
				lon_inc = fabs(lon - lon_inc);

			if (lNodeRead > 1)
			{
				if (fabs(lat - init_lat) > PRECISION_1E2)
				{
					lat_inc = fabs(lat - init_lat);
					init_lat = lat;
				}
			}

			if (++lNodeRead == 1)
			{
				// initialise separations
				init_lat = lat_inc = lat;
				lon_inc = lon;
			}		
		}
	}
	catch (const ios_base::failure& f) {
		if (!f_in.eof())
			throw NetGeoidException(ErrorString(ERR_GRIDFILE_READ) + "\n " + f.what(), ERR_GRIDFILE_READ);
	}

	// update with relevant details from DAT file
	GridfileTmp.ptrIndex[0].dSlat = min_lat * DEG_TO_SEC;
	GridfileTmp.ptrIndex[0].dNlat = max_lat * DEG_TO_SEC;
	GridfileTmp.ptrIndex[0].dElong = min_lon * DEG_TO_SEC;
	GridfileTmp.ptrIndex[0].dWlong = max_lon * DEG_TO_SEC;
	GridfileTmp.ptrIndex[0].dLatinc = lat_inc * DEG_TO_SEC;
	GridfileTmp.ptrIndex[0].dLonginc = lon_inc * DEG_TO_SEC;

	double width(fabs(GridfileTmp.ptrIndex[0].dWlong - GridfileTmp.ptrIndex[0].dElong));
	double height(fabs(GridfileTmp.ptrIndex[0].dNlat - GridfileTmp.ptrIndex[0].dSlat));
		
	// check there are sufficient nodes for the grid parameters
	long lExpectedLatIncrements = long(floor(height / GridfileTmp.ptrIndex[0].dLatinc + PRECISION_1E5)) + 1;
	long lExpectedLonIncrements = long(floor(width / GridfileTmp.ptrIndex[0].dLonginc + PRECISION_1E5)) + 1;
	long lExpectedNodes = (lExpectedLatIncrements * lExpectedLonIncrements);

	if (lExpectedNodes != lNodeRead)
	{
		stringstream ss;
		ss << endl << "Expected number of rows is  " << lExpectedLatIncrements << endl;
		ss << endl << "Expected number of cols is  " << lExpectedLonIncrements << endl;
		ss << endl << "Expected number of nodes is " << lExpectedNodes << ", but read " << lNodeRead;
		throw NetGeoidException(ErrorString(ERR_GRID_PARAMETERS) + ss.str(), ERR_GRID_PARAMETERS);
	}

	m_iBytesRead = 0;

	cout << "done." << endl << "+ WINTER DAT file structure appears OK." << endl;
	cout <<	"+ Creating NTv2 gsb file... ";
	geoid_values* ag_data = new geoid_values[lNodeRead];

	// Update header record
	GridfileTmp.ptrIndex[0].lGscount = lNodeRead;

	////////////////////////////////////////
	// Now, write to the new binary file
	//

	// Print default header info.
	PrintDefaultGridHeaderInfo(&f_out, &GridfileTmp);
	PrintDefaultSubGridHeaderInfo(&f_out, GridfileTmp.ptrIndex, grid->chGs_type);
	
	// put file pointer back to beginning
	f_in.clear();
	f_in.seekg(0, ios::beg);

	// Read the first line and test for a comment line
	f_in.getline(szLine, MAX_RECORD_LENGTH);
	if (strncmp(szLine, "GEO", 3) == 0)
		f_in.seekg(0, ios::beg);		// put file pointer back to beginning

	// read first line of dat file and scan in values
	f_in.getline(szLine, MAX_RECORD_LENGTH);

	lNodeCount = 0;

	try {
		while (!f_in.eof())
		{
			m_iBytesRead = (int)f_in.tellg();
			m_dPercentComplete = (double)(m_iBytesRead) / (double)lFileLen * 100.0;
		
			// extract data:
			// n value (metres)
			// deflection in prime meridian (seconds)
			// deflection in prime vertical (seconds)
			ScanDatFileValues(szLine, &n_value, &c_northsouth, &lat_deg, &lat_min, &lat_sec, &c_eastwest, &lon_deg, &lon_min, &lon_sec, &dDefl_meridian, &dDefl_primev);

			// Has the user specified radians?
			if (strcmp(grid->chGs_type, "RADIANS ") == 0)
			{
				// convert seconds values to radians
				dDefl_meridian /= (float)RAD_TO_SEC;
				dDefl_primev /= (float)RAD_TO_SEC;
			}

			// write to array
			ag_data[lNodeCount].dN_value = n_value;
			ag_data[lNodeCount].dDefl_meridian = dDefl_meridian;
			ag_data[lNodeCount++].dDefl_primev = dDefl_primev;
			
			// get next line in AusGeoid file
			f_in.getline(szLine, MAX_RECORD_LENGTH);	
		}
	}
	catch (const ios_base::failure& f) {
		if (!f_in.eof())
			throw NetGeoidException(ErrorString(ERR_GRIDFILE_READ) + "\n " + f.what(), ERR_GRIDFILE_READ);
	}

	m_dPercentComplete = 0;

	UINT32 i;
	for (i=0; i<lNodeRead; ++i)
	{
		--lNodeCount;
		WriteBinaryRecords(&f_out, (float)ag_data[lNodeCount].dN_value, (float)ag_data[lNodeCount].dDefl_meridian, (float)ag_data[lNodeCount].dDefl_primev);
		m_dPercentComplete = (double)(lNodeRead - lNodeCount) / (double)lNodeRead * 100.0;
	}

	delete [] ag_data;
	
	// print end of file record
	f_out.write("END     ", OVERVIEW_RECS);
	double dTemp = (double) 3.33E+32;
	f_out.write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
	f_out.close();
	f_in.close();

	cout << "done." << endl;
}


/////////////////////////////////////////////////////////////////////////
// ExportToAscii:  Exports a Binary NTv2 distortion grid to the specified
//			  output file. Any errors that occur are captured and an error
//            value is saved to IO_Status.
/////////////////////////////////////////////////////////////////////////
// On Entry:  File path for Binary distortion grid file 
// On Exit:   An ASCII grid file is created and saved to the specified file path
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::ExportToAscii(char *gridFile, char *gridType, char *OutputGrid, int *IO_Status)
{
	int i, j;
	float fTemp;
	
	*IO_Status = ERR_TRANS_SUCCESS;

	m_dPercentComplete = 0.0;

	// Open grid file
	if (m_pGridfile == NULL)
		m_Grid_Success = OpenGridFile(gridFile, gridType, m_pGridfile, false);

	// A new grid file specified?
	if (!iequals(m_pGridfile->filename, gridFile))
	{
		ClearGridFileMemory();
		m_Grid_Success = OpenGridFile(gridFile, gridType, m_pGridfile, false);
	}
	
	*IO_Status = m_Grid_Success;

	// Test for successful opening of grid file
	if (m_Grid_Success > ERR_TRANS_SUCCESS)
	{
		ClearGridFileMemory();
		throw NetGeoidException(ErrorString(m_Grid_Success), m_Grid_Success);
	}

	std::ofstream f_out;
	*IO_Status = ERR_OUTFILE_WRITE;
	try {
		// Create distortion grid file.  Throws runtime_error on failure.
		file_opener(f_out, OutputGrid);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}
	*IO_Status = ERR_TRANS_SUCCESS;
	
	// print header block information
	f_out << "NUM_OREC" << setw(8) << right << m_pGridfile->iH_info << endl;	// Number of header identifiers (NUM_OREC)
	f_out << "NUM_SREC" << setw(8) << right << m_pGridfile->iSubH_info << endl;	// Number of sub-header idents (NUM_SREC)
	f_out << "NUM_FILE" << setw(8) << right << m_pGridfile->iNumsubgrids << endl;	// number of subgrids in file (NUM_FILE)
	f_out << "GS_TYPE " << setw(8) << right << m_pGridfile->chGs_type << endl;	// grid shift type (GS_TYPE)
	f_out << "VERSION " << setw(8) << right << m_pGridfile->chVersion << endl;	// grid file version (VERSION)
	f_out << "SYSTEM_F" << setw(8) << right << m_pGridfile->chSystem_f << endl;	// reference system (SYSTEM_F)
	f_out << "SYSTEM_T" << setw(8) << right << m_pGridfile->chSystem_t << endl;	// reference system (SYSTEM_T)
	f_out << "MAJOR_F " << setw(15) << right << setprecision(3) << m_pGridfile->daf << endl;	// semi major of from system (MAJOR_F)
	f_out << "MINOR_F " << setw(15) << right << setprecision(3) << m_pGridfile->dbf << endl;	// semi minor of from system (MINOR_F)
	f_out << "MAJOR_T " << setw(15) << right << setprecision(3) << m_pGridfile->dat << endl;	// semi major of to system (MAJOR_T)
	f_out << "MINOR_T " << setw(15) << right << setprecision(3) << m_pGridfile->dbt << endl;	// semi minor of to system (MINOR_T)
	
	// loop through number of sub grids
	for (i=0; i<m_pGridfile->iNumsubgrids; i++)
	{
		f_out << "SUB_NAME" << setw(8) << right << m_pGridfile->ptrIndex[i].chSubname << endl;
		f_out << "PARENT  " << setw(8) << right << m_pGridfile->ptrIndex[i].chParent << endl;
		f_out << "CREATED " << setw(8) << right << m_pGridfile->ptrIndex[i].chCreated << endl;
		f_out << "UPDATED " << setw(8) << right << m_pGridfile->ptrIndex[i].chUpdated << endl;

		if (strcmp(m_pGridfile->chGs_type, "RADIANS ") == 0)
		{
			// convert all seconds values to radians
			f_out << "S_LAT   " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dSlat / RAD_TO_SEC << endl;
			f_out << "N_LAT   " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dNlat / RAD_TO_SEC << endl;
			f_out << "E_LONG  " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dElong / RAD_TO_SEC << endl;
			f_out << "W_LONG  " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dWlong / RAD_TO_SEC << endl;
			f_out << "LAT_INC " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dLatinc / RAD_TO_SEC << endl;
			f_out << "LONG_INC" << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dLonginc / RAD_TO_SEC << endl;			
		}
		else
		{
			f_out << "S_LAT   " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dSlat << endl;
			f_out << "N_LAT   " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dNlat << endl;
			f_out << "E_LONG  " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dElong << endl;
			f_out << "W_LONG  " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dWlong << endl;
			f_out << "LAT_INC " << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dLatinc << endl;
			f_out << "LONG_INC" << setw(15) << right << setprecision(6) << m_pGridfile->ptrIndex[i].dLonginc << endl;			
		}

		f_out << "GS_COUNT" << setw(6) << right << m_pGridfile->ptrIndex[i].lGscount << endl;
		
		// set file pointer to file position held by ptrIndex[i].fpGridPos
		m_pGfileptr.seekg(m_pGridfile->ptrIndex[i].iGridPos);

		try {
			// read / write all coordinate shifts in this sub file
			for (j=0; j<m_pGridfile->ptrIndex[i].lGscount; j++)
			{
				// Latitude shift
				m_pGfileptr.read(reinterpret_cast<char *>(&fTemp), sizeof(float));
				f_out << setw(10) << setprecision(6) << fTemp;			

				// Longitude shift
				m_pGfileptr.read(reinterpret_cast<char *>(&fTemp), sizeof(float));
				f_out << setw(10) << setprecision(6) << fTemp;			

				// Latitude accuracy
				m_pGfileptr.read(reinterpret_cast<char *>(&fTemp), sizeof(float));
				f_out << setw(10) << setprecision(6) << fTemp;			
				
				// Longitude accuracy
				m_pGfileptr.read(reinterpret_cast<char *>(&fTemp), sizeof(float));
				f_out << setw(10) << setprecision(6) << fTemp;

				m_dPercentComplete = (double)(m_pGfileptr.tellg()) / (double)m_pGridfile->iGfilelength * 100.0;	
			}
		}
		catch (...) {
			*IO_Status = ERR_READ_BIN_SHIFT;
			f_out.close();		
			ClearGridFileMemory();
			throw NetGeoidException(ErrorString(ERR_READ_BIN_SHIFT), ERR_READ_BIN_SHIFT);		
		}
	}

	m_dPercentComplete = 0.0;

	// print end of file record
	f_out << "END     " << setw(10) << right << setprecision(2) << scientific << 3.33E+32 << endl;

	// Finished!
	f_out.close();
}


/////////////////////////////////////////////////////////////////////////
// ExportToBinary:  Exports an ASCII NTv2 distortion grid to the specified
//			  output file. Any errors that occur are captured and an error
//            value is saved to IO_Status.
/////////////////////////////////////////////////////////////////////////
// On Entry:  File path for ASCII distortion grid file 
// On Exit:   A Binary grid file is created and saved to the specified file path
/////////////////////////////////////////////////////////////////////////
void dna_geoid_interpolation::ExportToBinary(char *gridFile, char *gridType, char *OutputGrid, int *IO_Status)
{
	*IO_Status = ERR_TRANS_SUCCESS;
	m_dPercentComplete = 0.0;

	// Open grid file
	if (m_pGridfile == NULL)
		m_Grid_Success = OpenGridFile(gridFile, gridType, m_pGridfile, false);

	// A new grid file specified?
	if (!iequals(m_pGridfile->filename, gridFile))
	{
		ClearGridFileMemory();
		m_Grid_Success = OpenGridFile(gridFile, gridType, m_pGridfile, false);
	}
	
	*IO_Status = m_Grid_Success;

	// Test for successful opening of grid file
	if (m_Grid_Success > ERR_TRANS_SUCCESS)
	{
		ClearGridFileMemory();
		throw NetGeoidException(ErrorString(m_Grid_Success), m_Grid_Success);
	}
	
	std::ofstream f_out;
	*IO_Status = ERR_OUTFILE_WRITE;
	try {
		// open distortion grid file.  Throws runtime_error on failure.
		file_opener(f_out, OutputGrid, ios::out | ios::binary, binary);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_OUTFILE_WRITE) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_OUTFILE_WRITE);
	}
	*IO_Status = ERR_TRANS_SUCCESS;
	
	// print header block information
	PrintDefaultGridHeaderInfo(&f_out, m_pGridfile);

	char cBuf[DATA_RECORD];
	float fGSRecord[5];
	
	// loop through number of sub grids and read / write coordinate shifts
	for (int j, i=0; i<m_pGridfile->iNumsubgrids; ++i)
	{
		// print sub-grid header block information
		PrintDefaultSubGridHeaderInfo(&f_out, &(m_pGridfile->ptrIndex[i]), m_pGridfile->chGs_type);
		
		m_pGfileptr.seekg(m_pGridfile->ptrIndex[i].iGridPos);
		
		try {
			// read / write all coordinate shifts in this sub file
			for (j=0; j<m_pGridfile->ptrIndex[i].lGscount; j++)
			{
				// Retrieve the whole record
				m_pGfileptr.getline(cBuf, DATA_RECORD);

				if (sscanf(cBuf, SHIFTS, &fGSRecord[0], &fGSRecord[1], &fGSRecord[2], &fGSRecord[3]) < 4)
				{
					// Could not retrieve shifts from Ascii file
					*IO_Status = ERR_READ_ASC_SHIFT;
					f_out.close();
					ClearGridFileMemory();
					throw NetGeoidException(ErrorString(*IO_Status), *IO_Status);
				}
				else
				{
					f_out.write(reinterpret_cast<char *>(&fGSRecord[0]), sizeof(float));
					f_out.write(reinterpret_cast<char *>(&fGSRecord[1]), sizeof(float));
					f_out.write(reinterpret_cast<char *>(&fGSRecord[2]), sizeof(float));
					f_out.write(reinterpret_cast<char *>(&fGSRecord[3]), sizeof(float));	// last element (fGSRecord[3]) is zero
				}

				m_dPercentComplete = (double)(m_pGfileptr.tellg()) / (double)m_pGridfile->iGfilelength * 100.0;
			}
		}
		catch (...) {
			*IO_Status = ERR_READ_ASC_SHIFT;
			f_out.close();
			ClearGridFileMemory();
			throw NetGeoidException(ErrorString(*IO_Status), *IO_Status);
		}
	}

	m_dPercentComplete = 0.0;

	// print end of file record
	f_out.write(reinterpret_cast<const char *>("END     "), OVERVIEW_RECS);
	double dTemp = (double) 3.33E+32;
	f_out.write(reinterpret_cast<char *>(&dTemp), sizeof(double));
	f_out.close();
}


/////////////////////////////////////////////////////////////////////////
// FindSubGrid: Finds the most dense (sub) grid for interpolating
//				the lat and long shifts. This procedure succeeds for
//				multiple density grid files when the input point is contained
//				within the limits of the most dense sub grid. Tests are made to
//				ensure that only one sub grid can be selected. 
/////////////////////////////////////////////////////////////////////////
// On Entry:  The header information for the first sub grid file is held by
//			  m_pGridfile. The Input lat and long are stored in seconds.
// On Exit:   If the point is within the limits of the grid file, the most dense grid
//			  is returned. -1 is returned if the point is ourside all (sub) grids.
/////////////////////////////////////////////////////////////////////////
int dna_geoid_interpolation::FindSubGrid()
{	
	double dBuf1, dBuf2, dBuf3, dBuf4;
	char cCurrentGrid[9];
	strcpy(cCurrentGrid, "NONE    \0");		// initial parent grid set to NONE
	int i, iCurrentFlag, iPrevFlag;
	int iGridSuccess, iInSubGrid;
	int iPointInGrid;

	// initialise flags
	m_pGridfile->iTheGrid = ERR_FINDSUBGRID_OUTSIDE;
	m_pGridfile->iPointFlag = 0;
	iPointInGrid = 0;
	iGridSuccess = false;

	while (!iGridSuccess)
	{
		// initialise flags
		iPrevFlag = iInSubGrid = iCurrentFlag = 0;

		// loop through all subgrids in grid file array
		for (i=0; i<m_pGridfile->iNumsubgrids; i++)
		{
			if (strcmp(cCurrentGrid, m_pGridfile->ptrIndex[i].chParent) == 0)
			{
				// This grid has the same parent name.
				// Test whether point lies within the grid's limits
				// Use buffer precision of 1.0e-5 for lat & long to eliminate double & float precision mismatch
				dBuf1 = fabs(m_pGridfile->dLat - m_pGridfile->ptrIndex[i].dNlat);
				if (dBuf1 < PRECISION_1E5)
					dBuf1 = 0.0;
				dBuf3 = fabs(m_pGridfile->dLat - m_pGridfile->ptrIndex[i].dSlat);
				if (dBuf3 < PRECISION_1E5)
					dBuf3 = 0.0;
				dBuf2 = fabs(m_pGridfile->dLong - m_pGridfile->ptrIndex[i].dWlong);
				if (dBuf2 < PRECISION_1E5)
					dBuf2 = 0.0;
				dBuf4 = fabs(m_pGridfile->dLong - m_pGridfile->ptrIndex[i].dElong);
				if (dBuf4 < PRECISION_1E5)
					dBuf4 = 0.0;
					
				if ( ((m_pGridfile->dLat < m_pGridfile->ptrIndex[i].dNlat) || (dBuf1 == 0.0)) &&
					 ((m_pGridfile->dLat > m_pGridfile->ptrIndex[i].dSlat) || (dBuf3 == 0.0)) &&
					 ((m_pGridfile->dLong < m_pGridfile->ptrIndex[i].dWlong) || (dBuf2 == 0.0)) &&
					 ((m_pGridfile->dLong > m_pGridfile->ptrIndex[i].dElong) || (dBuf4 == 0.0)) )
				{
					// The point lies within the grid limits...test whether point lies on upper limits.					
					if (dBuf1 != 0.0 && dBuf2 != 0.0)
						iCurrentFlag = 1;			// The point is within the limits
					if (dBuf1 == 0.0 && dBuf2 != 0.0)
						iCurrentFlag = 2;			// The point is on the upper latitude limit (only)
					if (dBuf1 != 0.0 && dBuf2 == 0.0)
						iCurrentFlag = 3;			// The point is on the upper longitude limit (only)
					if (dBuf1 == 0.0 && dBuf2 == 0.0)
						iCurrentFlag = 4;			// The point is on both upper latitude and longitude limits
						
					if (iInSubGrid == 0)
					{
						// This is the first sub grid found
						iPrevFlag = iCurrentFlag;
						m_pGridfile->iTheGrid = i;
					}
					else if (iCurrentFlag < iPrevFlag)
					{
						// The 2nd or n'th grid found with the same parent that is adjacent to the previous sub grid.
						// Use flags to determine precedence.
						iPrevFlag = iCurrentFlag;
						m_pGridfile->iTheGrid = i;
					}
					else if (iCurrentFlag == iPrevFlag)
					{
						// ERROR...a point cannot be in the same location for two different sub grids
						m_pGridfile->iTheGrid = ERR_FINDSUBGRID_GRIDERR;
					}
					iInSubGrid = 1;
				}
			}
		}
		
		if (iInSubGrid == 0)
			iGridSuccess = true;	// No sub grids were found in the current parent grid name, or that contain the point
		else
		{
			// A sub grid was found to contain the point
			// cCurrentGrid becomes the sub file's SUB_NAME...continue loop
			strcpy(cCurrentGrid, m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].chSubname);
			m_pGridfile->iPointFlag = iPrevFlag;
			iPointInGrid = 1;
		}
	}

	// The point was not in any grid
	if (iPointInGrid == 0)
		m_pGridfile->iTheGrid = ERR_FINDSUBGRID_OUTSIDE;

	// return the index # for the most dense (sub) grid in the grid file array
	return m_pGridfile->iTheGrid;
}

bool dna_geoid_interpolation::IsWithinUpperLatitudeGridInterval(n_gridfileindex* gfIndex)
{
	double dBuf = fabs(m_pGridfile->dLat - gfIndex->dNlat) - gfIndex->dLatinc;
	if (dBuf < PRECISION_1E5)
		dBuf = 0.0;	
	
	// Is the point within the interval?
	// dBuf will be zero if point is on the second last grid limit, so check both.
	if (dBuf < gfIndex->dLatinc || dBuf == 0.0)
		return true;
	else
		return false;
}

bool dna_geoid_interpolation::IsWithinLowerLatitudeGridInterval(n_gridfileindex* gfIndex)
{
	double dBuf = fabs(m_pGridfile->dLat - gfIndex->dSlat) - gfIndex->dLatinc;
	if (dBuf < PRECISION_1E5)
		dBuf = 0.0;
	
	// Is the point within the interval?
	// dBuf will be zero if point is on the second last grid limit, so check both.
	if (dBuf < gfIndex->dLatinc || dBuf == 0.0)
		return true;
	else
		return false;
}

bool dna_geoid_interpolation::IsWithinUpperLongitudeGridInterval(n_gridfileindex* gfIndex)
{
	double dBuf = fabs(m_pGridfile->dLong - gfIndex->dWlong) - gfIndex->dLonginc;
	if (dBuf < PRECISION_1E5)
		dBuf = 0.0;	
	
	// Is the point within the interval?
	// dBuf will be zero if point is on the second last grid limit, so check both.
	if (dBuf < gfIndex->dLonginc || dBuf == 0.0)
		return true;
	else
		return false;
}

bool dna_geoid_interpolation::IsWithinLowerLongitudeGridInterval(n_gridfileindex* gfIndex)
{
	double dBuf = fabs(m_pGridfile->dLong - gfIndex->dElong) - gfIndex->dLonginc;
	if (dBuf < PRECISION_1E5)
		dBuf = 0.0;
	
	// Is the point within the interval?
	// dBuf will be zero if point is on the second last grid limit, so check both.
	if (dBuf < gfIndex->dLonginc || dBuf == 0.0)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////
// DetermineFileType: Determines the distortion grid's file type from a char array.
//
/////////////////////////////////////////////////////////////////////////
// On Entry:   The grid file type is passed by cType.
// On Exit:    The grid file type is compared against "asc" or "gsb".
//			   An integer is returned indicating the filetype.
/////////////////////////////////////////////////////////////////////////
int dna_geoid_interpolation::DetermineFileType(const char *cType)
{
	// case insensitive
	if (iequals(cType, ASC))		// asc "ASCII" file
		return TYPE_ASC;			
	else if (iequals(cType, GSB))	// gsb "Binary" file
		return TYPE_GSB;			
	else if (iequals(cType, TXT) ||	// dat/txt/prn file
			 iequals(cType, DAT) ||	// ..
			 iequals(cType, PRN))	// ..
		return TYPE_DAT;
	else if (iequals(cType, CSV))	// csv file
		return TYPE_CSV;
	else
		return -1;					// Unsupported filetype
}

int dna_geoid_interpolation::InterpolateNvalue_BiLinear(double dlat, double dlon, geoid_point *dInterpPoint)
{
	double dLatA, dLongA, dX, dY, da0, da1, da2, da3;
	long iA, iB, iC, iD, iRow_Index, iColumn_Index, iGrid_Points_Per_Row;
	int iFtype;

	// declare a coordinate struct for the four values at each node
	geoid_values cNodeShifts[5];
	geoid_values *pNShifts[5];

	int i;

	// set up pointers
	for (i=0; i<4; i++)
		pNShifts[i] = &(cNodeShifts[i]);

	////////////  1. Retrieve Grid Shifts  (see Page 18 of the NTv2 Developer's Guide) ////////////
	// Compute Row_Index of Grid Node A...
	iRow_Index = (int)((m_pGridfile->dLat - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dSlat + PRECISION_1E5) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc);

	// Compute Column_Index of Grid Node A...
	iColumn_Index = (int)((m_pGridfile->dLong - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + PRECISION_1E5) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);

	// Compute Grid_Points_Per_Row (No of Columns)
	iGrid_Points_Per_Row = (int)((m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dWlong - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + PRECISION_1E5) /
		m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);
	iGrid_Points_Per_Row++;

	// Compute Record Index for nodes A, B, C and D.
	iA = (iRow_Index * iGrid_Points_Per_Row) + iColumn_Index + 1;			// Lower Right node
	iB = iA + 1;															// Lower Left node
	iC = iA + iGrid_Points_Per_Row;											// Upper Right node
	iD = iC + 1;															// Upper Left node 

	// Assign the correct grid node number depending on limit flag set in FindSubGrid
	// (see Page 19 & 20 of the NTv2 Developer's Guide)
	switch (m_pGridfile->iPointFlag)
	{
	case 1:
		// point is within the limits...no virtual cell is required
		break;
	case 2:
		// point is on the upper latitude limit
		iC = iA;				// Upper Right node becomes Lower Right node
		iD = iB;				// Upper Left node becomes Lower Left node
		break;
	case 3:
		// point is on the upper longitude limit
		iB = iA;				// Lower Left node becomes Lower Right node
		iD = iC;				// Upper Left node becomes Upper Right node
		break;
	case 4:
		// point is on both upper latitude and upper longitude limits
		iB = iA;				// Lower Left node becomes Lower Right node
		iC = iA;				// Upper Right node becomes Lower Right node
		iD = iA;				// Upper Left node becomes Lower Right node
		break;
	}	

	// determine grid file type...ASCII = 0; Binary = 1.
	iFtype = DetermineFileType(m_pGridfile->filetype);
	if (iFtype < 0 || iFtype > 1)
		return ERR_GRIDFILE_TYPE;

	// initialise pNShifts index
	i = 0;

	// Retrieve the values at the above four grid nodes.
	if (iFtype == TYPE_ASC)
	{	
		if (!(ReadAsciiShifts(pNShifts, i, iA)))			// Node A
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i+1, iB)))			// Node B
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file
	}
	else if (iFtype == TYPE_GSB)
	{
		if (!(ReadBinaryShifts(pNShifts, i, iA)))			// Node A
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i+1, iB)))			// Node B
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file
	}

	i += 2;

	if (iFtype == TYPE_ASC)
	{	
		if (!(ReadAsciiShifts(pNShifts, i, iC)))			// Node C
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i+1, iD)))			// Node D
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file
	}
	else if (iFtype == TYPE_GSB)
	{
		if (!(ReadBinaryShifts(pNShifts, i, iC)))			// Node C
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i+1, iD)))			// Node D
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file
	}
	
	////////////  2. Interpolate Grid Shifts  (see Page 21 of the NTv2 Developer's Guide) ////////////
	// Compute Coordinates of Grid Node A
	dLatA = (float)(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dSlat + (iRow_Index * m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc));
	dLongA = (float)(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + (iColumn_Index * m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc));

	// Compute interpolation scale factors
	dY = (float)((m_pGridfile->dLat - dLatA) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc);
	dX = (float)((m_pGridfile->dLong - dLongA) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);

	///////////// N value /////////////////////
	// Compute N Value interpolation parameters
	da0 = pNShifts[0]->dN_value;
	da1 = pNShifts[1]->dN_value - pNShifts[0]->dN_value;
	da2 = pNShifts[2]->dN_value - pNShifts[0]->dN_value;
	da3 = pNShifts[0]->dN_value + pNShifts[3]->dN_value - pNShifts[1]->dN_value - pNShifts[2]->dN_value;

	// Interpolate final latitude value
	dInterpPoint->gVar.dN_value = da0 + (da1 * dX) + (da2 * dY) + (da3 * dX * dY);
	
	///////////// Deflection in Prime meridian /////////////////////
	// Compute deflection interpolation parameters
	da0 = pNShifts[0]->dDefl_meridian;
	da1 = pNShifts[1]->dDefl_meridian - pNShifts[0]->dDefl_meridian;
	da2 = pNShifts[2]->dDefl_meridian - pNShifts[0]->dDefl_meridian;
	da3 = pNShifts[0]->dDefl_meridian + pNShifts[3]->dDefl_meridian - pNShifts[1]->dDefl_meridian - pNShifts[2]->dDefl_meridian;

	// Interpolate final meridian deflection value
	dInterpPoint->gVar.dDefl_meridian = da0 + (da1 * dX) + (da2 * dY) + (da3 * dX * dY);

	///////////// Deflection in Prime vertical /////////////////////
	// Compute deflection interpolation parameters			
	da0 = pNShifts[0]->dDefl_primev;
	da1 = pNShifts[1]->dDefl_primev - pNShifts[0]->dDefl_primev;
	da2 = pNShifts[2]->dDefl_primev - pNShifts[0]->dDefl_primev;
	da3 = pNShifts[0]->dDefl_primev + pNShifts[3]->dDefl_primev - pNShifts[1]->dDefl_primev - pNShifts[2]->dDefl_primev;

	// Interpolate final vertical deflection value
	dInterpPoint->gVar.dDefl_primev = da0 + (da1 * dX) + (da2 * dY) + (da3 * dX * dY);	

	// Return success
	return ERR_TRANS_SUCCESS;
}
	

int dna_geoid_interpolation::InterpolateNvalue_BiCubic(double dlat, double dlon, geoid_point *dInterpPoint)
{
	double dLatLower, dLongLower, dLongUpper;
	long iRow_Index, iColumn_Index, iGrid_Points_Per_Row;
	long iA, iB, iC, iD, iE, iF, iG, iH, iI, iJ, iK, iL, iM, iN, iO, iP;
	int iFtype;

	// declare a coordinate struct for the four values at each node
	geoid_values cNodeShifts[17];
	geoid_values *pNShifts[17];

	int i;

	// set up pointers
	for (i=0; i<16; i++)
		pNShifts[i] = &(cNodeShifts[i]);

	////////////  1. Retrieve Grid Node values (see Page 18 of the NTv2 Developer's Guide) ////////////
	// Compute Row_Index of Grid Node A...
	iRow_Index = (int)((m_pGridfile->dLat - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dSlat + PRECISION_1E5) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc);

	// Compute Column_Index of Grid Node A...
	iColumn_Index = (int)((m_pGridfile->dLong - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + PRECISION_1E5) / m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);

	// Compute Grid_Points_Per_Row (No of Columns)
	iGrid_Points_Per_Row = (int)((m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dWlong - m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + PRECISION_1E5) /
		m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);
	iGrid_Points_Per_Row++;

	// Compute Record Index for 16 nodes (A, B, C and D, through to node P).
	// P  O  N  M
	// L  D  C  K
	// J  B  A  I
	// H  G  F  E
	
	// Compute the four immediately surrounding node indeces
	iA = (iRow_Index * iGrid_Points_Per_Row) + iColumn_Index + 1;			// Lower Right node
	iB = iA + 1;															// Lower Left node
	iC = iA + iGrid_Points_Per_Row;											// Upper Right node
	iD = iC + 1;															// Upper Left node 

	// Now compute the surrounding 12
	iI = iA - 1;
	iJ = iB + 1;
	iK = iC - 1;
	iL = iD + 1;
	
	iE = iI - iGrid_Points_Per_Row;
	iF = iE + 1;
	iG = iF + 1;
	iH = iG + 1;

	iM = iK + iGrid_Points_Per_Row;
	iN = iM + 1;
	iO = iN + 1;
	iP = iO + 1;

	bool bWithinLowerLatInterval = IsWithinLowerLatitudeGridInterval(&(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid]));
	bool bWithinLowerLongInterval = IsWithinLowerLongitudeGridInterval(&(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid]));
	bool bWithinUpperLatInterval = IsWithinUpperLatitudeGridInterval(&(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid]));
	bool bWithinUpperLongInterval = IsWithinUpperLongitudeGridInterval(&(m_pGridfile->ptrIndex[m_pGridfile->iTheGrid]));

	// Assign the correct grid node number depending on limit flag set in FindSubGrid
	// (see Page 19 & 20 of the NTv2 Developer's Guide)
	switch (m_pGridfile->iPointFlag)
	{
	case 1:
		// point is within the limits...no virtual cell is required
		// check now if surrounding cells need virtual cells
		if (bWithinLowerLatInterval && bWithinLowerLongInterval)
		{
			// lower right
			iI = iA;
			iK = iC;
			iM = iN;
			// lower
			iE = iI;
			iF = iA;
			iG = iB;
			iH = iJ;
		}
		else if (bWithinLowerLatInterval && bWithinUpperLongInterval)
		{
			// lower left
			iJ = iB;
			iL = iD;
			iP = iO;
			// lower
			iE = iI;
			iF = iA;
			iG = iB;
			iH = iJ;
		}
		else if (bWithinLowerLatInterval)
		{
			// lower
			iE = iI;
			iF = iA;
			iG = iB;
			iH = iJ;
		}
		else if (bWithinUpperLatInterval && bWithinLowerLongInterval)
		{
			// upper rght
			iK = iC;
			iI = iA;
			iE = iF;
			// upper
			iM = iK;
			iN = iC;
			iO = iD;
			iP = iL;
		}
		else if (bWithinUpperLatInterval && bWithinUpperLongInterval)
		{
			// upper left
			iL = iD;
			iJ = iB;
			iP = iO;
			// upper
			iM = iK;
			iN = iC;
			iO = iD;
			iP = iL;
		}
		else if (bWithinUpperLatInterval)
		{
			// upper
			iM = iK;
			iN = iC;
			iO = iD;
			iP = iL;
		}
		else if (bWithinLowerLongInterval)
		{
			// right
			iE = iF;
			iI = iA;
			iK = iC;
			iM = iN;
		}
		else if (bWithinUpperLongInterval)
		{
			// left
			iH = iG;
			iJ = iB;
			iL = iD;
			iP = iO;
		}
		break;
	case 2:
		// point is on the upper latitude limit
		if (bWithinLowerLongInterval)
		{
			iI = iA;
			iE = iF;
		}
		else if (bWithinUpperLongInterval)
		{
			iJ = iB;
			iH = iG;
		}
		iM = iK = iI;
		iN = iC = iA;				// Upper Right node becomes Lower Right node
		iO = iD = iB;				// Upper Left node becomes Lower Left node
		iP = iL = iJ;		
		break;
	case 3:
		// point is on the upper longitude limit
		if (bWithinLowerLatInterval)
		{
			iF = iA;
			iE = iI;
		}
		if (bWithinUpperLatInterval)
		{
			iN = iC;
			iK = iM;
		}
		iH = iG = iF;
		iJ = iB = iA;				// Lower Left node becomes Lower Right node
		iL = iD = iC;				// Upper Left node becomes Upper Right node
		iP = iO = iN;
		break;
	case 4:
		// point is on both upper latitude and upper longitude limits
		iJ = iB = iA;				// Lower Left node becomes Lower Right node
		iN = iC = iA;				// Upper Right node becomes Lower Right node
		iD = iA;					// Upper Left node becomes Lower Right node
		iP = iL = iO = iD;
		iH = iG = iF;
		iM = iK = iI;
		break;
	}

	// determine grid file type...ASCII = 0; Binary = 1.
	iFtype = DetermineFileType(m_pGridfile->filetype);
	if (iFtype < TYPE_ASC || iFtype > TYPE_GSB)
		return ERR_GRIDFILE_TYPE;

	// initialise pNShifts index
	i = 0;

	// Retrieve the values at the sixteen grid nodes.
	if (iFtype == TYPE_ASC)
	{	
		if (!(ReadAsciiShifts(pNShifts, i++, iA)))			// Node A
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iB)))			// Node B
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iC)))			// Node C
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iD)))			// Node D
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iE)))			// Node E
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iF)))			// Node F
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iG)))			// Node G
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iH)))			// Node H
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iI)))			// Node I
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iJ)))			// Node J
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iK)))			// Node K
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iL)))			// Node L
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iM)))			// Node M
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iN)))			// Node N
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i++, iO)))			// Node O
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file

		if (!(ReadAsciiShifts(pNShifts, i, iP)))			// Node P
			return ERR_READ_ASC_SHIFT;		// Could not retrieve shifts from Ascii file
	}
	else if (iFtype == TYPE_GSB)
	{	
		if (!(ReadBinaryShifts(pNShifts, i++, iA)))			// Node A
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iB)))			// Node B
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iC)))			// Node C
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iD)))			// Node D
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iE)))			// Node E
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iF)))			// Node F
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iG)))			// Node G
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iH)))			// Node H
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iI)))			// Node I
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iJ)))			// Node J
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iK)))			// Node K
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iL)))			// Node L
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iM)))			// Node M
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iN)))			// Node N
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i++, iO)))			// Node O
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file

		if (!(ReadBinaryShifts(pNShifts, i, iP)))			// Node P
			return ERR_READ_BIN_SHIFT;		// Could not retrieve shifts from Binary file
	}

	////////////  2. Interpolate Grid Shifts  (see Page 126 of Numerical Recipes in C) ////////////
	// Compute Coordinates of x1 and x2 (lower)
	dLatLower = m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dSlat + (iRow_Index * m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc);
	dLongLower = m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dElong + (iColumn_Index * m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc);
	// Compute Coordinates of x2 (upper)
	dLongUpper = dLongLower + m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc;

	double D1 = fabs(m_pGridfile->dLong - dLongUpper);
	double D2 = fabs(m_pGridfile->dLat - dLatLower);

	// ------------- put the following in a function, as it is used for N value, deflection Meridian and deflection P vertical
	///< ----------- interpolation of N value ------------------>
	double n[4] = { pNShifts[1]->dN_value, pNShifts[0]->dN_value, pNShifts[2]->dN_value, pNShifts[3]->dN_value} ;
	double n1[4] = {
		// [row][column]
		(pNShifts[0]->dN_value - pNShifts[9]->dN_value) / 2.,
		(pNShifts[8]->dN_value - pNShifts[1]->dN_value) / 2.,
		(pNShifts[10]->dN_value - pNShifts[3]->dN_value) / 2.,
		(pNShifts[2]->dN_value - pNShifts[11]->dN_value) / 2.
	};
	double n2[4] = { 
		(pNShifts[3]->dN_value - pNShifts[6]->dN_value) / 2.,
		(pNShifts[2]->dN_value - pNShifts[5]->dN_value) / 2.,
		(pNShifts[13]->dN_value - pNShifts[0]->dN_value) / 2.,
		(pNShifts[14]->dN_value - pNShifts[1]->dN_value) / 2.
	};
	double n12[4] = { 
		(pNShifts[2]->dN_value - pNShifts[5]->dN_value - pNShifts[11]->dN_value + pNShifts[7]->dN_value) / 4.,
		(pNShifts[10]->dN_value - pNShifts[4]->dN_value - pNShifts[3]->dN_value + pNShifts[6]->dN_value) / 4.,
		(pNShifts[12]->dN_value - pNShifts[8]->dN_value - pNShifts[14]->dN_value + pNShifts[1]->dN_value) / 4.,
		(pNShifts[13]->dN_value - pNShifts[0]->dN_value - pNShifts[15]->dN_value + pNShifts[9]->dN_value) / 4.
	};

	try {
		bcuint<double>(n, n1, n2, n12, D1, D2, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc, 
			&(dInterpPoint->gVar.dN_value));
	}
	catch (const runtime_error& e) {
		throw NetGeoidException(e.what(), 0); 
	}

	///< ----------- interpolation of prime meridian ------------------>
	double m[4] = { pNShifts[1]->dDefl_meridian, pNShifts[0]->dDefl_meridian, pNShifts[2]->dDefl_meridian, pNShifts[3]->dDefl_meridian};
	double m1[4] = {
		// [row][column]
		(pNShifts[0]->dDefl_meridian - pNShifts[9]->dDefl_meridian) / 2.,
		(pNShifts[8]->dDefl_meridian - pNShifts[1]->dDefl_meridian) / 2.,
		(pNShifts[10]->dDefl_meridian - pNShifts[3]->dDefl_meridian) / 2.,
		(pNShifts[2]->dDefl_meridian - pNShifts[11]->dDefl_meridian) / 2.
	};
	double m2[4] = { 
		(pNShifts[3]->dDefl_meridian - pNShifts[6]->dDefl_meridian) / 2.,
		(pNShifts[2]->dDefl_meridian - pNShifts[5]->dDefl_meridian) / 2.,
		(pNShifts[13]->dDefl_meridian - pNShifts[0]->dDefl_meridian) / 2.,
		(pNShifts[14]->dDefl_meridian - pNShifts[1]->dDefl_meridian) / 2.
	};
	double m12[4] = { 
		(pNShifts[2]->dDefl_meridian - pNShifts[5]->dDefl_meridian - pNShifts[11]->dDefl_meridian + pNShifts[7]->dDefl_meridian) / 4.,
		(pNShifts[10]->dDefl_meridian - pNShifts[4]->dDefl_meridian - pNShifts[3]->dDefl_meridian + pNShifts[6]->dDefl_meridian) / 4.,
		(pNShifts[12]->dDefl_meridian - pNShifts[8]->dDefl_meridian - pNShifts[14]->dDefl_meridian + pNShifts[1]->dDefl_meridian) / 4.,
		(pNShifts[13]->dDefl_meridian - pNShifts[0]->dDefl_meridian - pNShifts[15]->dDefl_meridian + pNShifts[9]->dDefl_meridian) / 4.
	};

	// interpolation
	try {
		bcuint<double>(m, m1, m2, m12, D1, D2, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc, 
			&(dInterpPoint->gVar.dDefl_meridian));

	}
	catch (const runtime_error& e) {
		throw NetGeoidException(e.what(), 0); 
	}

	///< ----------- interpolation of prime vertical ------------------>
	double v[4] = { pNShifts[1]->dDefl_primev, pNShifts[0]->dDefl_primev, pNShifts[2]->dDefl_primev, pNShifts[3]->dDefl_primev} ;
	double v1[4] = {
		// [row][column]
		(pNShifts[0]->dDefl_primev - pNShifts[9]->dDefl_primev) / 2.,
		(pNShifts[8]->dDefl_primev - pNShifts[1]->dDefl_primev) / 2.,
		(pNShifts[10]->dDefl_primev - pNShifts[3]->dDefl_primev) / 2.,
		(pNShifts[2]->dDefl_primev - pNShifts[11]->dDefl_primev) / 2.
	};
	double v2[4] = { 
		(pNShifts[3]->dDefl_primev - pNShifts[6]->dDefl_primev) / 2.,
		(pNShifts[2]->dDefl_primev - pNShifts[5]->dDefl_primev) / 2.,
		(pNShifts[13]->dDefl_primev - pNShifts[0]->dDefl_primev) / 2.,
		(pNShifts[14]->dDefl_primev - pNShifts[1]->dDefl_primev) / 2.
	};
	double v12[4] = { 
		(pNShifts[2]->dDefl_primev - pNShifts[5]->dDefl_primev - pNShifts[11]->dDefl_primev + pNShifts[7]->dDefl_primev) / 4.,
		(pNShifts[10]->dDefl_primev - pNShifts[4]->dDefl_primev - pNShifts[3]->dDefl_primev + pNShifts[6]->dDefl_primev) / 4.,
		(pNShifts[12]->dDefl_primev - pNShifts[8]->dDefl_primev - pNShifts[14]->dDefl_primev + pNShifts[1]->dDefl_primev) / 4.,
		(pNShifts[13]->dDefl_primev - pNShifts[0]->dDefl_primev - pNShifts[15]->dDefl_primev + pNShifts[9]->dDefl_primev) / 4.
	};

	// interpolation
	try {
		bcuint<double>(v, v1, v2, v12, D1, D2, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLonginc, 
			&m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].dLatinc, 
			&(dInterpPoint->gVar.dDefl_primev));
	}
	catch (const runtime_error& e) {
		throw NetGeoidException(e.what(), 0); 
	}


	//TRACE("Interpolation values: %.3f, %.3f, %.3f\n", dInterpPoint->gVar.dN_value, dInterpPoint->gVar.dDefl_meridian, dInterpPoint->gVar.dDefl_primev);	

	return ERR_TRANS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////
// ReadAsciiShifts:	Reads the Latitude and Longitude shifts & accuracies from an
//					an ascii grid file.
/////////////////////////////////////////////////////////////////////////
// On Entry:  A char array is read (being one node record) from the grid file
//			  This array should contain (as text) 1 Latitude shift, 
//			  1 Longitude shift, 1 Latitude accuracy and 1 Longitude accuracy. A
//			  valid file pointer must exist.
// On Exit:   Four floats are read from the char array. The success of the sscanf_s
//			  from the array is assigned 1 or 0. The floats are then stored as 
//			  doubles and returned via the "crdNode" struct of type "coordinate".
/////////////////////////////////////////////////////////////////////////
bool dna_geoid_interpolation::ReadAsciiShifts(geoid_values *pNShifts[], int iNodeIndex, long lNode)
{
	float fNum1, fNum2, fNum3, fNum4;
	char cBuf[DATA_RECORD];
	int filePos;

	try {
		// calculate file position and set stream pointer
		filePos = m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].iGridPos + ((lNode-1) * 42);
		m_pGfileptr.seekg(filePos);
			
		// Retrieve the whole line for node A
		m_pGfileptr.getline(cBuf, DATA_RECORD);
		if (sscanf(cBuf, SHIFTS, &fNum1, &fNum2, &fNum3, &fNum4) < 4)
			return false;			// Could not read from grid file
	}
	catch (...) {
		return false;
	}
	
	pNShifts[iNodeIndex]->dN_value = fNum1;				// metres
	pNShifts[iNodeIndex]->dDefl_meridian = fNum2;		// seconds by default
	pNShifts[iNodeIndex]->dDefl_primev = fNum3;			// seconds by default

	if (strcmp(m_pGridfile->chGs_type, "RADIANS ") == 0)
	{
		// convert radians values to seconds
		pNShifts[iNodeIndex]->dDefl_meridian *= (float)RAD_TO_SEC;
		pNShifts[iNodeIndex]->dDefl_primev *= (float)RAD_TO_SEC;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
// ReadBinaryShifts: Reads the Latitude and Longitude shifts & accuracies from an
//					 a binary grid file.
/////////////////////////////////////////////////////////////////////////
// On Entry:  A char array is read (being one node record) from the grid file
//			  This array should contain (as text) 1 Latitude shift, 
//			  1 Longitude shift, 1 Latitude accuracy and 1 Longitude accuracy. A
//			  valid file pointer must exist.
// On Exit:   Four floats are read from the char array. The success of the sscanf_s
//			  from the array is assigned 1 or 0. The floats are then stored as 
//			  doubles and returned via the "crdNode" struct of type "coordinate".
/////////////////////////////////////////////////////////////////////////
bool dna_geoid_interpolation::ReadBinaryShifts(geoid_values *pNShifts[], int iNodeIndex, long lNode)
{
	float fNum;
	
	try {
		// calculate file position and set stream pointer
		int filePos = m_pGridfile->ptrIndex[m_pGridfile->iTheGrid].iGridPos + ((lNode-1) * 4 * sizeof(float));
		m_pGfileptr.seekg(filePos);
	
		m_pGfileptr.read(reinterpret_cast<char *>(&fNum), sizeof(float));
		pNShifts[iNodeIndex]->dN_value = fNum;	

		m_pGfileptr.read(reinterpret_cast<char *>(&fNum), sizeof(float));
		pNShifts[iNodeIndex]->dDefl_meridian = fNum;	

		m_pGfileptr.read(reinterpret_cast<char *>(&fNum), sizeof(float));
		pNShifts[iNodeIndex]->dDefl_primev = fNum;	
	}
	catch (...) {
		return false;
	}
	
	if (strcmp(m_pGridfile->chGs_type, "RADIANS ") == 0)
	{
		// convert radians values to seconds
		pNShifts[iNodeIndex]->dDefl_meridian *= (float)RAD_TO_SEC;
		pNShifts[iNodeIndex]->dDefl_primev *= (float)RAD_TO_SEC;
	}

	return true;
}

int dna_geoid_interpolation::OpenGridFile(const char *filename, const char *filetype, n_file_par* pGridfile, bool isTEMP)
{
	n_file_par* ptheGrid;
	std::ifstream grid_ifs, *pgrid_ifs;

	if (isTEMP)
	{
		// temporary n_file_par
		if (pGridfile == NULL)
			pGridfile = new n_file_par;
		ptheGrid = pGridfile;

		// temporary std::ifstream
		pgrid_ifs = &grid_ifs;
	}
	else
	{
		// in-memory n_file_par
		if (m_pGridfile == NULL)
			m_pGridfile = new n_file_par;
		ptheGrid = m_pGridfile;

		// in-memory std::ifstream
		if (m_pGfileptr.is_open())
			m_pGfileptr.close();
		m_pGfileptr.clear();
		pgrid_ifs = &m_pGfileptr;
	}

	if (ptheGrid->ptrIndex != NULL)
		delete [] ptheGrid->ptrIndex;

	strcpy(ptheGrid->filename, trimstr<string>(filename).c_str());
	strcpy(ptheGrid->filetype, trimstr<string>(filetype).c_str());
	
	int iLineLength = 4 * sizeof(float);
	ptheGrid->Can_Format = true;			// assume file will be in Canadian format
	
	// Initialise Success
	int gridType = DetermineFileType(ptheGrid->filetype);

	// Open the distortion grid file
	try {
		if (gridType == TYPE_ASC)
			// open in ascii mode and go to end of file
			file_opener(pgrid_ifs, ptheGrid->filename, ios::in | ios::ate, ascii, true);
		else if (gridType == TYPE_GSB)
			// open in binary mode and go to end of file
			file_opener(pgrid_ifs, ptheGrid->filename, ios::in | ios::binary | ios::ate, binary, true);
		else
			// unknown file type, so throw an exception
			throw NetGeoidException(ErrorString(ERR_GRIDFILE_TYPE), ERR_GRIDFILE_TYPE);
	}
	catch (const runtime_error& e) {
		ClearGridFileMemory();
		stringstream ss;
		ss << ErrorString(ERR_GRIDFILE_READ) << "\n" <<
			"  " << e.what();
		throw NetGeoidException(ss.str(), ERR_GRIDFILE_READ);
	}

	// calculate file length
	ptheGrid->iGfilelength = (int)pgrid_ifs->tellg();
	pgrid_ifs->seekg(0, ios::beg);

	char identifier[IDENT_BUF + 1], cBuf[HEADER_RECORD + 1];

	try {
		// read in all Overview Header information
		if (gridType == TYPE_ASC)		// ascii
		{
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%d", &ptheGrid->iH_info);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%d", &ptheGrid->iSubH_info);
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%d", &ptheGrid->iNumsubgrids);
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->chGs_type, OVERVIEW_RECS, '\n');

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->chVersion, OVERVIEW_RECS, '\n');

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->chSystem_f, OVERVIEW_RECS, '\n');

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->chSystem_t, OVERVIEW_RECS, '\n');
				
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->daf);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->dbf);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->dat);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->dbt);
		}
		else					// binary
		{
			pgrid_ifs->ignore(IDENT_BUF);
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->iH_info), sizeof(int));		// NUM_OREC

			pgrid_ifs->read(reinterpret_cast<char *>(identifier), NULL_PAD);						// "NUM_" or "    "
			
			// Test for Australian/Canadian Binary
			if (strncmp("NUM_", identifier, NULL_PAD) == 0)
			{
				// Australian Binary format
				// It is highly unlikely that any of these files will be in existence.
				// Nonetheless, cater for it anyway to prevent an exception from being thrown.
				pgrid_ifs->ignore(NULL_PAD);														// SREC
				pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->iSubH_info), sizeof(int));

				pgrid_ifs->ignore(IDENT_BUF);														// NUM_FILE
				pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->iNumsubgrids), sizeof(int));

				ptheGrid->Can_Format = false;
			}
			else
			{
				// Canadian Binary format
				pgrid_ifs->ignore(IDENT_BUF);														// SREC
				pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->iSubH_info), sizeof(int));
				pgrid_ifs->ignore(NULL_PAD);

				pgrid_ifs->ignore(IDENT_BUF);														// NUM_FILE
				pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->iNumsubgrids), sizeof(int));
				pgrid_ifs->ignore(NULL_PAD);
			}

			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->chGs_type), IDENT_BUF);

			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->chVersion), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->chSystem_f), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->chSystem_t), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->daf), sizeof(double));

			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->dbf), sizeof(double));

			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->dat), sizeof(double));

			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->dbt), sizeof(double));
		}
	}
	catch (...) {
		return ERR_GRID_CORRUPT;
	}
	
	// create header file array
	// fills the new elements with default values
	ptheGrid->ptrIndex = new n_gridfileindex[ptheGrid->iNumsubgrids];

	for (int i=0; i<ptheGrid->iNumsubgrids; i++)
	{	
		if (gridType == TYPE_ASC)		// ascii
		{
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->ptrIndex[i].chSubname, OVERVIEW_RECS, '\n');
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->ptrIndex[i].chParent, OVERVIEW_RECS, '\n');
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->ptrIndex[i].chCreated, OVERVIEW_RECS, '\n');
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(ptheGrid->ptrIndex[i].chUpdated, OVERVIEW_RECS, '\n');
			
			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dSlat);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dNlat);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dElong);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dWlong);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dLatinc);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%lf", &ptheGrid->ptrIndex[i].dLonginc);

			pgrid_ifs->ignore(OVERVIEW_RECS);
			pgrid_ifs->getline(cBuf, OVERVIEW_RECS, '\n');
			sscanf(cBuf, "%ld", &ptheGrid->ptrIndex[i].lGscount);

			// Save ASCII position in grid file for first record of lat & long shifts 
			ptheGrid->ptrIndex[i].iGridPos = (int)pgrid_ifs->tellg();

			// Set the file position after the GS_COUNT number...which is the start of the next sub file.
			pgrid_ifs->seekg(ptheGrid->ptrIndex[i].iGridPos + (ptheGrid->ptrIndex[i].lGscount * 42));
		}
		else					// binary
		{
		
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->ptrIndex[i].chSubname), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->ptrIndex[i].chParent), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->ptrIndex[i].chCreated), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(ptheGrid->ptrIndex[i].chUpdated), IDENT_BUF);
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dSlat), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dNlat), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dElong), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dWlong), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dLatinc), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].dLonginc), sizeof(double));
			
			pgrid_ifs->ignore(IDENT_BUF);	
			pgrid_ifs->read(reinterpret_cast<char *>(&ptheGrid->ptrIndex[i].lGscount), sizeof(int));
			
			if (ptheGrid->Can_Format)			// Canadian binary format?
				pgrid_ifs->ignore(NULL_PAD);	
		
			// get Binary position in grid file for first record of lat & long shifts 
			ptheGrid->ptrIndex[i].iGridPos = (int)pgrid_ifs->tellg();
	
			// End of file encountered first???
			pgrid_ifs->seekg(ptheGrid->ptrIndex[i].lGscount * iLineLength, ios::cur);
		}

		if (strcmp(ptheGrid->chGs_type, "RADIANS ") == 0)
		{
			// convert all radians values to seconds
			ptheGrid->ptrIndex[i].dSlat *= RAD_TO_SEC;					// S_LAT
			ptheGrid->ptrIndex[i].dNlat *= RAD_TO_SEC;					// N_LAT
			ptheGrid->ptrIndex[i].dElong *= RAD_TO_SEC;					// E_LONG
			ptheGrid->ptrIndex[i].dWlong *= RAD_TO_SEC;					// W_LONG
			ptheGrid->ptrIndex[i].dLatinc *= RAD_TO_SEC;				// LAT_INC
			ptheGrid->ptrIndex[i].dLonginc *= RAD_TO_SEC;				// LONG_INC
		}
	}

	return 0;
}

void dna_geoid_interpolation::PrintDefaultGridHeaderInfo(std::ofstream* f_out, n_file_par* pGridfile, bool isBinary)
{
	char cPad[NULL_PAD + 1];
	memset(cPad, '\0', NULL_PAD);

	// print header block information
	f_out->write(reinterpret_cast<const char *>("NUM_OREC"), OVERVIEW_RECS);	// Number of header identifiers (NUM_OREC)
	f_out->write(reinterpret_cast<char *>(&pGridfile->iH_info), sizeof(int));
	f_out->write(reinterpret_cast<char *>(cPad), NULL_PAD);				// Pad out extra 4 bytes of null space
	
	f_out->write(reinterpret_cast<const char *>("NUM_SREC"), OVERVIEW_RECS);	// Number of header identifiers (NUM_SREC)
	f_out->write(reinterpret_cast<char *>(&pGridfile->iSubH_info), sizeof(int));
	f_out->write(reinterpret_cast<char *>(cPad), NULL_PAD);				// Pad out extra 4 bytes of null space
		
	f_out->write(reinterpret_cast<const char *>("NUM_FILE"), OVERVIEW_RECS);	// number of subgrids in file (NUM_FILE)
	f_out->write(reinterpret_cast<char *>(&pGridfile->iNumsubgrids), sizeof(int));
	f_out->write(reinterpret_cast<char *>(cPad), NULL_PAD);				// Pad out extra 4 bytes of null space
	
	f_out->write(reinterpret_cast<const char *>("GS_TYPE "), OVERVIEW_RECS);	// grid shift type (GS_TYPE)
	f_out->write(reinterpret_cast<char *>(&pGridfile->chGs_type), OVERVIEW_RECS);
	
	f_out->write(reinterpret_cast<const char *>("VERSION "), OVERVIEW_RECS);	// grid shift version (VERSION)
	f_out->write(reinterpret_cast<char *>(&pGridfile->chVersion), OVERVIEW_RECS);
	
	f_out->write(reinterpret_cast<const char *>("SYSTEM_F"), OVERVIEW_RECS);	// reference system (SYSTEM_F)
	f_out->write(reinterpret_cast<char *>(&pGridfile->chSystem_f), OVERVIEW_RECS);
	
	f_out->write(reinterpret_cast<const char *>("SYSTEM_T"), OVERVIEW_RECS);	// reference system (SYSTEM_T)
	f_out->write(reinterpret_cast<char *>(&pGridfile->chSystem_t), OVERVIEW_RECS);
	
	f_out->write(reinterpret_cast<const char *>("MAJOR_F "), OVERVIEW_RECS);	// semi major of from system (MAJOR_F)
	f_out->write(reinterpret_cast<char *>(&pGridfile->daf), sizeof(double));
	
	f_out->write(reinterpret_cast<const char *>("MINOR_F "), OVERVIEW_RECS);	// semi minor of from system (MINOR_F)
	f_out->write(reinterpret_cast<char *>(&pGridfile->dbf), sizeof(double));
	
	f_out->write(reinterpret_cast<const char *>("MAJOR_T "), OVERVIEW_RECS);	// semi major of to system (MAJOR_T)
	f_out->write(reinterpret_cast<char *>(&pGridfile->dat), sizeof(double));
	
	f_out->write(reinterpret_cast<const char *>("MINOR_T "), OVERVIEW_RECS);	// semi minor of to system (MINOR_T)
	f_out->write(reinterpret_cast<char *>(&pGridfile->dbt), sizeof(double));
}

void dna_geoid_interpolation::PrintDefaultSubGridHeaderInfo(std::ofstream* f_out, n_gridfileindex* gfIndex, const char* chGs_type, bool isBinary)
{
	// Print header info for sub-grid
	f_out->write(reinterpret_cast<const char *>("SUB_NAME"), OVERVIEW_RECS);
	f_out->write(reinterpret_cast<char *>(&gfIndex->chSubname), OVERVIEW_RECS);

	f_out->write(reinterpret_cast<const char *>("PARENT  "), OVERVIEW_RECS);
	f_out->write(reinterpret_cast<char *>(&gfIndex->chParent), OVERVIEW_RECS);

	f_out->write(reinterpret_cast<const char *>("CREATED "), OVERVIEW_RECS);
	f_out->write(reinterpret_cast<char *>(&gfIndex->chCreated), OVERVIEW_RECS);

	f_out->write(reinterpret_cast<const char *>("UPDATED "), OVERVIEW_RECS);
	f_out->write(reinterpret_cast<char *>(&gfIndex->chUpdated), OVERVIEW_RECS);

	if (strcmp(chGs_type, "RADIANS ") == 0)
	{
		double dTemp;
		// convert all seconds values to radians
		f_out->write(reinterpret_cast<const char *>("S_LAT   "), OVERVIEW_RECS);
		dTemp = gfIndex->dSlat / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("N_LAT   "), OVERVIEW_RECS);
		dTemp = gfIndex->dNlat / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("E_LONG  "), OVERVIEW_RECS);
		dTemp = gfIndex->dElong / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("W_LONG  "), OVERVIEW_RECS);
		dTemp = gfIndex->dWlong / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("LAT_INC "), OVERVIEW_RECS);
		dTemp = gfIndex->dLatinc / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("LONG_INC"), OVERVIEW_RECS);
		dTemp = gfIndex->dLonginc / RAD_TO_SEC;
		f_out->write(reinterpret_cast<char *>(&dTemp), sizeof(double));
	}
	else
	{
		f_out->write(reinterpret_cast<const char *>("S_LAT   "), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dSlat), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("N_LAT   "), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dNlat), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("E_LONG  "), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dElong), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("W_LONG  "), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dWlong), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("LAT_INC "), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dLatinc), sizeof(double));
		
		f_out->write(reinterpret_cast<const char *>("LONG_INC"), OVERVIEW_RECS);
		f_out->write(reinterpret_cast<char *>(&gfIndex->dLonginc), sizeof(double));
	}

	f_out->write(reinterpret_cast<const char *>("GS_COUNT"), OVERVIEW_RECS);
	f_out->write(reinterpret_cast<char *>(&gfIndex->lGscount), sizeof(int));
	
	char cPad[NULL_PAD + 1];
	memset(cPad, '\0', NULL_PAD);
	f_out->write(reinterpret_cast<char *>(cPad), NULL_PAD);				// Pad out extra 4 bytes of null space
}

// Sets up the temporary grid file buffers with default parameters (based on national grid file)
//
//  Default AUSGEOID parameters (whole continent
//	----------- deg  sec
//	S_LAT       -46  -165600
//	N_LAT        -8  -28800
//	E_LONG     -160  -576000
//	W_LONG     -108  -388800
//		
//	----------- min  sec
//	LAT_INC       1  60
//	LONG_INC      1  60
//
void dna_geoid_interpolation::SetDefaultGridFileParametersTmp(n_file_par* ntv2_params)
{		
	// set grid file settings
	sprintf(ntv2_params->filename, "%s", ".");			// NULL path
	sprintf(ntv2_params->filetype, "%s", "GSB");		// file type

	// set Overview Header information
	ntv2_params->iH_info = 11;								// NUM_OREC
	ntv2_params->iSubH_info = 11;							// NUM_SREC
	ntv2_params->iNumsubgrids = 1;							// NUM_FILE
	sprintf(ntv2_params->chGs_type, "%s", "SECONDS ");		// GS_TYPE
	sprintf(ntv2_params->chVersion, "%s", "GEOID98 ");		// VERSION
	sprintf(ntv2_params->chSystem_f, "%s", "ANS     ");		// SYSTEM_F
	sprintf(ntv2_params->chSystem_t, "%s", "ORTHOMET");		// SYSTEM_T
	ntv2_params->daf = 6378137.000;							// MAJOR_F
	ntv2_params->dbf = 6356752.314;							// MINOR_F
	ntv2_params->dat = 6378137.000;							// MAJOR_T
	ntv2_params->dbt = 6356752.314;							// MINOR_T

	if (!ntv2_params->ptrIndex)
	{
		ntv2_params->ptrIndex = new n_gridfileindex;
		memset(ntv2_params->ptrIndex, 0, sizeof(n_gridfileindex));

		sprintf(ntv2_params->ptrIndex[0].chSubname, "%s", "AUSGEOID");	// SUB_NAME
		sprintf(ntv2_params->ptrIndex[0].chParent, "%s", "NONE    ");	// PARENT
		sprintf(ntv2_params->ptrIndex[0].chCreated, "%s", "20012010");	// CREATED
		sprintf(ntv2_params->ptrIndex[0].chUpdated, "%s", "20012010");	// UPDATED
		ntv2_params->ptrIndex[0].dSlat = -165600.;				// S_LAT    (-45* 60')
		ntv2_params->ptrIndex[0].dNlat = -28800.;				// N_LAT    (- 8* 00')
		ntv2_params->ptrIndex[0].dElong = -576000.;				// E_LONG   (160* 00')
		ntv2_params->ptrIndex[0].dWlong = -388800.;				// W_LONG   (108* 00')
		ntv2_params->ptrIndex[0].dLatinc = 60.;					// LAT_INC  (  0* 01')
		ntv2_params->ptrIndex[0].dLonginc = 60.;				// LONG_INC (  0* 01')
		ntv2_params->ptrIndex[0].lGscount = -1;					// GS_COUNT
		ntv2_params->ptrIndex[0].iGridPos = 0;					// file pointer to file position of this subgrid
	}
}

void dna_geoid_interpolation::WriteBinaryRecords(std::ofstream* f_out, float n_value, float dDefl_meridian, float dDefl_primev)
{
	//float fGSRecord[5];
	//
	//// Set the values
	//fGSRecord[0] = n_value;
	//fGSRecord[1] = dDefl_meridian;
	//fGSRecord[2] = dDefl_primev;
	//fGSRecord[3] = 0.;

	float dZero(0.);
	f_out->write(reinterpret_cast<char *>(&n_value), sizeof(float));
	f_out->write(reinterpret_cast<char *>(&dDefl_meridian), sizeof(float));
	f_out->write(reinterpret_cast<char *>(&dDefl_primev), sizeof(float));
	f_out->write(reinterpret_cast<char *>(&dZero), sizeof(float));
}

void dna_geoid_interpolation::ScanDatFileValues(char* szLine, float* n_value, char* c_northsouth, int* lat_deg, int* lat_min, float* lat_sec, char* c_eastwest, int* lon_deg, int* lon_min, float* lon_sec, float* dDefl_meridian, float* dDefl_primev)  
{
	// extract data (AusGeoid98 file)
	sscanf(&(szLine[4]), "%f", n_value);
	sscanf(&(szLine[13]), "%c", c_northsouth);
	sscanf(&(szLine[14]), "%d", lat_deg);
	sscanf(&(szLine[17]), "%d", lat_min);
	sscanf(&(szLine[20]), "%f", lat_sec);
	sscanf(&(szLine[27]), "%c", c_eastwest);
	sscanf(&(szLine[28]), "%d", lon_deg);
	sscanf(&(szLine[32]), "%d", lon_min);
	sscanf(&(szLine[35]), "%f", lon_sec);
	sscanf(&(szLine[44]), "%f", dDefl_meridian);
	sscanf(&(szLine[53]), "%f", dDefl_primev);
}

void dna_geoid_interpolation::ScanNodeLocations(char* szLine, double* latitude, double* longitude, const UINT32& lNodeRead)
{
	// extract data (AusGeoid98 file)
	double deg, min, sec;
	sscanf(&(szLine[14]), "%lf", &deg);
	sscanf(&(szLine[17]), "%lf", &min);
	sscanf(&(szLine[20]), "%lf", &sec);
	
	*latitude = deg + (min/60.) + (sec/3600.);

	if (szLine[13] == 'S')
		*latitude *= -1;	
	
	sscanf(&(szLine[28]), "%lf", &deg);
	sscanf(&(szLine[32]), "%lf", &min);
	sscanf(&(szLine[35]), "%lf", &sec);

	*longitude = deg + (min/60.) + (sec/3600.);
	
	if (szLine[27] == 'E')
		*longitude *= -1;
}
	

void dna_geoid_interpolation::UpdateHeaderInfo(std::ofstream* f_out)
{
	// move the file pointer to S_LAT and update
	f_out->seekp(248, ios::beg);
	f_out->write(reinterpret_cast<char *>(&m_pGridfile->ptrIndex[0].dSlat), sizeof(double));
	
	// set the file pointer to E_LONG and update 
	f_out->seekp(280, ios::beg);
	f_out->write(reinterpret_cast<char *>(&m_pGridfile->ptrIndex[0].dElong), sizeof(double));
	
	// set the file pointer to LAT_INC and update 
	f_out->seekp(312, ios::beg);
	f_out->write(reinterpret_cast<char *>(&m_pGridfile->ptrIndex[0].dLatinc), sizeof(double));
		
	// set the file pointer to LONG_INC and update 
	f_out->seekp(328, ios::beg);
	f_out->write(reinterpret_cast<char *>(&m_pGridfile->ptrIndex[0].dLonginc), sizeof(double));
	
	char cPad[NULL_PAD + 1];
	memset(cPad, '\0', NULL_PAD);
	
	// set the file pointer to GS_COUNT and update
	f_out->seekp(344, ios::beg);
	f_out->write(reinterpret_cast<char *>(&m_pGridfile->ptrIndex[0].lGscount), sizeof(int));
	f_out->write(reinterpret_cast<char *>(cPad), NULL_PAD);				// Pad out extra 4 bytes of null space
}


void dna_geoid_interpolation::ComputeLatLong(double* dlat_initial, const char& c_northsouth, const int& lat_deg, const int& lat_min, const double& lat_sec, double* dlon_initial, const char& c_eastwest, const int& lon_deg, const int& lon_min, const double& lon_sec)
{
	*dlat_initial = (double)lat_deg + ((double)lat_min)/100. + (lat_sec)/10000.;	
	if (c_northsouth == 'S')
		*dlat_initial *= -1;

	DmstoDeg(*dlat_initial, dlat_initial);

	// *** NOTE ***    NTv2 uses a positive west system
	*dlon_initial = (double)lon_deg + ((double)lon_min)/100. + (lon_sec)/10000.;	
	if (c_eastwest == 'E')
		*dlon_initial *= -1;

	DmstoDeg(*dlon_initial, dlon_initial);
}

// When the file is first read, it is likely that m_gfIndex->dLatinc will equal 0.
// On the next pass, m_gfIndex->dLatinc will either equal dlat_separation, or 0, depending
// on whether increments progress column-wise first or row-wise first.
// AusGeoid files typically run column-wise first (left to right on longitude), then 
// row-wise (top to bottom on latitude). However, this assumption will not be used for the benefit of doubt!!!
// Every record following needs to be tested to see whether:
//   1. the increment is significant (i.e. 160�00'00" - 160�02'00"), AND 
//   2. that the current grid node record is not on the opposite edge of the grid to the last node.
//      That is, last_long = 160�00'00", and current_long = 108�00'00".
// TOLERANCE is set to 1.0 seconds. The default interval is 2 minutes (or 120 seconds).
void dna_geoid_interpolation::ReComputeGridSeparation(const double& dlat_current, const double& dlon_current, const double& dlat_previous, const double& dlon_previous)
{
	double dlat_separation = fabs(dlat_current - dlat_previous) * DEG_TO_SEC;
	double dlon_separation = fabs(dlon_current - dlon_previous) * DEG_TO_SEC;

	// has there been an increment?	
	if ((dlat_separation - 0.0) > TOLERANCE_ZERO)
	{
		// is the increment of a significant size, but not excessive (i.e. the width of the grid)
		// Yes - this is fraught with disaster, but AusGeoid files do not store the latitude grid interval
		if (fabs(dlat_separation - m_pGridfile->ptrIndex[0].dLatinc) > TOLERANCE_SEC_MIN && fabs(dlat_separation - m_pGridfile->ptrIndex[0].dLatinc) < TOLERANCE_SEC_MAX)
			m_pGridfile->ptrIndex[0].dLatinc = dlat_separation;
	}
	// has there been an increment?	
	if ((dlon_separation - 0.0) > TOLERANCE_ZERO)
	{
		// is the increment of a significant size, but not excessive (i.e. limit the sepatation to two degrees)
		// Yes - this is fraught with disaster, but AusGeoid files do not store the longitude grid interval
		if (fabs(dlon_separation - m_pGridfile->ptrIndex[0].dLonginc) > TOLERANCE_SEC_MIN && fabs(dlon_separation - m_pGridfile->ptrIndex[0].dLonginc) < TOLERANCE_SEC_MAX)
			m_pGridfile->ptrIndex[0].dLonginc = dlon_separation;
	}
}

string dna_geoid_interpolation::ErrorString(const int& error, const string& data)
{
	switch(error)
	{
	case ERR_AUS_BINARY:	// -6
		return "The NTv2 version of the specified grid file is not supported.";
	case ERR_GRID_PARAMETERS:
		return "The parameters for the specified grid file do not match the number of records.";
	case ERR_GRID_MEMORY:	// -4
		return "Could not allocate the required memory.";
	case ERR_GRID_UNOPENED:	// -3
		return "A grid file has not been opened.";
	case ERR_FINDSUBGRID_GRIDERR:	// -2
		return "Cannot locate the required sub grid.";
	case ERR_GRIDFILE_READ:		// 1
		return "The specified grid file could not be opened.";
	case ERR_GRIDFILE_TYPE:		// 2
		return string(__BINARY_NAME__) + " cannot read this type of grid file.";
	case ERR_GRID_CORRUPT:		// 3
	case ERR_GRIDFILE_ERROR:	// 14
		return string(__BINARY_NAME__) + " found an unrecoverable error in the specified grid file:\n\n   " + 
			data + "\n\nIt is likely that this file was downloaded or produced incorrectly.\nPlease select a new grid file.";
	case ERR_INFILE_READ:		// 4
		return "Could not read from the specified input file.";
	case ERR_OUTFILE_WRITE:		// 5
		return "Could not write to the specified output file.";
	case ERR_INFILE_TYPE:		// 6
		return "Cannot read this type of input file.";
	case ERR_OUTFILE_TYPE:		// 7
		return "Cannot produce this type of output file.";
	case ERR_NODATA_READ:		// 8
		return "No data was contained within the last input record.";
	case ERR_LINE_TOO_SHORT:	// 9
		return "The record \"" + data + "\" is too short to contain valid data.";
	case ERR_INVALID_INPUT:		// 10
		return "The record \"" + data + "\" does not contain valid numeric input.";
	case ERR_READING_DATA:		// 11
		return "The required data could not be extracted from the record \"" + data + "\".";
	case ERR_INVALID_ZONE:		// 12
		return "The specified zone is invalid.";
	case ERR_PT_OUTSIDE_GRID:		// 13
	case ERR_FINDSUBGRID_OUTSIDE:	// -1
		return "The point \"" + data + "\" lies outside the extents of the specified grid file.";
	case ERR_READ_ASC_SHIFT:	// 15
		return "The interpolation shifts could not be retrieved from the Ascii grid file.";
	case ERR_READ_BIN_SHIFT:	// 16
		return "The interpolation shifts could not be retrieved from the Binary grid file.";
	case ERR_NUM_CSV_FIELDS:	// 17
		return "The csv record \"" + data + "\" does not contain sufficient records.";
	default:
		return "Unknown error";
	}
}
	
string dna_geoid_interpolation::ErrorCaption(const int& error)
{
	switch(error)
	{
	case ERR_AUS_BINARY:	// -6
		return "NTv2 version unsupported";
	case ERR_GRID_PARAMETERS:
		return "Grid file inconsistency";
	case ERR_GRID_MEMORY:	// -4
		return "Memory allocation failure";
	case ERR_GRID_UNOPENED:	// -3
		return "Grid file unopened";
	case ERR_FINDSUBGRID_GRIDERR:	// -2
		return "Sub grid not found";
	case ERR_GRIDFILE_READ:		// 1
		return "Grid file I/O (read) error";
	case ERR_GRIDFILE_TYPE:		// 2
		return "Unknown grid file type";
	case ERR_GRID_CORRUPT:		// 3
	case ERR_GRIDFILE_ERROR:	// 14
		return "Grid file corrupt";
	case ERR_INFILE_READ:		// 4
	case ERR_OUTFILE_WRITE:		// 5
		return "File I/O error";
	case ERR_INFILE_TYPE:		// 6
	case ERR_OUTFILE_TYPE:		// 7
		return "Unknown file type";
	case ERR_NODATA_READ:		// 8
	case ERR_LINE_TOO_SHORT:	// 9
		return "Insufficient data";
	case ERR_INVALID_INPUT:		// 10
		return "Invalid input data";
	case ERR_READING_DATA:		// 11
		return "Data read error";
	case ERR_INVALID_ZONE:		// 12
		return "Invalid zone";
	case ERR_PT_OUTSIDE_GRID:		// 13
	case ERR_FINDSUBGRID_OUTSIDE:	// -1
		return "Point outside grid";
	case ERR_READ_ASC_SHIFT:	// 15
	case ERR_READ_BIN_SHIFT:	// 16
		return "Grid shift read error";
	case ERR_NUM_CSV_FIELDS:	// 17
		return "Insufficient CSV records";
	default:
		return "Error";
	}
}

}	// namespace networksegment
}	// namespace dynadjust

