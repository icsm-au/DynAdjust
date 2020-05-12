//============================================================================
// Name         : dnadirectionset.cpp
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
// Description  : CDnaDirectionSet implementation file
//============================================================================

#include <include/functions/dnatemplatefuncs.hpp>
#include <include/measurement_types/dnadirection.hpp>
#include <include/measurement_types/dnadirectionset.hpp>

namespace dynadjust {
namespace measurements {

CDnaDirectionSet::CDnaDirectionSet(void)
	: m_strTarget("")
	, m_drValue(0.)
	, m_dStdDev(0.)
	, m_lRecordedTotal(0)
	, m_lsetID(0)
{
	m_strType = "D";
	m_MSmeasurementStations = TWO_STATION;

	m_vTargetDirections.clear();
	m_vTargetDirections.reserve(15);
}

CDnaDirectionSet::~CDnaDirectionSet(void)
{
	m_vTargetDirections.empty();
}
	
CDnaDirectionSet::CDnaDirectionSet(const UINT32 lsetID)
	: m_strTarget("")
	, m_drValue(0.)
	, m_dStdDev(0.)
	, m_lRecordedTotal(0)
	, m_lsetID(lsetID)
{
	m_strType = "D";
	m_MSmeasurementStations = TWO_STATION;

	m_vTargetDirections.clear();
	m_vTargetDirections.reserve(15);
}
	
// copy constructors
CDnaDirectionSet::CDnaDirectionSet(const CDnaDirectionSet& newDirectionSet)
{
	m_bIgnore = newDirectionSet.m_bIgnore;
	m_strFirst = newDirectionSet.m_strFirst;
	m_strTarget = newDirectionSet.m_strTarget;
	m_drValue = newDirectionSet.m_drValue;
	m_dStdDev = newDirectionSet.m_dStdDev;
	m_lRecordedTotal = newDirectionSet.m_lRecordedTotal;
	m_MSmeasurementStations = newDirectionSet.m_MSmeasurementStations;

	m_lsetID = newDirectionSet.m_lsetID;

	m_strType = "D";
	m_vTargetDirections = newDirectionSet.m_vTargetDirections;

	m_databaseIdSet = newDirectionSet.m_databaseIdSet;
}


CDnaDirectionSet::CDnaDirectionSet(bool bIgnore, const string& strFirst, const string& strTarget,
	const double& drValue, const double& dStdDev, const float& fInstrHeight, const float& fTargetHeight)
{
	m_strType = "D";
	m_bIgnore = bIgnore;
	m_strFirst = strFirst;
	m_strTarget = strTarget;
	m_drValue = drValue;
	m_dStdDev = dStdDev;
	m_lsetID = 0;
	m_vTargetDirections.clear();
}


CDnaDirectionSet& CDnaDirectionSet::operator= (const CDnaDirectionSet& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_drValue = rhs.m_drValue;
	m_dStdDev = rhs.m_dStdDev;
	m_lRecordedTotal = rhs.m_lRecordedTotal;
	m_lsetID = rhs.m_lsetID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;
	m_vTargetDirections = rhs.m_vTargetDirections;
	
	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	return *this;
}


CDnaDirectionSet* CDnaDirectionSet::operator= (const CDnaDirectionSet* rhs)
{
	// check for assignment to self!
	if (this == rhs)
		return this;

	CDnaMeasurement::operator=(*rhs);
	m_strTarget = rhs->m_strTarget;
	m_drValue = rhs->m_drValue;
	m_dStdDev = rhs->m_dStdDev;
	m_lRecordedTotal = rhs->m_lRecordedTotal;
	m_lsetID = rhs->m_lsetID;
	m_MSmeasurementStations = rhs->m_MSmeasurementStations;
	m_vTargetDirections = rhs->m_vTargetDirections;
	
	m_databaseIdSet = rhs->m_databaseIdSet;

	return this;
}


bool CDnaDirectionSet::operator== (const CDnaDirectionSet& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strType == rhs.m_strType &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_drValue == rhs.m_drValue &&
		m_dStdDev == rhs.m_dStdDev &&
		m_lRecordedTotal == rhs.m_lRecordedTotal &&
		m_vTargetDirections == rhs.m_vTargetDirections
		);
}


bool CDnaDirectionSet::operator< (const CDnaDirectionSet& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// I don't think this is needed
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_strTarget == rhs.m_strTarget) {
					if (m_drValue == rhs.m_drValue) {
						if (m_dStdDev == rhs.m_dStdDev)
							return m_vTargetDirections < rhs.m_vTargetDirections;
						else
							return m_dStdDev < rhs.m_dStdDev; }
					else
						return m_drValue < rhs.m_drValue; }
				else
					return m_strTarget < rhs.m_strTarget; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}


