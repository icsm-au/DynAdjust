//============================================================================
// Name         : dnameasurement.cpp
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
// Description  : CDnaMeasurement and CDnaCovariance implementation file
//============================================================================

#include <include/exception/dnaexception.hpp>
#include <include/measurement_types/dnameasurement.hpp>

using namespace dynadjust::exception;

namespace dynadjust {
namespace measurements {

CDnaCovariance::CDnaCovariance(void)
	: m_bIgnore(false)
	, m_lstn1Index(0)
	, m_lstn2Index(0)
	, m_strType("")
	, m_dM11(0.)
	, m_dM12(0.)
	, m_dM13(0.)
	, m_dM21(0.)
	, m_dM22(0.)
	, m_dM23(0.)
	, m_dM31(0.)
	, m_dM32(0.)
	, m_dM33(0.)
	, m_lclusterID(0)
{
}


CDnaCovariance::~CDnaCovariance(void)
{

}


CDnaCovariance::CDnaCovariance(const CDnaCovariance& newCovariance)
{
	m_bIgnore = newCovariance.m_bIgnore;
	m_lstn1Index = newCovariance.m_lstn1Index;
	m_lstn2Index = newCovariance.m_lstn2Index;
	m_strType = newCovariance.m_strType;
	m_dM11 = newCovariance.m_dM11;
	m_dM12 = newCovariance.m_dM12;
	m_dM13 = newCovariance.m_dM13;
	m_dM21 = newCovariance.m_dM21;
	m_dM22 = newCovariance.m_dM22;
	m_dM23 = newCovariance.m_dM23;
	m_dM31 = newCovariance.m_dM31;
	m_dM32 = newCovariance.m_dM32;
	m_dM33 = newCovariance.m_dM33;
	m_lclusterID = newCovariance.m_lclusterID;
}


CDnaCovariance& CDnaCovariance::operator= (const CDnaCovariance& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	m_bIgnore = rhs.m_bIgnore;
	m_lstn1Index = rhs.m_lstn1Index;
	m_lstn2Index = rhs.m_lstn2Index;
	m_strType = rhs.m_strType;
	m_dM11 = rhs.m_dM11;
	m_dM12 = rhs.m_dM12;
	m_dM13 = rhs.m_dM13;
	m_dM21 = rhs.m_dM21;
	m_dM22 = rhs.m_dM22;
	m_dM23 = rhs.m_dM23;
	m_dM31 = rhs.m_dM31;
	m_dM32 = rhs.m_dM32;
	m_dM33 = rhs.m_dM33;
	m_lclusterID = rhs.m_lclusterID;

	return *this;
}


bool CDnaCovariance::operator== (const CDnaCovariance& rhs) const
{
	return (
		m_bIgnore == rhs.m_bIgnore &&
		m_lstn1Index == rhs.m_lstn1Index &&
		m_lstn2Index == rhs.m_lstn2Index &&
		m_strType == rhs.m_strType &&
		m_dM11 == rhs.m_dM11 &&
		m_dM12 == rhs.m_dM12 &&
		m_dM13 == rhs.m_dM13 &&
		m_dM21 == rhs.m_dM21 &&
		m_dM22 == rhs.m_dM22 &&
		m_dM23 == rhs.m_dM23 &&
		m_dM31 == rhs.m_dM31 &&
		m_dM32 == rhs.m_dM32 &&
		m_dM33 == rhs.m_dM33
	);
}

void CDnaCovariance::coutCovarianceData(ostream &os) const
{

}
	

void CDnaCovariance::WriteDynaMLMsr(std::ofstream* dynaml_stream) const
{
	if (GetTypeC() == 'X')
		*dynaml_stream << "      <GPSCovariance>" << endl;
	else
		*dynaml_stream << "      <PointCovariance>" << endl;
		
	*dynaml_stream << "        <m11>" << scientific << setprecision(13) << m_dM11 << "</m11>" << endl;
	*dynaml_stream << "        <m12>" << m_dM12 << "</m12>" << endl;
	*dynaml_stream << "        <m13>" << m_dM13 << "</m13>" << endl;
	*dynaml_stream << "        <m21>" << m_dM21 << "</m21>" << endl;
	*dynaml_stream << "        <m22>" << m_dM22 << "</m22>" << endl;
	*dynaml_stream << "        <m23>" << m_dM23 << "</m23>" << endl;
	*dynaml_stream << "        <m31>" << m_dM31 << "</m31>" << endl;
	*dynaml_stream << "        <m32>" << m_dM32 << "</m32>" << endl;
	*dynaml_stream << "        <m33>" << m_dM33 << "</m33>" << endl;
	
	if (GetTypeC() == 'X')
		*dynaml_stream << "      </GPSCovariance>";
	else
		*dynaml_stream << "      </PointCovariance>";
}
	

void CDnaCovariance::WriteDNAMsr(std::ofstream* dynaml_stream, 
	const dna_msr_fields& dmw, const dna_msr_fields& dml, 
	const msr_database_id_map& dbidmap, bool dbidSet) const
{
	UINT32 pad(dmw.msr_type + dmw.msr_ignore + dmw.msr_inst + dmw.msr_targ1 + dmw.msr_targ2 + dmw.msr_gps);
	// X
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream <<
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dM11 <<
		right << setw(dmw.msr_gps_vcv_2) << m_dM12 <<
		right << setw(dmw.msr_gps_vcv_3) << m_dM13;

	if (dbidSet)
	{
		*dynaml_stream << setw(dmw.msr_id_msr) << dbidmap.msr_id;
		*dynaml_stream << setw(dmw.msr_id_cluster) << dbidmap.cluster_id;
	}

	*dynaml_stream << endl;
		
	// Y
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dM21 <<
		right << setw(dmw.msr_gps_vcv_2) << m_dM22 <<
		right << setw(dmw.msr_gps_vcv_3) << m_dM23;

	if (dbidSet)
	{
		*dynaml_stream << setw(dmw.msr_id_msr) << dbidmap.msr_id;
		*dynaml_stream << setw(dmw.msr_id_cluster) << dbidmap.cluster_id;
	}

	*dynaml_stream << endl;

	// Z
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dM31 <<
		right << setw(dmw.msr_gps_vcv_2) << m_dM32 <<
		right << setw(dmw.msr_gps_vcv_3) << m_dM33;

	if (dbidSet)
	{
		*dynaml_stream << setw(dmw.msr_id_msr) << dbidmap.msr_id;
		*dynaml_stream << setw(dmw.msr_id_cluster) << dbidmap.cluster_id;
	}

	*dynaml_stream << endl;
}
	

void CDnaCovariance::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	m_dM11 = 0.0;
	m_dM12 = 0.0;
	m_dM13 = 0.0;
	m_dM21 = 0.0;
	m_dM22 = 0.0;
	m_dM23 = 0.0;
	m_dM31 = 0.0;
	m_dM32 = 0.0;
	m_dM33 = 0.0;
}
	

UINT32 CDnaCovariance::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	UINT32 measrecordCount = 0;

