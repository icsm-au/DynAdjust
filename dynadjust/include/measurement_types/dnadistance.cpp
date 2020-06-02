//============================================================================
// Name         : dnadistance.cpp
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
// Description  : CDnaDistance implementation file
//============================================================================

#include <include/measurement_types/dnadistance.hpp>

namespace dynadjust {
namespace measurements {

CDnaDistance::CDnaDistance(void)
	: m_strTarget("")
	, m_dValue(0.)
	, m_dStdDev(0.)
	, m_fInstHeight(0.)
	, m_fTargHeight(0.)
{
	m_strType = "S";		// default is Slope distance, but could also be 'C' or 'E' or 'M'
	m_MSmeasurementStations = TWO_STATION;
}


CDnaDistance::~CDnaDistance(void)
{

}


CDnaDistance::CDnaDistance(const CDnaDistance& newDistance)
{
	m_strType = newDistance.m_strType;
	m_strFirst = newDistance.m_strFirst;
	m_bIgnore = newDistance.m_bIgnore;
	m_strTarget = newDistance.m_strTarget;
	m_dValue = newDistance.m_dValue;
	m_dStdDev = newDistance.m_dStdDev;
	m_fInstHeight = newDistance.m_fInstHeight;
	m_fTargHeight = newDistance.m_fTargHeight;
	m_MSmeasurementStations = newDistance.m_MSmeasurementStations;

}


CDnaDistance::CDnaDistance(const bool bIgnore, const string& strType, const string& strFirst, const string& strTarget, const double& dValue, const double& dStdDev)
{
	m_strType = strType;
	m_strFirst = strFirst;
	m_bIgnore = bIgnore;
	m_strTarget = strTarget;
	m_dValue = dValue;
	m_dStdDev = dStdDev;
}


CDnaDistance& CDnaDistance::operator= (const CDnaDistance& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_dValue = rhs.m_dValue;
	m_dStdDev = rhs.m_dStdDev;
	m_fInstHeight = rhs.m_fInstHeight;
	m_fTargHeight = rhs.m_fTargHeight;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	return *this;
}


bool CDnaDistance::operator== (const CDnaDistance& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_dValue == rhs.m_dValue &&
		m_dStdDev == rhs.m_dStdDev &&
		m_strType == rhs.m_strType &&
		m_fInstHeight == rhs.m_fInstHeight &&
		m_fTargHeight == rhs.m_fTargHeight
		);
}


bool CDnaDistance::operator< (const CDnaDistance& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// could be C, E, M, S
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_strTarget == rhs.m_strTarget) {
					if (m_dValue == rhs.m_dValue) {
						if (m_dStdDev == rhs.m_dStdDev) {
							if (m_fInstHeight == rhs.m_fInstHeight)
								return m_fTargHeight < rhs.m_fTargHeight;
							else
								return m_fInstHeight < rhs.m_fInstHeight; }
						else
							return m_dStdDev < rhs.m_dStdDev; }
					else
						return m_dValue < rhs.m_dValue; }
				else
					return m_strTarget < rhs.m_strTarget; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}


void CDnaDistance::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	os << setw(TARG_WIDTH) << m_strTarget;
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << setprecision(4) << fixed << m_dValue;
	os << setw(VAR_WIDTH) << setprecision(3) << fixed << m_dStdDev;
	switch (GetTypeC())
	{
	case 'S':
		os << setw(7) << setprecision(3) << fixed << m_fInstHeight << 
			setw(7) << setprecision(3) << fixed << m_fTargHeight;
		break;
	default:
		os << setw(7) << " " << setw(7) << " ";
		break;
	}
	os << endl;
}
	

void CDnaDistance::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
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
	*dynaml_stream << "    <Value>" << fixed << setprecision(4) << m_dValue << "</Value>" << endl;
	*dynaml_stream << "    <StdDev>" << fixed << setprecision(4) << m_dStdDev << "</StdDev>" << endl;

	switch (GetTypeC())
	{
	case 'S':
		*dynaml_stream << "    <InstHeight>" << fixed << setprecision(3) << m_fInstHeight << "</InstHeight>" << endl;
		*dynaml_stream << "    <TargHeight>" << fixed << setprecision(3) << m_fTargHeight << "</TargHeight>" << endl;
		break;
	}
		
	if (m_databaseIdSet)
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}
	

void CDnaDistance::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";

	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << left << setw(dmw.msr_targ1) << m_strTarget;
	*dynaml_stream << setw(dmw.msr_targ2) << " ";
	*dynaml_stream << right << setw(dmw.msr_linear) << fixed << setprecision(4) << m_dValue;
	*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << " ";
	*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(3) << m_dStdDev;
	
	// database id width
	UINT32 width(dml.msr_id_msr - dml.msr_inst_ht);
	
	// Slope distance
	switch (GetTypeC())
	{
	case 'S':
		*dynaml_stream << setw(dmw.msr_inst_ht) << fixed << setprecision(3) << m_fInstHeight;
		*dynaml_stream << setw(dmw.msr_targ_ht) << fixed << setprecision(3) << m_fTargHeight;
		width = dml.msr_id_msr - dml.msr_targ_ht - dmw.msr_targ_ht;
		break;
	}
	
	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(width) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
	}

	*dynaml_stream << endl;
}
	

