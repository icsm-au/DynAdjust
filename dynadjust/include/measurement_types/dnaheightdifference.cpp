//============================================================================
// Name         : dnaheightdifference.cpp
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
// Description  : CDnaHeightDifference implementation file
//============================================================================

#include <include/measurement_types/dnaheightdifference.hpp>

//extern boost::random::mt19937 rng;
//extern boost::random::uniform_real_distribution<double> stdev;
//extern boost::random::uniform_real_distribution<double> pertu;

namespace dynadjust {
namespace measurements {

CDnaHeightDifference::CDnaHeightDifference(void)
	: m_strTarget("")
	, m_dValue(0.)
	, m_dStdDev(0.)
{
	m_strType = "L";		// default is Slope distance, but could also be 'C' or 'E' or 'M'
	m_MSmeasurementStations = TWO_STATION;
}


CDnaHeightDifference::~CDnaHeightDifference(void)
{

}


// copy constructor (disallowed)
//CDnaHeightDifference::CDnaHeightDifference(const CDnaHeightDifference& newHeightDifference)
//{
//	m_strFirst = newHeightDifference.m_strFirst;
//	m_strTarget = newHeightDifference.m_strTarget;
//	m_strType = newHeightDifference.m_strType;
//	m_bIgnore = newHeightDifference.m_bIgnore;
//	m_dValue = newHeightDifference.m_dValue;
//	m_dStdDev = newHeightDifference.m_dStdDev;
//	m_MSmeasurementStations = newHeightDifference.m_MSmeasurementStations;
//}


//CDnaHeightDifference::CDnaHeightDifference(const bool bIgnore, const string& strType, const string& strFirst, const string& strTarget, const double& dValue, const double& dStdDev)
//{
//	m_strFirst = strFirst;
//	m_strTarget = strTarget;
//	m_strType = strType;
//	m_bIgnore = bIgnore;
//	m_dValue = dValue;
//	m_dStdDev = dStdDev;
//}


//CDnaHeightDifference& CDnaHeightDifference::operator= (const CDnaHeightDifference& rhs)
//{
//	// check for assignment to self!
//	if (this == &rhs)
//		return *this;
//
//	CDnaMeasurement::operator=(rhs);
//	m_strTarget = rhs.m_strTarget;
//	m_dValue = rhs.m_dValue;
//	m_dStdDev = rhs.m_dStdDev;
//	m_MSmeasurementStations = rhs.m_MSmeasurementStations;
//
//	return *this;
//}


bool CDnaHeightDifference::operator== (const CDnaHeightDifference& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strTarget == rhs.m_strTarget &&
		m_strType == rhs.m_strType &&
		m_bIgnore == rhs.m_bIgnore &&
		m_dValue == rhs.m_dValue &&
		m_dStdDev == rhs.m_dStdDev &&
		m_epoch == rhs.m_epoch
		);
}


bool CDnaHeightDifference::operator< (const CDnaHeightDifference& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// don't think this is needed
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_epoch == rhs.m_epoch) {
					if (m_strTarget == rhs.m_strTarget) {
						if (m_dValue == rhs.m_dValue)
							return m_dStdDev < rhs.m_dStdDev; 
						else
							return m_dValue < rhs.m_dValue; }
					else
						return m_strTarget < rhs.m_strTarget; }
				else
					return m_epoch < rhs.m_epoch; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}
	

void CDnaHeightDifference::WriteDynaMLMsr(std::ofstream* dynaml_stream, const string& comment, bool) const
{
	if (comment.empty())
		*dynaml_stream << "  <!-- Type " << measurement_name<char, string>(GetTypeC()) << " -->" << endl;
	else
		*dynaml_stream << "  <!-- " << comment << " -->" << endl;

	*dynaml_stream << "  <DnaMeasurement>" << endl;
	*dynaml_stream << "    <Type>" << m_strType << "</Type>" << endl;
	// Source file from which the measurement came
	*dynaml_stream << "    <Source>" << m_sourceFile << "</Source>" << endl;
	if (m_bIgnore)
		*dynaml_stream << "    <Ignore>*</Ignore>" << endl;
	else
		*dynaml_stream << "    <Ignore/>" << endl;
	
	if (m_epoch.empty())
		*dynaml_stream << "    <Epoch/>" << endl;
	else
		*dynaml_stream << "    <Epoch>" << m_epoch << "</Epoch>" << endl;

	*dynaml_stream << "    <First>" << m_strFirst << "</First>" << endl;
	*dynaml_stream << "    <Second>" << m_strTarget << "</Second>" << endl;
	*dynaml_stream << "    <Value>" << fixed << setprecision(4) << m_dValue << "</Value>" << endl;
	*dynaml_stream << "    <StdDev>" << fixed << setprecision(6) << m_dStdDev << "</StdDev>" << endl;
	if (m_databaseIdSet)
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}


