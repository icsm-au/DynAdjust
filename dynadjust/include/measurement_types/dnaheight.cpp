//============================================================================
// Name         : dnaheight.cpp
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
// Description  : CDnaHeight implementation file
//============================================================================

#include <include/measurement_types/dnaheight.hpp>

namespace dynadjust {
namespace measurements {

CDnaHeight::CDnaHeight(void)
	: m_dValue(0.)
	, m_dStdDev(0.)
{
	m_strType = "H";		// default is orthometric distance, but could also be 'R'
	m_MSmeasurementStations = ONE_STATION;
}


CDnaHeight::~CDnaHeight(void)
{

}


CDnaHeight::CDnaHeight(const CDnaHeight& newHeight)
{
	m_strFirst = newHeight.m_strFirst;
	m_strType = newHeight.m_strType;
	m_bIgnore = newHeight.m_bIgnore;
	m_dValue = newHeight.m_dValue;
	m_dStdDev = newHeight.m_dStdDev;
	m_MSmeasurementStations = newHeight.m_MSmeasurementStations;
}


CDnaHeight::CDnaHeight(const bool bIgnore, const string& strType, const string& strFirst, const double& dValue, const double& dStdDev)
{
	m_strFirst = strFirst;
	m_strType = strType;
	m_bIgnore = bIgnore;
	m_dValue = dValue;
	m_dStdDev = dStdDev;
}


CDnaHeight& CDnaHeight::operator= (const CDnaHeight& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_dValue = rhs.m_dValue;
	m_dStdDev = rhs.m_dStdDev;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	return *this;
}


bool CDnaHeight::operator== (const CDnaHeight& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strType == rhs.m_strType &&
		m_bIgnore == rhs.m_bIgnore &&
		m_dValue == rhs.m_dValue &&
		m_dStdDev == rhs.m_dStdDev
		);
}

bool CDnaHeight::operator< (const CDnaHeight& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// don't think this is needed
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_dValue == rhs.m_dValue)
					return m_dStdDev < rhs.m_dStdDev; 
				else
					return m_dValue < rhs.m_dValue; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}


void CDnaHeight::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	os << setw(TARG_WIDTH) << " ";
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << m_dValue;
	os << setw(VAR_WIDTH) << m_dStdDev;
	os << endl;
}
	

void CDnaHeight::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
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
	*dynaml_stream << "    <Value>" << fixed << setprecision(4) << m_dValue << "</Value>" << endl;
	*dynaml_stream << "    <StdDev>" << scientific << setprecision(6) << m_dStdDev << "</StdDev>" << endl;
	if (m_databaseIdSet)
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}
	

void CDnaHeight::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";

	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << setw(dmw.msr_targ1) << " ";
	*dynaml_stream << setw(dmw.msr_targ2) << " ";
	*dynaml_stream << right << setw(dmw.msr_linear) << fixed << setprecision(4) << m_dValue;	// linear measurement value
	*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << " ";
	*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(6) << m_dStdDev;

	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_inst_ht) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
	}

	*dynaml_stream << endl;
}
	

void CDnaHeight::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	// Zn is the z coordinate element of the point on the z-axis 
	// which intersects with the the normal at the given Latitude
	double nu1, Zn1;

	// calculated height
	m_dValue = EllipsoidHeight<double>(
		vStations->at(m_lstn1Index).get()->GetXAxis(), 
		vStations->at(m_lstn1Index).get()->GetYAxis(),
		vStations->at(m_lstn1Index).get()->GetZAxis(),
		vStations->at(m_lstn1Index).get()->GetcurrentLatitude(),
		&nu1, &Zn1, ellipsoid);	

	m_dStdDev = 0.024;

	switch (GetTypeC())
	{
	case 'H':
		// N value available?
		if (fabs(vStations->at(m_lstn1Index).get()->GetgeoidSep()) > PRECISION_1E4)
		{
			// reduce to orthometric height
			m_preAdjCorr = vStations->at(m_lstn1Index).get()->GetgeoidSep();
			m_dValue -= m_preAdjCorr;
		}
		break;
	}
}
	

UINT32 CDnaHeight::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
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
	m_dValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	return 0;
}

UINT32 CDnaHeight::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
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
	m_dValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);
	return 0;
}
	

void CDnaHeight::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
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
	measRecord.term1 = m_dValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	measRecord.term3 = 0.;
	measRecord.term4 = 0.;
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}


void CDnaHeight::SetValue(const string& str)
{
	DoubleFromString(m_dValue, trimstr(str));
}

void CDnaHeight::SetStdDev(const string& str)
{
	DoubleFromString(m_dStdDev, trimstr(str));
}


}	// namespace measurements
}	// namespace dynadjust
