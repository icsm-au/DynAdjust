//============================================================================
// Name         : dnaangle.cpp
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
// Description  : CDnaAngle implementation file
//============================================================================

#include <include/measurement_types/dnaangle.hpp>

namespace dynadjust {
namespace measurements {

CDnaAngle::CDnaAngle(void)
	: m_strTarget("")
	, m_strTarget2("")
	, m_drValue(0.)
	, m_dStdDev(0.)
{
	// from abstract class
	m_strType = "A";
	m_MSmeasurementStations = THREE_STATION;
}

CDnaAngle::~CDnaAngle(void)
{
}

// copy constructors
CDnaAngle::CDnaAngle(const CDnaAngle& newAngle)
{
	m_strType = newAngle.m_strType;
	m_bIgnore = newAngle.m_bIgnore;
	m_strFirst = newAngle.m_strFirst;
	m_strTarget = newAngle.m_strTarget;
	m_strTarget2 = newAngle.m_strTarget2;
	m_drValue = newAngle.m_drValue;
	m_dStdDev = newAngle.m_dStdDev;
	m_MSmeasurementStations = newAngle.m_MSmeasurementStations;
}


CDnaAngle::CDnaAngle(const bool strIgnore, const string& strFirst, const string& strTarget, const string& strTarget2, const double& drValue, const double& dStdDev)
{
	m_strType = "A";
	m_bIgnore = strIgnore;
	m_strFirst = strFirst;
	m_strTarget = strTarget;
	m_strTarget2 = strTarget2;
	m_drValue = drValue;
	m_dStdDev = dStdDev;


}


CDnaAngle& CDnaAngle::operator= (const CDnaAngle& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_strTarget2 = rhs.m_strTarget2;
	m_drValue = rhs.m_drValue;
	m_dStdDev = rhs.m_dStdDev;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	return *this;
}


bool CDnaAngle::operator== (const CDnaAngle& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strType == rhs.m_strType &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_strTarget2 == rhs.m_strTarget2 &&
		m_drValue == rhs.m_drValue &&
		m_dStdDev == rhs.m_dStdDev
		);
}

bool CDnaAngle::operator< (const CDnaAngle& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {		
		if (m_strTarget == rhs.m_strTarget) {
			if (m_strTarget2 == rhs.m_strTarget2) {
				if (m_bIgnore == rhs.m_bIgnore) {
					if (m_drValue == rhs.m_drValue)
						return m_dStdDev < rhs.m_dStdDev; 
					else
						return m_drValue < rhs.m_drValue; }
				else
					return m_bIgnore < rhs.m_bIgnore; }		
			else
				return m_strTarget2 < rhs.m_strTarget2; }
		else
			return m_strTarget < rhs.m_strTarget; }
		
	else
		return m_strFirst < rhs.m_strFirst;
}

void CDnaAngle::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	os << setw(TARG_WIDTH) << m_strTarget;
	os << setw(28) << m_strTarget2;
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << setprecision(9) << fixed << RadtoDms(m_drValue);
	os << setw(VAR_WIDTH) << fixed << setprecision(3) << Seconds(m_dStdDev);
	os << setw(7) << " ";
	os << setw(7) << " ";
	os << setw(7) << m_lstn1Index << setw(7) << m_lstn2Index << setw(7) << m_lstn3Index;
	os << endl;
}
	

void CDnaAngle::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << "  <!--Type " << measurement_name<char, string>(GetTypeC()) << "-->" << endl;
	*dynaml_stream << "  <DnaMeasurement>" << endl;
	*dynaml_stream << "    <Type>" << m_strType << "</Type>" << endl;
	// Source file from which the measurement came
	*dynaml_stream << "    <Source>" << m_sourceFile << "</Source>" << endl;
	if (m_bIgnore)
		*dynaml_stream << "    <Ignore>*</Ignore>" << endl;
	else
		*dynaml_stream << "    <Ignore/>" << endl;
	*dynaml_stream << "    <First>" << m_strFirst << "</First>" << endl;
	*dynaml_stream << "    <Second>" << m_strTarget << "</Second>" << endl;
	*dynaml_stream << "    <Third>" << m_strTarget2 << "</Third>" << endl;
	*dynaml_stream << "    <Value>" << setprecision(8) << fixed << RadtoDms(m_drValue) << "</Value>" << endl;
	*dynaml_stream << "    <StdDev>" << scientific << setprecision(6) << Seconds(m_dStdDev) << "</StdDev>" << endl;
	if (m_databaseIdSet)
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}
	

void CDnaAngle::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";
	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << left << setw(dmw.msr_targ1) << m_strTarget;
	*dynaml_stream << left << setw(dmw.msr_targ2) << m_strTarget2;
	*dynaml_stream << setw(dmw.msr_linear) << " ";	// linear measurement value
	*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << 
		right << FormatDnaDmsString(RadtoDms(m_drValue), 8);
	*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(3) << Seconds(m_dStdDev);
	
	if (m_databaseIdSet)
	{ 
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_inst_ht) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
	}
	
	*dynaml_stream << endl;
}
	