	m_lstn1Index = measRecord->station1;
	m_lstn2Index = measRecord->station2;
	m_strType = measRecord->measType;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	if (ifs_msrs->eof() || !ifs_msrs->good())
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
	
	// check integrity of the binary file (see WriteBinaryMsr())
	if (measRecord->measType != 'X' && measRecord->measType != 'Y')
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint Covariance element.", 0);
		
	m_dM11 = measRecord->term1;
	m_dM12 = measRecord->term2;
	m_dM13 = measRecord->term3;

	m_lclusterID = measRecord->clusterID;

	measrecordCount++;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	if (ifs_msrs->eof() || !ifs_msrs->good())
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
	
	// check integrity of the binary file (see WriteBinaryMsr())
	if (measRecord->measType != 'X' && measRecord->measType != 'Y')
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint Covariance element.", 0);
	
	m_lstn1Index = measRecord->station1;
	m_lstn2Index = measRecord->station2;
	m_dM21 = measRecord->term1;
	m_dM22 = measRecord->term2;
	m_dM23 = measRecord->term3;
	
	measrecordCount++;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	if (ifs_msrs->eof() || !ifs_msrs->good())
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
	
	m_lstn1Index = measRecord->station1;
	m_lstn2Index = measRecord->station2;
	m_dM31 = measRecord->term1;
	m_dM32 = measRecord->term2;
	m_dM33 = measRecord->term3;
	
	return ++measrecordCount;
}
	