void CDnaDistance::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	double dX, dY, dZ;
	double dXih, dYih, dZih, dXth, dYth, dZth;

	_it_vdnastnptr stn1_it(vStations->begin() + m_lstn1Index);
	_it_vdnastnptr stn2_it(vStations->begin() + m_lstn2Index);

	m_fInstHeight = float(0.000);
	m_fTargHeight = float(0.000);

	switch (GetTypeC())
	{
	case 'S':
		CartesianElementsFromInstrumentHeight<double>(m_fInstHeight,				// instrument height
			&dXih, &dYih, &dZih, 
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude());
		CartesianElementsFromInstrumentHeight<double>(m_fTargHeight,				// target height
			&dXth, &dYth, &dZth,
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude());
		dX = (stn2_it->get()->GetXAxis() - stn1_it->get()->GetXAxis() + dXth - dXih);
		dY = (stn2_it->get()->GetYAxis() - stn1_it->get()->GetYAxis() + dYth - dYih);
		dZ = (stn2_it->get()->GetZAxis() - stn1_it->get()->GetZAxis() + dZth - dZih);

		m_dValue = magnitude(dX, dY, dZ);
		break;
	case 'C':
		m_dValue = EllipsoidChordDistance<double>(
			stn1_it->get()->GetXAxis(),
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(),
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentHeight(),
			stn2_it->get()->GetcurrentHeight(),
			&dX, &dY, &dZ, ellipsoid);
		break;
	case 'E':		
		m_dValue = EllipsoidArcDistance<double>(
			stn1_it->get()->GetXAxis(),
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(),
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentHeight(),
			stn2_it->get()->GetcurrentHeight(),
			ellipsoid);
		break;
	case 'M':		
		m_dValue = MSLArcDistance<double>(
			stn1_it->get()->GetXAxis(),
			stn1_it->get()->GetYAxis(),
			stn1_it->get()->GetZAxis(),
			stn2_it->get()->GetXAxis(),
			stn2_it->get()->GetYAxis(),
			stn2_it->get()->GetZAxis(),
			stn1_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentLongitude(),
			stn2_it->get()->GetcurrentLatitude(),
			stn1_it->get()->GetcurrentHeight(),
			stn2_it->get()->GetcurrentHeight(),
			stn1_it->get()->GetgeoidSep(),
			stn2_it->get()->GetgeoidSep(),
			ellipsoid);
		break;
	}

	m_dStdDev = 3.0 * sqrt(m_dValue / 1000.0) / 100.0;
}
	

UINT32 CDnaDistance::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
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
	
	m_measAdj = measRecord->measAdj;
	m_measCorr = measRecord->measCorr;
	m_measAdjPrec = measRecord->measAdjPrec;
	m_residualPrec = measRecord->residualPrec;
	m_preAdjCorr = measRecord->preAdjCorr;
	m_dValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	m_fInstHeight = static_cast<float> (measRecord->term3);
	m_fTargHeight = static_cast<float> (measRecord->term4);
	return 0;
}
	

UINT32 CDnaDistance::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_bIgnore = it_msr->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	
	m_strType = it_msr->measType;
	
	// first station
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;
	
	if (GetTypeC() == 'S')		// slope distance
	{
		// inst and targ heights
		m_fInstHeight = static_cast<float> (it_msr->term3);
		m_fTargHeight = static_cast<float> (it_msr->term4);
	}

	// target station
	m_lstn2Index = it_msr->station2;
	m_strTarget = binaryStn.at(it_msr->station2).stationName;
	
	m_measAdj = it_msr->measAdj;
	m_measCorr = it_msr->measCorr;
	m_measAdjPrec = it_msr->measAdjPrec;
	m_residualPrec = it_msr->residualPrec;
	m_preAdjCorr = it_msr->preAdjCorr;
	m_dValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);

	return 0;
}
	

void CDnaDistance::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.station2 = m_lstn2Index;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_dValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	measRecord.term3 = m_fInstHeight;
	measRecord.term4 = m_fTargHeight;
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}


void CDnaDistance::SetValue(const string& str)
{
	DoubleFromString(m_dValue, trimstr(str));
}

void CDnaDistance::SetStdDev(const string& str)
{
	DoubleFromString(m_dStdDev, trimstr(str));
}

// Instrument and target heights only make sense for 
// slope distances, vertical angles and zenith distances.
// Note: slope distances are derived from CDnaDistance, so
// these methods are provided here, but don't need 
// instrument height for 'C', 'E' and 'M'  measurements.
void CDnaDistance::SetInstrumentHeight(const string& str)
{
	FloatFromString(m_fInstHeight, trimstr(str));
}

void CDnaDistance::SetTargetHeight(const string& str)
{
	DoubleFromString(m_fTargHeight, trimstr(str));
}


}	// namespace measurements
}	// namespace dynadjust
