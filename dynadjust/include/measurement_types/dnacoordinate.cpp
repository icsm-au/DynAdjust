//============================================================================
// Name         : dnacoordinate.cpp
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
// Description  : CDnaCoordinate implementation file
//============================================================================

#include <include/measurement_types/dnacoordinate.hpp>

namespace dynadjust {
namespace measurements {

CDnaCoordinate::CDnaCoordinate(void)
	: m_drValue(0.)
	, m_dStdDev(0.)
{
	m_MSmeasurementStations = ONE_STATION;
}


CDnaCoordinate::~CDnaCoordinate(void)
{

}


CDnaCoordinate::CDnaCoordinate(const CDnaCoordinate& newDistance)
{
	m_strFirst = newDistance.m_strFirst;
	m_drValue = newDistance.m_drValue;
	m_dStdDev = newDistance.m_dStdDev;
	m_bIgnore = newDistance.m_bIgnore;
	m_MSmeasurementStations = newDistance.m_MSmeasurementStations;

}


CDnaCoordinate::CDnaCoordinate(const bool bIgnore, const string& strFirst, const double& dValue, const double& dStdDev)
{
	m_strFirst = strFirst;
	m_bIgnore = bIgnore;
	m_drValue = dValue;
	m_dStdDev = dStdDev;
}


CDnaCoordinate& CDnaCoordinate::operator= (const CDnaCoordinate& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_drValue = rhs.m_drValue;
	m_dStdDev = rhs.m_dStdDev;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	return *this;
}


bool CDnaCoordinate::operator== (const CDnaCoordinate& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strType == rhs.m_strType &&
		m_bIgnore == rhs.m_bIgnore &&
		m_drValue == rhs.m_drValue &&
		m_dStdDev == rhs.m_dStdDev
		);
}

bool CDnaCoordinate::operator< (const CDnaCoordinate& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// could be P, Q, J, K
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_drValue == rhs.m_drValue)
					return m_dStdDev < rhs.m_dStdDev; 
				else
					return m_drValue < rhs.m_drValue; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}


void CDnaCoordinate::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	os << setw(TARG_WIDTH) << "";
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << m_drValue;
	os << endl;
}
	

void CDnaCoordinate::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
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
	
	*dynaml_stream << "    <Value>" << fixed << setprecision(10);
	switch (GetTypeC())
	{
	case 'P':
	case 'I':		// Latitude requires Degrees(...)
		*dynaml_stream << RadtoDms(m_drValue);
		break;
	case 'Q':
	case 'J':		// Longitude requires DegreesL(...)
		*dynaml_stream << RadtoDmsL(m_drValue);
		break;
	}

	*dynaml_stream << "</Value>" << endl;
	
	*dynaml_stream << "    <StdDev>" << scientific << setprecision(6) << Seconds(m_dStdDev) << "</StdDev>" << endl;
	if (m_databaseIdSet)
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}
	

void CDnaCoordinate::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";
	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << setw(dmw.msr_targ1) << " ";	// first target
	*dynaml_stream << setw(dmw.msr_targ2) << " ";	// second target
	*dynaml_stream << right << setw(dmw.msr_linear) << fixed << setprecision(9) << RadtoDms(m_drValue);	// linear measurement value
	*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << " ";
	*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(6) << Seconds(m_dStdDev);

	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_inst_ht) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
	}

	*dynaml_stream << endl;
}
	

void CDnaCoordinate::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	switch (GetTypeC())
	{
	case 'P':
	case 'I':
		m_drValue = vStations->at(m_lstn1Index).get()->GetcurrentLatitude();
		break;
	case 'Q':
	case 'J':
		m_drValue = vStations->at(m_lstn1Index).get()->GetcurrentLongitude();
		break;
	}

	switch (GetTypeC())
	{
	case 'I':
		// convert to astro (requires geoid)
		if (fabs(vStations->at(m_lstn1Index).get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
			m_drValue = vStations->at(m_lstn1Index).get()->GetcurrentLatitude() +
				vStations->at(m_lstn1Index).get()->GetmeridianDef();
		break;
	case 'J':
		// convert to astro (requires geoid)
		if (fabs(vStations->at(m_lstn1Index).get()->GetverticalDef()) > E4_SEC_DEFLECTION)
			m_drValue = vStations->at(m_lstn1Index).get()->GetcurrentLongitude() +
				vStations->at(m_lstn1Index).get()->GetverticalDef() / 
				cos(vStations->at(m_lstn1Index).get()->GetcurrentLatitude());
		break;
	}

	// compute standard deviation in radians
	m_dStdDev = SecondstoRadians(0.021);
}
	

UINT32 CDnaCoordinate::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	char stationName[STN_NAME_WIDTH];
	m_strType = measRecord->measType;
	m_bIgnore = measRecord->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;

	m_lstn1Index = measRecord->station1;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strFirst = stationName;
	
	m_measAdj = measRecord->measAdj;
	m_measCorr = measRecord->measCorr;
	m_measAdjPrec = measRecord->measAdjPrec;
	m_residualPrec = measRecord->residualPrec;
	m_preAdjCorr = measRecord->preAdjCorr;
	m_drValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	return 0;
}
	

UINT32 CDnaCoordinate::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_strType = it_msr->measType;
	m_bIgnore = it_msr->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;
	
	m_measAdj = it_msr->measAdj;
	m_measCorr = it_msr->measCorr;
	m_measAdjPrec = it_msr->measAdjPrec;
	m_residualPrec = it_msr->residualPrec;
	m_preAdjCorr = it_msr->preAdjCorr;
	m_drValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);
	return 0;
}

void CDnaCoordinate::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_drValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	measRecord.term3 = measRecord.term4 = 0.;
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}


void CDnaCoordinate::SetValue(const string& str)
{
	RadFromDmsString(&m_drValue, trimstr(str));
}

void CDnaCoordinate::SetStdDev(const string& str)
{
	RadFromSecondsString(&m_dStdDev, trimstr(str));			// convert to radians
}


}	// namespace measurements
}	// namespace dynadjust