void CDnaDirectionSet::AddDirection(const CDnaMeasurement* pDirection)
{
	CDnaDirection d = (CDnaDirection&)*pDirection;
	m_vTargetDirections.push_back(d);
}


void CDnaDirectionSet::ClearDirections()
{
	m_vTargetDirections.clear();
}


bool CDnaDirectionSet::IsRepeatedDirection(string strTarget)
{
	if (m_vTargetDirections.empty())
		return false;

	vector<CDnaDirection>::const_iterator _it_dirn;

	if (find_if(m_vTargetDirections.begin(), m_vTargetDirections.end(),
		bind2nd(operator_on_mem(&CDnaDirection::m_strTarget, std::equal_to<string>()), strTarget)) != m_vTargetDirections.end())
		return true;
	return false;
}


void CDnaDirectionSet::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	coutMeasurement(os);
	os << setw(INST_WIDTH) << m_strFirst;
	const size_t j(GetNumDirections());
	os << setw(TARG_WIDTH) << m_strTarget;
	os << setw(3) << (m_bIgnore ? "*" : " ") << setw(MEAS_WIDTH) << setprecision(9) << fixed << m_drValue;
	os << setw(VAR_WIDTH) << setprecision(3) << fixed << m_dStdDev;
	os << setw(7) << " " << setw(7) << " ";
	if (j > 0)
		os << "    " << j << (j<2 ? " observed direction" : " observed directions") << endl;
	for (size_t i(0); i<j; i++)
		m_vTargetDirections[i].coutDirectionData(os);
	if (j < 1)
		os << endl;
}
	
//void CDnaDirectionSet::SetDatabaseMap_bmsIndex(const UINT32& bmsIndex) 
//{ 
//	m_msr_db_map.bms_index = bmsIndex; 
//	UINT32 i(bmsIndex+1);
//	for_each(m_vTargetDirections.begin(), m_vTargetDirections.end(),
//		[this, &i](const CDnaDirection& dir) {
//			((CDnaMeasurement*)&dir)->SetDatabaseMap_bmsIndex(i++);
//	});
//}
	

void CDnaDirectionSet::SerialiseDatabaseMap(std::ofstream* os)
{
	CDnaMeasurement::SerialiseDatabaseMap(os);

	for_each(m_vTargetDirections.begin(), m_vTargetDirections.end(),
		[this, os](const CDnaDirection& dir) {
			((CDnaMeasurement*)&dir)->SerialiseDatabaseMap(os);
	});
}

// UINT32 CDnaDirectionSet::CalcDbidRecordCount() const
// {
// 	// Direction set has 1 RO direction and n directions
// 	UINT32 recordCount(1);
// 	for_each(m_vTargetDirections.begin(), m_vTargetDirections.end(),
// 		[&recordCount](const CDnaDirection& dir) {
// 			recordCount += dir.CalcDbidRecordCount();
// 	});
// 	return recordCount;
// }
	
UINT32 CDnaDirectionSet::CalcBinaryRecordCount() const
{
	// Direction set has 1 RO direction and n directions
	UINT32 recordCount(1);
	for_each(m_vTargetDirections.begin(), m_vTargetDirections.end(),
		[&recordCount](const CDnaDirection& dir) {
			recordCount += dir.CalcBinaryRecordCount();
	});
	return recordCount;
}
	