void CDnaHeightDifference::WriteDNAMsr(std::ofstream* dna_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool) const
{
	*dna_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dna_stream << setw(dmw.msr_ignore) << "*";
	else
		*dna_stream << setw(dmw.msr_ignore) << " ";

	*dna_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dna_stream << left << setw(dmw.msr_targ1) << m_strTarget;
	*dna_stream << setw(dmw.msr_targ2) << " ";
	*dna_stream << right << setw(dmw.msr_linear) << fixed << setprecision(4) << m_dValue;	// linear measurement value
	*dna_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << " ";
	*dna_stream << setw(dmw.msr_stddev) << fixed << setprecision(6) << m_dStdDev;

	*dna_stream << setw(dml.msr_gps_epoch - dml.msr_inst_ht) << " ";
	*dna_stream << setw(dmw.msr_gps_epoch) << m_epoch;

	if (m_databaseIdSet)
	{
		*dna_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
	}

	*dna_stream << endl;
}
	

void CDnaHeightDifference::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	// Zn is the z coordinate element of the point on the z-axis 
	// which intersects with the the normal at the given Latitude
	double h1, h2, nu1, Zn1, nu2, Zn2;

	// calculated height
	m_dValue = EllipsoidHeightDifference<double>(
		vStations->at(m_lstn1Index).get()->GetXAxis(), 
		vStations->at(m_lstn1Index).get()->GetYAxis(),
		vStations->at(m_lstn1Index).get()->GetZAxis(),
		vStations->at(m_lstn2Index).get()->GetXAxis(), 
		vStations->at(m_lstn2Index).get()->GetYAxis(),
		vStations->at(m_lstn2Index).get()->GetZAxis(),
		vStations->at(m_lstn1Index).get()->GetcurrentLatitude(),
		vStations->at(m_lstn2Index).get()->GetcurrentLatitude(),
		&h1, &h2, &nu1, &Zn1, &nu2, &Zn2, ellipsoid);

	double distance = GreatCircleDistance<double>(
		vStations->at(m_lstn1Index).get()->GetcurrentLatitude(),
		vStations->at(m_lstn1Index).get()->GetcurrentLongitude(),
		vStations->at(m_lstn2Index).get()->GetcurrentLatitude(),
		vStations->at(m_lstn2Index).get()->GetcurrentLongitude());

	m_dStdDev = 3.0 * sqrt(distance / 1000.0) / 100.0;

	m_epoch = "01.10.1985";

	// N value available?
	if (fabs(vStations->at(m_lstn1Index).get()->GetgeoidSep()) > PRECISION_1E4 ||
		fabs(vStations->at(m_lstn2Index).get()->GetgeoidSep()) > PRECISION_1E4)
	{
		// reduce to orthometric height difference
		m_preAdjCorr = vStations->at(m_lstn2Index).get()->GetgeoidSep() - vStations->at(m_lstn1Index).get()->GetgeoidSep();
		m_dValue -= m_preAdjCorr;
	}

    // TODO - add option to perturb all measurements and standard deviations by a small (random) amount
	//      - Below is an example of how this might be achieved
    //
	//// perturb standard deviation
	//m_dStdDev = static_cast<double>(stdev(rng)) * sqrt(distance / 1000.0) / 100.0;
	//
	//// perturb measurement
	//double whichway = static_cast<double>(pertu(rng));
	//if (whichway > 0.5)
	//	m_dValue -= static_cast<double>(pertu(rng))/10.0;
	//else
	//	m_dValue += static_cast<double>(pertu(rng))/10.0;
}
	

//UINT32 CDnaHeightDifference::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
//{
//	char stationName[STN_NAME_WIDTH];
//	m_strType = measRecord->measType;
//	m_bIgnore = measRecord->ignore;
//	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
//	
//	// first station
//	m_lstn1Index = measRecord->station1;
//	ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
//	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
//	m_strFirst = stationName;
//	// target station
//	m_lstn2Index = measRecord->station2;
//	ifs_stns->seekg(sizeof(UINT32) + measRecord->station2 * sizeof(station_t), ios::beg);
//	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
//	m_strTarget = stationName;
//
//	m_measAdj = measRecord->measAdj;
//	m_measCorr = measRecord->measCorr;
//	m_measAdjPrec = measRecord->measAdjPrec;
//	m_residualPrec = measRecord->residualPrec;
//	m_preAdjCorr = measRecord->preAdjCorr;
//	m_dValue = measRecord->term1;
//	m_dStdDev = sqrt(measRecord->term2);
//	return 0;
//}


UINT32 CDnaHeightDifference::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_bIgnore = it_msr->ignore;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	
	m_strType = it_msr->measType;
	
	// first station
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;

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

	m_epoch = it_msr->epoch;

	return 0;
}
	

void CDnaHeightDifference::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
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
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	sprintf(measRecord.epoch, "%s", m_epoch.substr(0, STN_EPOCH_WIDTH).c_str());

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}


void CDnaHeightDifference::SetValue(const string& str)
{
	DoubleFromString(m_dValue, trimstr(str));
}

void CDnaHeightDifference::SetStdDev(const string& str)
{
	DoubleFromString(m_dStdDev, trimstr(str));
}

}	// namespace measurements
}	// namespace dynadjust