UINT32 CDnaCovariance::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	it_msr++;
	m_lstn1Index = it_msr->station1;
	m_lstn2Index = it_msr->station2;
	m_strType = it_msr->measType;
	m_dM11 = it_msr->term1;
	m_dM12 = it_msr->term2;
	m_dM13 = it_msr->term3;

	m_lclusterID = it_msr->clusterID;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	it_msr++;
	
	m_lstn1Index = it_msr->station1;
	m_lstn2Index = it_msr->station2;
	m_dM21 = it_msr->term1;
	m_dM22 = it_msr->term2;
	m_dM23 = it_msr->term3;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	it_msr++;
	
	m_lstn1Index = it_msr->station1;
	m_lstn2Index = it_msr->station2;
	m_dM31 = it_msr->term1;
	m_dM32 = it_msr->term2;
	m_dM33 = it_msr->term3;
	
	return 0;
}
	

void CDnaCovariance::WriteBinaryMsr(std::ofstream *binary_stream, PUINT32 msrIndex, const string& epsgCode, const string& epoch) const
{
	*msrIndex += 3;
	measurement_t measRecord;

	// Common
	measRecord.measType = GetTypeC();
	measRecord.station1 = m_lstn1Index;	
	measRecord.station2 = m_lstn2Index;	
	measRecord.clusterID = m_lclusterID;
	
	sprintf(measRecord.epsgCode, "%s", epsgCode.substr(0, STN_EPSG_WIDTH).c_str());
	sprintf(measRecord.epoch, "%s", epoch.substr(0, STN_EPSG_WIDTH).c_str());

	// X
	measRecord.measStart = xCov;
	measRecord.term1 = m_dM11;
	measRecord.term2 = m_dM12;
	measRecord.term3 = m_dM13;

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// Y
	measRecord.measStart = yCov;
	measRecord.term1 = m_dM21;
	measRecord.term2 = m_dM22;
	measRecord.term3 = m_dM23;

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// Z
	measRecord.measStart = zCov;
	measRecord.term1 = m_dM31;
	measRecord.term2 = m_dM32;
	measRecord.term3 = m_dM33;
	
	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));
}

void CDnaCovariance::SerialiseDatabaseMap(std::ofstream* os, const UINT32& msr_id, const UINT32& cluster_id)
{
	// X
	os->write(reinterpret_cast<const char *>(&msr_id), sizeof(UINT32));
	os->write(reinterpret_cast<const char *>(&cluster_id), sizeof(UINT32));
	// Y
	os->write(reinterpret_cast<const char *>(&msr_id), sizeof(UINT32));
	os->write(reinterpret_cast<const char *>(&cluster_id), sizeof(UINT32));
	// Z
	os->write(reinterpret_cast<const char *>(&msr_id), sizeof(UINT32));
	os->write(reinterpret_cast<const char *>(&cluster_id), sizeof(UINT32));
}

void CDnaCovariance::SetM11(const string& str)
{
	DoubleFromString(m_dM11, trimstr(str));
}

void CDnaCovariance::SetM12(const string& str)
{
	DoubleFromString(m_dM12, trimstr(str));
}

void CDnaCovariance::SetM13(const string& str)
{
	DoubleFromString(m_dM13, trimstr(str));
}

void CDnaCovariance::SetM21(const string& str)
{
	DoubleFromString(m_dM21, trimstr(str));
}

void CDnaCovariance::SetM22(const string& str)
{
	DoubleFromString(m_dM22, trimstr(str));
}

void CDnaCovariance::SetM23(const string& str)
{
	DoubleFromString(m_dM23, trimstr(str));
}

void CDnaCovariance::SetM31(const string& str)
{
	DoubleFromString(m_dM31, trimstr(str));
}

void CDnaCovariance::SetM32(const string& str)
{
	DoubleFromString(m_dM32, trimstr(str));
}

void CDnaCovariance::SetM33(const string& str)
{
	DoubleFromString(m_dM33, trimstr(str));
}



// Comparison functions
// m_strFirst
bool operator<(const CDnaMeasurement& left, const CDnaMeasurement& right)
{
	return left.m_strFirst < right.m_strFirst;
}

bool operator<(const dnaMsrPtr& left, const dnaMsrPtr& right)
{
	return left.get()->m_strFirst < right.get()->m_strFirst;
}