void CDnaDirectionSet::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
{
	const size_t dirCount(GetNumDirections());
	*dynaml_stream << "  <!--Type " << measurement_name<char, string>(GetTypeC());
	if (dirCount > 1)
		*dynaml_stream << " (set of " << dirCount << ")" << endl;
	else
		*dynaml_stream << "  (single)" << endl;
	*dynaml_stream << "-->" << endl;
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
	*dynaml_stream << "    <Value>" << setprecision(8) << fixed << RadtoDms(m_drValue) << "</Value>" << endl;
	*dynaml_stream << "    <StdDev>" << scientific << setprecision(6) << Seconds(m_dStdDev) << "</StdDev>" << endl;
	if (m_databaseIdSet)
	{
		*dynaml_stream << "    <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
		*dynaml_stream << "    <ClusterID>" << m_msr_db_map.cluster_id << "</ClusterID>" << endl;
	}
	*dynaml_stream << "    <Total>" << dirCount << "</Total>" << endl;
	
	// write directions
	vector<CDnaDirection>::const_iterator _it_dir = m_vTargetDirections.begin();
	for (_it_dir = m_vTargetDirections.begin(); _it_dir!=m_vTargetDirections.end(); _it_dir++)
		_it_dir->WriteDynaMLMsr(dynaml_stream, true);
	
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}
	

void CDnaDirectionSet::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	const size_t dirCount(GetNumDirections());
	
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";
	
	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << left << setw(dmw.msr_targ1) << m_strTarget;
	*dynaml_stream << left << setw(dmw.msr_targ2) << dirCount;
	*dynaml_stream << setw(dmw.msr_linear) << " ";	// linear measurement value
	*dynaml_stream << setw(dmw.msr_ang_d + dmw.msr_ang_m + dmw.msr_ang_s) << 
		right << FormatDnaDmsString(RadtoDms(m_drValue), 8);
	*dynaml_stream << setw(dmw.msr_stddev) << fixed << setprecision(3) << Seconds(m_dStdDev);
	
	if (m_databaseIdSet)
	{ 
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_inst_ht) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
		*dynaml_stream << setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
	}
	
	*dynaml_stream << endl;
	
	// write directions
	vector<CDnaDirection>::const_iterator _it_dir = m_vTargetDirections.begin();
	for (_it_dir = m_vTargetDirections.begin(); _it_dir!=m_vTargetDirections.end(); _it_dir++)
		_it_dir->WriteDNAMsr(dynaml_stream, dmw, dml, true);
}
	

void CDnaDirectionSet::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	_it_vdnastnptr stn1_it(vStations->begin() + m_lstn1Index);
	_it_vdnastnptr stn2_it(vStations->begin() + m_lstn2Index);
	
	// compute direction from instument to first target
	m_drValue = Direction(
		stn1_it->get()->GetXAxis(), 
		stn1_it->get()->GetYAxis(),
		stn1_it->get()->GetZAxis(),
		vStations->at(m_lstn2Index).get()->GetXAxis(), 
		vStations->at(m_lstn2Index).get()->GetYAxis(),
		vStations->at(m_lstn2Index).get()->GetZAxis(),
		stn1_it->get()->GetcurrentLatitude(),
		stn1_it->get()->GetcurrentLongitude());

	// Deflections available?
	if (fabs(stn1_it->get()->GetverticalDef()) > E4_SEC_DEFLECTION || fabs(stn1_it->get()->GetmeridianDef()) > E4_SEC_DEFLECTION)
	{
		// 1. compute zenith angle 
		double zenithDistance = ZenithDistance<double>(
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
			0.0,				
			0.0);	

		// 2. Compute deflection
		m_preAdjCorr = DirectionDeflectionCorrection<double>(
			m_drValue,
			zenithDistance,
			stn1_it->get()->GetverticalDef(),			// deflection in prime vertical
			stn1_it->get()->GetmeridianDef());			// deflection in prime meridian
			
		// 3. apply deflection correction
		m_drValue += m_preAdjCorr;
	}

	// compute standard deviation in radians
	m_dStdDev = SecondstoRadians(0.01);
	
	vector<CDnaDirection>::iterator _it_dir = m_vTargetDirections.begin();
	for (_it_dir = m_vTargetDirections.begin(); _it_dir!=m_vTargetDirections.end(); _it_dir++)
		_it_dir->SimulateMsr(vStations, ellipsoid);
}
	