void CDnaAngle::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	_it_vdnastnptr stn1_it(vStations->begin() + m_lstn1Index);
	_it_vdnastnptr stn2_it(vStations->begin() + m_lstn2Index);
	_it_vdnastnptr stn3_it(vStations->begin() + m_lstn3Index);

	double direction12, direction13;

	// calculated angle
	m_drValue = HorizontalAngle(
		stn1_it->get()->GetXAxis(),
		stn1_it->get()->GetYAxis(),
		stn1_it->get()->GetZAxis(),
		stn2_it->get()->GetXAxis(),
		stn2_it->get()->GetYAxis(),
		stn2_it->get()->GetZAxis(),
		stn3_it->get()->GetXAxis(),
		stn3_it->get()->GetYAxis(),
		stn3_it->get()->GetZAxis(),
		stn1_it->get()->GetcurrentLatitude(),
		stn1_it->get()->GetcurrentLongitude(),
		&direction12, &direction13);

	// Deflections available?
	if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
	{
		/////////////////////////////////////////////////////////////////////////////////
		// Angles (observed or derived from directions) must be corrected for deflection 
		// of the vertical via "Laplace correction".  This correction requires zenith 
		// distance (zenith12, zenith13) and geodetic azimuth (direction12, direction13), 
		// both of which must be computed from coordinates.
		
		// 1. Compute zenith distance 1 -> 2
		double zenith12(ZenithDistance<double>(
			stn1_it->get()->GetXAxis(),
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(),
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn2_it->get()->GetcurrentLongitude(),
			0.0,									// instrument height
			0.0));									// target height

		// 2. Compute zenith distance 1 -> 3
		double zenith13(ZenithDistance<double>(
			stn1_it->get()->GetXAxis(),
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn3_it->get()->GetXAxis(),
			stn3_it->get()->GetYAxis(),
			stn3_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn3_it->get()->GetcurrentLatitude(),
			stn3_it->get()->GetcurrentLongitude(),
			0.0,									// instrument height
			0.0));									// target height

		// 3. Laplace correction 1 -> 2 -> 3
		m_preAdjCorr = HzAngleDeflectionCorrection<double>(		
			direction12,							// geodetic azimuth 1 -> 2
			zenith12,								// zenith distance 1 -> 2
			direction13,							// geodetic azimuth 1 -> 3
			zenith13,								// zenith distance 1 -> 3
			stn1_it->get()->GetverticalDef(),		// deflection in prime vertical
			stn1_it->get()->GetmeridianDef());		// deflection in prime meridian

		// 4. apply deflection correction
		m_drValue += m_preAdjCorr;
	}

	// compute standard deviation in radians
	m_dStdDev = SecondstoRadians(0.01);
}
	

UINT32 CDnaAngle::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	char stationName[STN_NAME_WIDTH];
	m_strType = measRecord->measType;
	m_bIgnore = measRecord->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
	
	// first station
	m_lstn1Index = measRecord->station1;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strFirst = stationName;
	// target station
	m_lstn2Index = measRecord->station2;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station2 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strTarget = stationName;
	// target2 station
	m_lstn3Index = measRecord->station3;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station3 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strTarget2 = stationName;
	
	m_measAdj = measRecord->measAdj;
	m_measCorr = measRecord->measCorr;
	m_measAdjPrec = measRecord->measAdjPrec;
	m_residualPrec = measRecord->residualPrec;
	m_preAdjCorr = measRecord->preAdjCorr;
	m_drValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	return 0;
}
	

UINT32 CDnaAngle::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_strType = it_msr->measType;
	m_bIgnore = it_msr->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	
	// first station
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;

	// target station
	m_lstn2Index = it_msr->station2;
	m_strTarget = binaryStn.at(it_msr->station2).stationName;
	
	// target2 station
	m_lstn3Index = it_msr->station3;
	m_strTarget2 = binaryStn.at(it_msr->station3).stationName;
	
	m_measAdj = it_msr->measAdj;
	m_measCorr = it_msr->measCorr;
	m_measAdjPrec = it_msr->measAdjPrec;
	m_residualPrec = it_msr->residualPrec;
	m_preAdjCorr = it_msr->preAdjCorr;
	m_drValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);
	return 0;
}

void CDnaAngle::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.station2 = m_lstn2Index;
	measRecord.station3 = m_lstn3Index;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_drValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}

void CDnaAngle::SetValue(const string& str)
{
	// convert to radians
	RadFromDmsString(&m_drValue, trimstr(str));
}

void CDnaAngle::SetStdDev(const string& str)
{
	// convert seconds to radians
	RadFromSecondsString(&m_dStdDev, trimstr(str));
}

}	// namespace measurements
}	// namespace dynadjust