CDnaMeasurement::CDnaMeasurement()
	: m_strFirst("")
	, m_MSmeasurementStations(ONE_STATION)
	, m_bIgnore(false)
	, m_lmeasurementIndex(0)
	, m_lstn1Index(0)
	, m_lstn2Index(0)
	, m_lstn3Index(0)
	, m_measAdj(0)
	, m_measCorr(0)
	, m_measAdjPrec(0)
	, m_residualPrec(0)
	, m_preAdjCorr(0)
	, m_epsgCode(DEFAULT_EPSG_S)
	, m_databaseIdSet(false)
{
}
	

CDnaMeasurement::~CDnaMeasurement()
{
}

// copy constructors
CDnaMeasurement::CDnaMeasurement(const CDnaMeasurement& newMeasurement)
{
	m_strType = newMeasurement.m_strType;
	m_bIgnore = newMeasurement.m_bIgnore;
	m_strFirst = newMeasurement.m_strFirst;
	m_MSmeasurementStations = newMeasurement.m_MSmeasurementStations;
	m_lmeasurementIndex = newMeasurement.m_lmeasurementIndex;
	m_lstn1Index = newMeasurement.m_lstn1Index;
	m_lstn2Index = newMeasurement.m_lstn2Index;
	m_lstn3Index = newMeasurement.m_lstn3Index;

	m_measAdj = newMeasurement.m_measAdj;
	m_measCorr = newMeasurement.m_measCorr;
	m_measAdjPrec = newMeasurement.m_measAdjPrec;
	m_residualPrec = newMeasurement.m_residualPrec;
	m_preAdjCorr = newMeasurement.m_preAdjCorr;

	m_epsgCode = newMeasurement.m_epsgCode;

	m_msr_db_map = newMeasurement.m_msr_db_map;
	m_databaseIdSet = newMeasurement.m_databaseIdSet;
}



CDnaMeasurement& CDnaMeasurement::operator= (const CDnaMeasurement& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	m_strFirst = rhs.m_strFirst;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;
	m_strType = rhs.m_strType;
	m_bIgnore = rhs.m_bIgnore;
	m_lmeasurementIndex = rhs.m_lmeasurementIndex;
	m_lstn1Index = rhs.m_lstn1Index;
	m_lstn2Index = rhs.m_lstn2Index;
	m_lstn3Index = rhs.m_lstn3Index;

	m_measAdj = rhs.m_measAdj;
	m_measCorr = rhs.m_measCorr;
	m_measAdjPrec = rhs.m_measAdjPrec;
	m_residualPrec = rhs.m_residualPrec;
	m_preAdjCorr = rhs.m_preAdjCorr;

	m_epsgCode = rhs.m_epsgCode;

	m_msr_db_map = rhs.m_msr_db_map;
	m_databaseIdSet = rhs.m_databaseIdSet;

	return *this;
}

// virtual functions
// void CDnaMeasurement::coutMeasurementData() { }

void CDnaMeasurement::coutMeasurement(ostream& os) const
{
	os << setw(2) << left << "+ " << setw(2) << m_strType;
}

void CDnaMeasurement::SetMeasurementDBID(const string& str)
{
	m_msr_db_map.msr_id = LongFromString<UINT32>(str);
	m_databaseIdSet = true;
}

void CDnaMeasurement::SetClusterDBID(const string& str)
{
	m_msr_db_map.cluster_id = LongFromString<UINT32>(str);
}

void CDnaMeasurement::SetType(const string& str)
{ 
	m_strType = trimstr(str);
	str_toupper<int>(m_strType);
}

void CDnaMeasurement::SetDatabaseMap(const msr_database_id_map& dbidmap, bool dbidSet) 
{
	m_databaseIdSet = dbidSet;
	if (dbidSet)
		m_msr_db_map = dbidmap;
}

void CDnaMeasurement::SerialiseDatabaseMap(std::ofstream* os)
{
	os->write(reinterpret_cast<const char *>(&m_msr_db_map.msr_id), sizeof(UINT32));
	os->write(reinterpret_cast<const char *>(&m_msr_db_map.cluster_id), sizeof(UINT32));
}

}	// namespace measurements
}	// namespace dynadjust