UINT32 CDnaDirectionSet::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
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
	m_drValue = measRecord->term1;
	m_dStdDev = sqrt(measRecord->term2);
	
	// measRecord holds the full number of measurement blocks, which is 
	// the number of directions in the vector plus one for the RO
	m_lRecordedTotal = measRecord->vectorCount1 - 1;

	m_lsetID = measRecord->clusterID;

	m_vTargetDirections.clear();
	m_vTargetDirections.resize(m_lRecordedTotal);

	UINT32 measrecordCount = 0;	

	// now covariances
	vector<CDnaDirection>::iterator _it_dir = m_vTargetDirections.begin();
	for (; _it_dir!=m_vTargetDirections.end(); _it_dir++)
	{
		_it_dir->SetType(m_strType);
		_it_dir->SetFirst(m_strFirst);
		measrecordCount += _it_dir->SetMeasurementRec(ifs_stns, ifs_msrs, measRecord);
	}

	return measrecordCount;
}
	

UINT32 CDnaDirectionSet::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
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
	
	m_measAdj = it_msr->measAdj;
	m_measCorr = it_msr->measCorr;
	m_measAdjPrec = it_msr->measAdjPrec;
	m_residualPrec = it_msr->residualPrec;
	m_preAdjCorr = it_msr->preAdjCorr;
	m_drValue = it_msr->term1;
	m_dStdDev = sqrt(it_msr->term2);
	
	// it_msr holds the full number of measurement blocks, which is 
	// the number of directions in the vector plus one for the RO
	m_lRecordedTotal = it_msr->vectorCount1 - 1;

	m_lsetID = it_msr->clusterID;

	m_vTargetDirections.clear();
	m_vTargetDirections.resize(m_lRecordedTotal);

	UINT32 it_msrCount = m_lRecordedTotal;	

	// now covariances
	vector<CDnaDirection>::iterator _it_dir = m_vTargetDirections.begin();
	for (; _it_dir!=m_vTargetDirections.end(); _it_dir++)
	{
		it_msr++;
		_it_dir->SetType(m_strType);
		_it_dir->SetFirst(m_strFirst);
		_it_dir->SetMeasurementRec(binaryStn, it_msr);
	}

	return it_msrCount;
}
	

void CDnaDirectionSet::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.station2 = m_lstn2Index;
	
	// number of Directions in the parent cluster including the first
	measRecord.vectorCount1 = static_cast<UINT32>(m_vTargetDirections.size()) + 1;	

	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_drValue;
	measRecord.term2 = m_dStdDev * m_dStdDev;	// convert to variance
	
	measRecord.clusterID = m_lsetID;
	measRecord.measurementStations = m_MSmeasurementStations;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	vector<CDnaDirection>::const_iterator _it_dir;
	for (_it_dir = m_vTargetDirections.begin(); _it_dir!=m_vTargetDirections.end(); _it_dir++)
		_it_dir->WriteBinaryMsr(binary_stream, msrIndex);
}

void CDnaDirectionSet::SetValue(const string& str)
{
	RadFromDmsString(&m_drValue, trimstr(str));
}

void CDnaDirectionSet::SetStdDev(const string& str)
{
	RadFromSecondsString(&m_dStdDev, trimstr(str));
}

void CDnaDirectionSet::SetTotal(const string& str)
{
	m_lRecordedTotal = LongFromString<UINT32>(trimstr(str));
	m_vTargetDirections.reserve(m_lRecordedTotal);
}




}	// namespace measurements
}	// namespace dynadjust
