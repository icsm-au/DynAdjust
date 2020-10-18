//============================================================================
// Name         : dnagpsbaseline.cpp
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
// Description  : CDnaGpsBaseline and CDnaGpsBaselineCluster implementation file
//============================================================================

#include <include/exception/dnaexception.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/measurement_types/dnagpsbaseline.hpp>

using namespace dynadjust::exception;
using namespace dynadjust::epsg;

namespace dynadjust {
namespace measurements {

CDnaGpsBaseline::CDnaGpsBaseline(void)
	: m_strTarget("")
	, m_lRecordedTotal(0)
	, m_dX(0.)
	, m_dY(0.)
	, m_dZ(0.)
	, m_dSigmaXX(0.)
	, m_dSigmaXY(0.)
	, m_dSigmaXZ(0.)
	, m_dSigmaYY(0.)
	, m_dSigmaYZ(0.)
	, m_dSigmaZZ(0.)
	, m_dPscale(1.)
	, m_dLscale(1.)
	, m_dHscale(1.)
	, m_dVscale(1.)
	, m_referenceFrame(DEFAULT_DATUM)
	, m_epoch(DEFAULT_EPOCH)
	, m_lclusterID(0)
{
	SetEpsg(epsgStringFromName<string>(m_referenceFrame));

	m_vGpsCovariances.clear();
	m_vGpsCovariances.reserve(0);
	m_MSmeasurementStations = TWO_STATION;
}


CDnaGpsBaseline::~CDnaGpsBaseline(void)
{

}


CDnaGpsBaseline::CDnaGpsBaseline(const CDnaGpsBaseline& newGpsBaseline)
{
	m_strType = newGpsBaseline.m_strType;
	m_strFirst = newGpsBaseline.m_strFirst;
	m_strTarget = newGpsBaseline.m_strTarget;
	m_bIgnore = newGpsBaseline.m_bIgnore;
	m_lRecordedTotal = newGpsBaseline.m_lRecordedTotal;

	m_referenceFrame = newGpsBaseline.m_referenceFrame;
	m_epsgCode = newGpsBaseline.m_epsgCode;
	m_epoch = newGpsBaseline.m_epoch;

	m_dX = newGpsBaseline.m_dX;
	m_dY = newGpsBaseline.m_dY;
	m_dZ = newGpsBaseline.m_dZ;
	m_dSigmaXX = newGpsBaseline.m_dSigmaXX;
	m_dSigmaXY = newGpsBaseline.m_dSigmaXY;
	m_dSigmaXZ = newGpsBaseline.m_dSigmaXZ;
	m_dSigmaYY = newGpsBaseline.m_dSigmaYY;
	m_dSigmaYZ = newGpsBaseline.m_dSigmaYZ;
	m_dSigmaZZ = newGpsBaseline.m_dSigmaZZ;

	m_dPscale = newGpsBaseline.m_dPscale;
	m_dLscale = newGpsBaseline.m_dLscale;
	m_dHscale = newGpsBaseline.m_dHscale;
	m_dVscale = newGpsBaseline.m_dVscale;

	m_lclusterID = newGpsBaseline.m_lclusterID;
	m_MSmeasurementStations = newGpsBaseline.m_MSmeasurementStations;

	m_vGpsCovariances = newGpsBaseline.m_vGpsCovariances;

	m_databaseIdSet = newGpsBaseline.m_databaseIdSet;
	m_msr_db_map = newGpsBaseline.m_msr_db_map;
}


CDnaGpsBaseline::CDnaGpsBaseline(const bool bIgnore, const string& strType, const string& strFirstStation, const string& strSecondStation)
{
	m_strFirst = strFirstStation;
	m_strTarget = strSecondStation;
	m_bIgnore = bIgnore;

	m_referenceFrame = DEFAULT_DATUM;
	m_epoch = DEFAULT_EPOCH;
	SetEpsg(epsgStringFromName<string>(m_referenceFrame));
}


CDnaGpsBaseline& CDnaGpsBaseline::operator= (const CDnaGpsBaseline& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_lRecordedTotal = rhs.m_lRecordedTotal;

	m_referenceFrame = rhs.m_referenceFrame;
	m_epsgCode = rhs.m_epsgCode;
	m_epoch = rhs.m_epoch;

	m_dX = rhs.m_dX;
	m_dY = rhs.m_dY;
	m_dZ = rhs.m_dZ;
	m_dSigmaXX = rhs.m_dSigmaXX;
	m_dSigmaXY = rhs.m_dSigmaXY;
	m_dSigmaXZ = rhs.m_dSigmaXZ;
	m_dSigmaYY = rhs.m_dSigmaYY;
	m_dSigmaYZ = rhs.m_dSigmaYZ;
	m_dSigmaZZ = rhs.m_dSigmaZZ;

	m_dPscale = rhs.m_dPscale;
	m_dLscale = rhs.m_dLscale;
	m_dHscale = rhs.m_dHscale;
	m_dVscale = rhs.m_dVscale;

	m_lclusterID = rhs.m_lclusterID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	m_vGpsCovariances = rhs.m_vGpsCovariances;

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	return *this;
}


bool CDnaGpsBaseline::operator== (const CDnaGpsBaseline& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strTarget == rhs.m_strTarget &&
		m_strType == rhs.m_strType &&
		m_lRecordedTotal == rhs.m_lRecordedTotal &&
		m_bIgnore == rhs.m_bIgnore &&
		fabs(m_dX - rhs.m_dX) < PRECISION_1E4 &&
		fabs(m_dY - rhs.m_dY) < PRECISION_1E4 &&
		fabs(m_dZ - rhs.m_dZ) < PRECISION_1E4 &&
		iequals(m_referenceFrame, rhs.m_referenceFrame)
		);
}

bool CDnaGpsBaseline::operator< (const CDnaGpsBaseline& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strTarget == rhs.m_strTarget) {
			if (m_strType == rhs.m_strType) {
				if (m_lRecordedTotal == rhs.m_lRecordedTotal) {
					if (m_bIgnore == rhs.m_bIgnore) {
						if (fabs(m_dX - rhs.m_dX) < PRECISION_1E4) {
							if (fabs(m_dY - rhs.m_dY) < PRECISION_1E4) {
								return m_dZ < rhs.m_dZ; }
							else
								return m_dY < rhs.m_dY; }
						else
							return m_dX < rhs.m_dX; }
					else
						return m_bIgnore < rhs.m_bIgnore; }
				else
					return m_lRecordedTotal < rhs.m_lRecordedTotal;	}
			else
				return m_strType < rhs.m_strType; }
		else
			return m_strTarget < rhs.m_strTarget; }
	else
		return m_strFirst < rhs.m_strFirst;
}

void CDnaGpsBaseline::ReserveGpsCovariancesCount(const UINT32& size)
{
	m_vGpsCovariances.reserve(size);
}


void CDnaGpsBaseline::ResizeGpsCovariancesCount(const UINT32& size)
{
	m_vGpsCovariances.resize(size);
}


void CDnaGpsBaseline::AddGpsCovariance(const CDnaCovariance* pGpsCovariance)
{
	CDnaCovariance vcv = (CDnaCovariance&)*pGpsCovariance;
	m_vGpsCovariances.push_back(vcv);
}





void CDnaGpsBaseline::coutMeasurementData(ostream &os, const UINT16& uType) const
{
	switch (uType)
	{
	case DNA_COUT:
	case GEOLAB_COUT:
	case NEWGAN_COUT:
		coutMeasurement(os);
		break;
	}
}
	

UINT32 CDnaGpsBaseline::CalcBinaryRecordCount() const
{
	UINT32 RecordCount = 3;
	vector<CDnaCovariance>::const_iterator _it_cov = m_vGpsCovariances.begin();
	for (; _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		RecordCount += _it_cov->CalcBinaryRecordCount();
	return RecordCount;
}


void CDnaGpsBaseline::coutBaselineData(ostream &os, const int& pad, const UINT16& uType) const
{
	size_t i, j;
	string ignoreFlag;

	switch (uType)
	{
	case DNA_COUT:
	case GEOLAB_COUT:
		// normal format
		os << setw(pad) << left << m_strFirst;
		os << setw(TARG_WIDTH) << m_strTarget;
		ignoreFlag = " ";

		if (m_bIgnore)
			ignoreFlag = "*";
		j = m_vGpsCovariances.size();
		os << setw(2) << ignoreFlag << setw(8) << setprecision(3) << fixed << m_dPscale << setw(2) << "X" << setw(MSR2_WIDTH) << right << m_dX << setw(VAR_WIDTH) << scientific << m_dSigmaXX << setw(VAR_WIDTH) << m_dSigmaXY << setw(VAR_WIDTH) << m_dSigmaXZ;
		for (i=0; i<j; i++)
			os << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM11() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM12() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM13();
		os << endl;
		os << setw(4) << " " << 
			setw(pad) << " " << setw(TARG_WIDTH) << " " <<
			setw(2) << left << ignoreFlag << setw(8) << setprecision(3) << fixed << m_dLscale << setw(2) << "Y" << setw(MSR2_WIDTH) << right << m_dY << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << scientific << m_dSigmaYY << setw(VAR_WIDTH) << m_dSigmaYZ;
		for (i=0; i<j; i++)
			os << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM21() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM22() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM23();
		os << endl;
		os << setw(4) << " " <<
			setw(pad) << " " << setw(TARG_WIDTH) << " " <<
			setw(2) << left << ignoreFlag << setw(8) << setprecision(3) << fixed << m_dHscale << setw(2) << "Z" << setw(MSR2_WIDTH) << right << m_dZ << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << scientific << m_dSigmaZZ;
		for (i=0; i<j; i++)
			os << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM31() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM32() << setw(COV_WIDTH) << m_vGpsCovariances[i].GetM33();
		os << endl;
		break;
	case NEWGAN_COUT:

		// NEWGAN format (ignore correlations)
		const float M2TOCM2 = 10000.;
		os << endl << "115" << endl;
		os << setw(3) << "116" << setw(3) << " " << setw(9) << m_strFirst << setw(3) << " " << setw(9) << m_strTarget;
		ignoreFlag = " ";

		if (m_bIgnore)
			ignoreFlag = "*";
		j = m_vGpsCovariances.size();
		os << setw(23) << setprecision(4) << fixed << m_dX << setw(13) << m_dY << setw(13) << m_dZ << endl;
		os << "117" << setw(8) << "UPPER" << endl;
		os << setw(26) << setprecision(8) << scientific << m_dSigmaXX * M2TOCM2 << setw(20) << m_dSigmaXY * M2TOCM2 << setw(20) << m_dSigmaXZ * M2TOCM2 << endl;
		os << setw(26) << setprecision(8) << scientific << m_dSigmaYY * M2TOCM2 << setw(20) << m_dSigmaYZ * M2TOCM2 << endl;
		os << setw(26) << setprecision(8) << scientific << m_dSigmaZZ * M2TOCM2 << endl;
		break;
	}
}


void CDnaGpsBaseline::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << "    <First>" << m_strFirst << "</First>" << endl;
	*dynaml_stream << "    <Second>" << m_strTarget << "</Second>" << endl;

	UINT32 precision = 4;
	
	*dynaml_stream << "    <GPSBaseline>" << endl;
	*dynaml_stream << "      <X>" << fixed << setprecision(precision) << m_dX << "</X>" << endl;
	*dynaml_stream << "      <Y>" << m_dY << "</Y>" << endl;
	*dynaml_stream << "      <Z>" << m_dZ << "</Z>" << endl;

	if (m_databaseIdSet)
		*dynaml_stream << "      <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;

	*dynaml_stream << "      <SigmaXX>" << scientific << setprecision(13) << m_dSigmaXX << "</SigmaXX>" << endl;
	*dynaml_stream << "      <SigmaXY>" << m_dSigmaXY << "</SigmaXY>" << endl;
	*dynaml_stream << "      <SigmaXZ>" << m_dSigmaXZ << "</SigmaXZ>" << endl;
	*dynaml_stream << "      <SigmaYY>" << m_dSigmaYY << "</SigmaYY>" << endl;
	*dynaml_stream << "      <SigmaYZ>" << m_dSigmaYZ << "</SigmaYZ>" << endl;
	*dynaml_stream << "      <SigmaZZ>" << m_dSigmaZZ << "</SigmaZZ>" << endl;

	// write GPSPoint covariances
	vector<CDnaCovariance>::const_iterator _it_cov = m_vGpsCovariances.begin();
	for (; _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		_it_cov->WriteDynaMLMsr(dynaml_stream);

	*dynaml_stream << "    </GPSBaseline>" << endl;

}


void CDnaGpsBaseline::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	*dynaml_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dynaml_stream << setw(dmw.msr_ignore) << "*";
	else
		*dynaml_stream << setw(dmw.msr_ignore) << " ";

	*dynaml_stream << left << setw(dmw.msr_inst) << m_strFirst;
	*dynaml_stream << left << setw(dmw.msr_targ1) << m_strTarget;

	// Print header for G baseline and first X cluster baseline
	bool printHeader(true);
	
	if (GetTypeC() == 'X')
	{
		if (m_lRecordedTotal != m_vGpsCovariances.size() + 1)
		{
			printHeader = false;
			// print database ids
			if (m_databaseIdSet)
			{
				*dynaml_stream << setw(dml.msr_id_msr - dml.msr_targ2) << " ";
				*dynaml_stream << right << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id <<
					setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
			}
		}
	}

	if (printHeader)
	{
		if (GetTypeC() == 'X')
			*dynaml_stream << left << setw(dmw.msr_targ2) << m_lRecordedTotal;
		else
			*dynaml_stream << right << setw(dmw.msr_targ2) << " ";
		
		// print scaling
		*dynaml_stream << 
			fixed << setprecision(2) << 
			right << setw(dmw.msr_gps_vscale) << double_string_width<double, UINT32, string>(m_dVscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_pscale) << double_string_width<double, UINT32, string>(m_dPscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_lscale) << double_string_width<double, UINT32, string>(m_dLscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_hscale) << double_string_width<double, UINT32, string>(m_dHscale, dmw.msr_gps_vscale);

		// print reference frame and epoch
		*dynaml_stream <<
			right << setw(dmw.msr_gps_reframe) << m_referenceFrame <<
			right << setw(dmw.msr_gps_epoch) << m_epoch;

		// print database ids
		if (m_databaseIdSet)
		{
			*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id <<
				setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
		}
	}

	*dynaml_stream << endl;

	UINT32 pad(dmw.msr_type + dmw.msr_ignore + dmw.msr_inst + dmw.msr_targ1 + dmw.msr_targ2);

	// X
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream << right << setw(dmw.msr_gps) << fixed << setprecision(4) << m_dX;
	*dynaml_stream << right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXX;

	// print database ids
	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_gps_vcv_2) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id <<
			setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
	}

	*dynaml_stream << endl;
		
	// Y
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream << right << setw(dmw.msr_gps) << fixed << setprecision(4) << m_dY;
	*dynaml_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXY << 
		right << setw(dmw.msr_gps_vcv_2) << m_dSigmaYY;

	// print database ids
	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(dml.msr_id_msr - dml.msr_gps_vcv_3) << " ";
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id <<
			setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
	}

	*dynaml_stream << endl;

	// Z
	*dynaml_stream << setw(pad) << " ";
	*dynaml_stream << right << setw(dmw.msr_gps) << fixed << setprecision(4) << m_dZ;
	*dynaml_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXZ <<
		right << setw(dmw.msr_gps_vcv_2) << m_dSigmaYZ << 
		right << setw(dmw.msr_gps_vcv_3) << m_dSigmaZZ;

	// print database ids
	if (m_databaseIdSet)
	{
		*dynaml_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id <<
			setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
	}

	*dynaml_stream << endl;

	// write GPSBaseline covariances (not supported by DNA format)
	vector<CDnaCovariance>::const_iterator _it_cov = m_vGpsCovariances.begin();
	for (_it_cov=m_vGpsCovariances.begin(); _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		_it_cov->WriteDNAMsr(dynaml_stream, dmw, dml, 
			m_msr_db_map, m_databaseIdSet);
}
	

void CDnaGpsBaseline::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	m_dX = (vStations->at(m_lstn2Index).get()->GetXAxis() - vStations->at(m_lstn1Index).get()->GetXAxis());
	m_dY = (vStations->at(m_lstn2Index).get()->GetYAxis() - vStations->at(m_lstn1Index).get()->GetYAxis());
	m_dZ = (vStations->at(m_lstn2Index).get()->GetZAxis() - vStations->at(m_lstn1Index).get()->GetZAxis());

	m_dSigmaXX = 4.022E-05;                    
	m_dSigmaXY = -1.369E-05;
	m_dSigmaXZ = 3.975E-05;
	m_dSigmaYY = 1.487E-05;
	m_dSigmaYZ = -2.035E-05;
	m_dSigmaZZ = 6.803E-05;

	vector<CDnaCovariance>::iterator _it_cov = m_vGpsCovariances.begin();
	for (_it_cov=m_vGpsCovariances.begin(); _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		_it_cov->SimulateMsr(vStations, ellipsoid);
}
	

UINT32 CDnaGpsBaseline::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	char stationName[STN_NAME_WIDTH];

	// get first station name
	m_lstn1Index = measRecord->station1;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strFirst = stationName;
	// target station
	m_lstn2Index = measRecord->station2;
	ifs_stns->seekg(sizeof(UINT32) + measRecord->station2 * sizeof(station_t), ios::beg);
	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
	m_strTarget = stationName;

	m_dPscale = measRecord->scale1;
	m_dLscale = measRecord->scale2;
	m_dHscale = measRecord->scale3;
	m_dVscale = measRecord->scale4;

	m_epoch = measRecord->epoch;
	m_epsgCode = measRecord->epsgCode;
	m_referenceFrame = datumFromEpsgString<string>(measRecord->epsgCode);

	m_lclusterID = measRecord->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;

	// X, sigmaXX
	m_bIgnore = measRecord->ignore;
	m_strType = measRecord->measType;
	m_dX = measRecord->term1;
	m_dSigmaXX = measRecord->term2;
	m_lRecordedTotal = measRecord->vectorCount1;

	UINT32 measrecordCount = 0;

	// get data relating to Y, sigmaYY, sigmaXY
	if (ifs_msrs->eof() || !ifs_msrs->good())
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));

	// check integrity of the binary file (see WriteBinaryMsr())
	if (measRecord->measType != 'X' && measRecord->measType != 'G')
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsBaseline element (Y block).", 0);

	m_dY = measRecord->term1;
	m_dSigmaXY = measRecord->term2;
	m_dSigmaYY = measRecord->term3;

	measrecordCount++;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	if (ifs_msrs->eof() || !ifs_msrs->good())
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));

	// check integrity of the binary file (see WriteBinaryMsr())
	if (measRecord->measType != 'X' && measRecord->measType != 'G')
		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsBAseline element (Z block).", 0);

	m_dZ = measRecord->term1;
	m_dSigmaXZ = measRecord->term2;
	m_dSigmaYZ = measRecord->term3;
	m_dSigmaZZ = measRecord->term4;
	m_lRecordedTotal = measRecord->vectorCount1;

	measrecordCount++;

	m_vGpsCovariances.clear();
	m_vGpsCovariances.resize(measRecord->vectorCount2);

	// now covariances
	vector<CDnaCovariance>::iterator _it_cov = m_vGpsCovariances.begin();
	for (; _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		measrecordCount += _it_cov->SetMeasurementRec(ifs_stns, ifs_msrs, measRecord);

	return measrecordCount;
}


UINT32 CDnaGpsBaseline::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	// first station
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;

	// target station
	m_lstn2Index = it_msr->station2;
	m_strTarget = binaryStn.at(it_msr->station2).stationName;
	
	m_dPscale = it_msr->scale1;
	m_dLscale = it_msr->scale2;
	m_dHscale = it_msr->scale3;
	m_dVscale = it_msr->scale4;

	m_epoch = it_msr->epoch;
	m_epsgCode = it_msr->epsgCode;
	m_referenceFrame = datumFromEpsgString<string>(it_msr->epsgCode);

	m_lclusterID = it_msr->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;

	// X, sigmaXX
	m_bIgnore = it_msr->ignore;
	m_strType = it_msr->measType;
	m_dX = it_msr->term1;
	m_dSigmaXX = it_msr->term2;
	m_lRecordedTotal = it_msr->vectorCount1;

	it_msr++;
	
	m_dY = it_msr->term1;
	m_dSigmaXY = it_msr->term2;
	m_dSigmaYY = it_msr->term3;

	it_msr++;

	m_dZ = it_msr->term1;
	m_dSigmaXZ = it_msr->term2;
	m_dSigmaYZ = it_msr->term3;
	m_dSigmaZZ = it_msr->term4;
	m_lRecordedTotal = it_msr->vectorCount1;

	m_vGpsCovariances.clear();
	m_vGpsCovariances.resize(it_msr->vectorCount2);

	// now covariances
	vector<CDnaCovariance>::iterator _it_cov = m_vGpsCovariances.begin();
	for (; _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		_it_cov->SetMeasurementRec(binaryStn, it_msr);

	return it_msr->vectorCount1;
}


void CDnaGpsBaseline::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.station2 = m_lstn2Index;
	measRecord.vectorCount1 = m_lRecordedTotal;									// number of GpsBaselines in the parent cluster
	measRecord.vectorCount2 = static_cast<UINT32>(m_vGpsCovariances.size());	// number of Covariances in the GpsBaseline

	measRecord.clusterID = m_lclusterID;
	measRecord.measurementStations = m_MSmeasurementStations;

	measRecord.scale1 = m_dPscale;
	measRecord.scale2 = m_dLscale;
	measRecord.scale3 = m_dHscale;
	measRecord.scale4 = m_dVscale;

	sprintf(measRecord.epsgCode, "%s", m_epsgCode.substr(0, STN_EPSG_WIDTH).c_str());
	sprintf(measRecord.epoch, "%s", m_epoch.substr(0, STN_EPOCH_WIDTH).c_str());

	// X
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_dX;
	measRecord.term2 = m_dSigmaXX;	// already a variance
	measRecord.term3 = 0.;
	measRecord.term4 = 0.;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// Y
	measRecord.measStart = yMeas;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_dY;
	measRecord.term2 = m_dSigmaXY;	// already a variance
	measRecord.term3 = m_dSigmaYY;
	measRecord.term4 = 0.;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// Z
	measRecord.measStart = zMeas;
	measRecord.measAdj = m_measAdj;
	measRecord.measCorr = m_measCorr;
	measRecord.measAdjPrec = m_measAdjPrec;
	measRecord.residualPrec = m_residualPrec;
	measRecord.preAdjCorr = m_preAdjCorr;
	measRecord.term1 = m_dZ;
	measRecord.term2 = m_dSigmaXZ;		// already a variance
	measRecord.term3 = m_dSigmaYZ;
	measRecord.term4 = m_dSigmaZZ;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// now write covariance elements
	vector<CDnaCovariance>::const_iterator _it_cov;
	for (_it_cov=m_vGpsCovariances.begin(); _it_cov!=m_vGpsCovariances.end(); ++_it_cov)
		_it_cov->WriteBinaryMsr(binary_stream, msrIndex, m_epsgCode, m_epoch);
}

void CDnaGpsBaseline::SerialiseDatabaseMap(std::ofstream* os)
{
	// X
	CDnaMeasurement::SerialiseDatabaseMap(os);
	
	// Y
	CDnaMeasurement::SerialiseDatabaseMap(os);
	
	// Z
	CDnaMeasurement::SerialiseDatabaseMap(os);

	for_each(m_vGpsCovariances.begin(), m_vGpsCovariances.end(),
		[this, os](const CDnaCovariance& cov) {
		((CDnaCovariance*)&cov)->SerialiseDatabaseMap(os, m_msr_db_map.msr_id, m_msr_db_map.cluster_id);
	});
}



void CDnaGpsBaseline::SetX(const string& str)
{
	DoubleFromString(m_dX, trimstr(str));
}

void CDnaGpsBaseline::SetY(const string& str)
{
	DoubleFromString(m_dY, trimstr(str));
}

void CDnaGpsBaseline::SetZ(const string& str)
{
	DoubleFromString(m_dZ, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaXX(const string& str)
{
	if (DoubleFromString_ZeroCheck(m_dSigmaXX, trimstr(str)))
	{
		stringstream ss;
		ss << "SetSigmaXX(): Variances cannot be zero: " << str << ".";
		throw XMLInteropException(ss.str(), 0);
	}
	//DoubleFromString(m_dSigmaXX, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaXY(const string& str)
{
	DoubleFromString(m_dSigmaXY, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaXZ(const string& str)
{
	DoubleFromString(m_dSigmaXZ, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaYY(const string& str)
{
	if (DoubleFromString_ZeroCheck(m_dSigmaYY, trimstr(str)))
	{
		stringstream ss;
		ss << "SetSigmaYY(): Variances cannot be zero: " << str << ".";
		throw XMLInteropException(ss.str(), 0);
	}
	//DoubleFromString(m_dSigmaYY, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaYZ(const string& str)
{
	DoubleFromString(m_dSigmaYZ, trimstr(str));
}

void CDnaGpsBaseline::SetSigmaZZ(const string& str)
{
	if (DoubleFromString_ZeroCheck(m_dSigmaZZ, trimstr(str)))
	{
		stringstream ss;
		ss << "SetSigmaZZ(): Variances cannot be zero: " << str << ".";
		throw XMLInteropException(ss.str(), 0);
	}
	//DoubleFromString(m_dSigmaZZ, trimstr(str));
}

void CDnaGpsBaseline::SetEpoch(const string& epoch) 
{
	m_epoch = epoch;
}


void CDnaGpsBaseline::SetEpsg(const string& epsg) 
{
	m_epsgCode = epsg;
}


void CDnaGpsBaseline::SetReferenceFrame(const string& refFrame) 
{
	m_referenceFrame = refFrame;
	SetEpsg(epsgStringFromName<string>(refFrame));
}


void CDnaGpsBaseline::SetPscale(const string& str)
{
	DoubleFromString(m_dPscale, trimstr(str));
}

void CDnaGpsBaseline::SetLscale(const string& str)
{
	DoubleFromString(m_dLscale, trimstr(str));
}

void CDnaGpsBaseline::SetHscale(const string& str)
{
	DoubleFromString(m_dHscale, trimstr(str));
}

void CDnaGpsBaseline::SetVscale(const string& str)
{
	DoubleFromString(m_dVscale, trimstr(str));
}









CDnaGpsBaselineCluster::CDnaGpsBaselineCluster(void)
	: m_strTarget("")
	, m_lRecordedTotal(1)
	, m_dPscale(1.)
	, m_dLscale(1.)
	, m_dHscale(1.)
	, m_dVscale(1.)
	, m_referenceFrame(DEFAULT_DATUM)
	, m_epoch(DEFAULT_EPOCH)
	, m_lclusterID(0)
{
	SetEpsg(epsgStringFromName<string>(m_referenceFrame));

	m_vGpsBaselines.clear();
	m_vGpsBaselines.reserve(1);

	// m_strType will be set by the parsing function as "X" or "G"
	m_MSmeasurementStations = TWO_STATION;
}


CDnaGpsBaselineCluster::~CDnaGpsBaselineCluster(void)
{

}


CDnaGpsBaselineCluster::CDnaGpsBaselineCluster(const UINT32 lclusterID, const string& referenceframe, const string& epoch)
	: m_strTarget("")
	, m_lRecordedTotal(1)
	, m_dPscale(1.)
	, m_dLscale(1.)
	, m_dHscale(1.)
	, m_dVscale(1.)
	, m_referenceFrame(referenceframe)
	, m_epoch(epoch)
	, m_lclusterID(lclusterID)
{
	SetEpsg(epsgStringFromName<string>(referenceframe));

	m_vGpsBaselines.clear();
	m_vGpsBaselines.reserve(1);
	// m_strType will be set by the parsing function as "X" or "G"
	m_MSmeasurementStations = TWO_STATION;
}

CDnaGpsBaselineCluster::CDnaGpsBaselineCluster(const CDnaGpsBaselineCluster& newGpsBaselineCluster)
{
	m_strType = newGpsBaselineCluster.m_strType;
	m_bIgnore = newGpsBaselineCluster.m_bIgnore;
	m_strTarget = newGpsBaselineCluster.m_strTarget;
	m_lRecordedTotal = newGpsBaselineCluster.m_lRecordedTotal;
	m_vGpsBaselines = newGpsBaselineCluster.m_vGpsBaselines;

	m_dPscale = newGpsBaselineCluster.m_dPscale;
	m_dLscale = newGpsBaselineCluster.m_dLscale;
	m_dHscale = newGpsBaselineCluster.m_dHscale;
	m_dVscale = newGpsBaselineCluster.m_dVscale;
	m_lclusterID = newGpsBaselineCluster.m_lclusterID;
	m_MSmeasurementStations = newGpsBaselineCluster.m_MSmeasurementStations;
	
	m_referenceFrame = newGpsBaselineCluster.m_referenceFrame;
	m_epsgCode = newGpsBaselineCluster.m_epsgCode;
	m_epoch = newGpsBaselineCluster.m_epoch;

	m_databaseIdSet = newGpsBaselineCluster.m_databaseIdSet;
	m_msr_db_map = newGpsBaselineCluster.m_msr_db_map;
}


CDnaGpsBaselineCluster::CDnaGpsBaselineCluster(const bool bIgnore, const string& strType, const string& strFirstStation)
{
	m_strType = strType;
	m_bIgnore = bIgnore;
}


CDnaGpsBaselineCluster& CDnaGpsBaselineCluster::operator= (const CDnaGpsBaselineCluster& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(rhs);
	m_strTarget = rhs.m_strTarget;
	m_lRecordedTotal = rhs.m_lRecordedTotal;

	m_referenceFrame = rhs.m_referenceFrame;
	m_epsgCode = rhs.m_epsgCode;
	m_epoch = rhs.m_epoch;

	m_dPscale = rhs.m_dPscale;
	m_dLscale = rhs.m_dLscale;
	m_dHscale = rhs.m_dHscale;
	m_dVscale = rhs.m_dVscale;

	m_lclusterID = rhs.m_lclusterID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	m_vGpsBaselines = rhs.m_vGpsBaselines;

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	return *this;
}


bool CDnaGpsBaselineCluster::operator== (const CDnaGpsBaselineCluster& rhs) const
{
	return (
		m_strType == rhs.m_strType &&
		m_strFirst == rhs.m_strFirst &&
		m_bIgnore == rhs.m_bIgnore &&
		m_strTarget == rhs.m_strTarget &&
		m_lRecordedTotal == rhs.m_lRecordedTotal &&
		m_dPscale == rhs.m_dPscale &&
		m_dLscale == rhs.m_dLscale &&
		m_dHscale == rhs.m_dHscale &&
		m_dVscale == rhs.m_dVscale &&
		m_vGpsBaselines == rhs.m_vGpsBaselines &&
		iequals(m_referenceFrame, rhs.m_referenceFrame)
		);
}

bool CDnaGpsBaselineCluster::operator< (const CDnaGpsBaselineCluster& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {	// could be G, X
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_strTarget == rhs.m_strTarget) {
					if (m_lRecordedTotal == rhs.m_lRecordedTotal) 
						return m_vGpsBaselines < rhs.m_vGpsBaselines;
					else
						return m_lRecordedTotal < rhs.m_lRecordedTotal; }
				else
					return m_strTarget < rhs.m_strTarget; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}


void CDnaGpsBaselineCluster::ReserveGpsBaselinesCount(const UINT32& size)
{
	m_vGpsBaselines.reserve(size);
}

void CDnaGpsBaselineCluster::AddGpsBaseline(const CDnaMeasurement* pGpsBaseline)
{
	CDnaGpsBaseline bsl = (CDnaGpsBaseline&)*pGpsBaseline;
	m_vGpsBaselines.push_back(bsl);
}


void CDnaGpsBaselineCluster::ClearBaselines()
{
	m_vGpsBaselines.clear();
}


void CDnaGpsBaselineCluster::coutMeasurementData(ostream &os, const UINT16& uType) const
{

	switch (uType)
	{
	case DNA_COUT:
	case GEOLAB_COUT:
		coutMeasurement(os);
		break;
	}
	
	// NEWGAN_COUT	
	const size_t j = m_vGpsBaselines.size();
	if (j>1)
	{
		os << " (" << j << " GPS baselines)" << endl;
		for (UINT32 i=0; i<j; i++)
			m_vGpsBaselines[i].coutBaselineData(os, STN_WIDTH, uType);
	}
	else if (j == 1)
		m_vGpsBaselines[0].coutBaselineData(os, 12, uType);
	else // if (j < 1)
		os << endl;
}


//void CDnaGpsBaselineCluster::SetDatabaseMap_bmsIndex(const UINT32& bmsIndex) 
//{
//	UINT32 i(bmsIndex);
//	for_each(m_vGpsBaselines.begin(), m_vGpsBaselines.end(),
//		[this, &i](const CDnaGpsBaseline& bsl) {
//			((CDnaMeasurement*)&bsl)->SetDatabaseMap_bmsIndex(i);
//			i += bsl.CalcBinaryRecordCount();
//	});
//}
	

void CDnaGpsBaselineCluster::SerialiseDatabaseMap(std::ofstream* os)
{
	for_each(m_vGpsBaselines.begin(), m_vGpsBaselines.end(),
		[this, os](const CDnaGpsBaseline& bsl) {
		((CDnaGpsBaseline*)&bsl)->SerialiseDatabaseMap(os);
	});
	
}

// UINT32 CDnaGpsBaselineCluster::CalcDbidRecordCount() const
// {
// 	UINT32 recordCount(0);
// 	for_each(m_vGpsBaselines.begin(), m_vGpsBaselines.end(),
// 		[&recordCount](const CDnaGpsBaseline& bsl) {
// 			recordCount += bsl.CalcDbidRecordCount();
// 	});
// 	return recordCount;
// }
	
UINT32 CDnaGpsBaselineCluster::CalcBinaryRecordCount() const
{
	UINT32 recordCount(0);
	for_each(m_vGpsBaselines.begin(), m_vGpsBaselines.end(),
		[&recordCount](const CDnaGpsBaseline& bsl) {
			recordCount += bsl.CalcBinaryRecordCount();
	});
	return recordCount;
}


void CDnaGpsBaselineCluster::WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement /*= false*/) const
{
	const size_t bslCount = m_vGpsBaselines.size();
	char cType = GetTypeC();

	*dynaml_stream << "  <!--Type " << measurement_name<char, string>(GetTypeC());
	if (cType == 'X')
	{
		if (bslCount > 1)
			*dynaml_stream << " (set of " << bslCount << ")" << endl;
		else
			*dynaml_stream << "  (single)" << endl;
	}
	*dynaml_stream << "-->" << endl;
	*dynaml_stream << "  <DnaMeasurement>" << endl;
	*dynaml_stream << "    <Type>" << cType << "</Type>" << endl;
	// Source file from which the measurement came
	*dynaml_stream << "    <Source>" << m_sourceFile << "</Source>" << endl;
	if (m_bIgnore)
		*dynaml_stream << "    <Ignore>*</Ignore>" << endl;
	else
		*dynaml_stream << "    <Ignore/>" << endl;
	
	// Reference frame and epoch
	*dynaml_stream << "    <ReferenceFrame>" << m_referenceFrame << "</ReferenceFrame>" << endl;
	*dynaml_stream << "    <Epoch>" << m_epoch << "</Epoch>" << endl;

	// Scalars
	*dynaml_stream << "    <Vscale>" << fixed << setprecision(3) << m_dVscale << "</Vscale>" << endl;
	*dynaml_stream << "    <Pscale>" << m_dPscale << "</Pscale>" << endl;
	*dynaml_stream << "    <Lscale>" << m_dLscale << "</Lscale>" << endl;
	*dynaml_stream << "    <Hscale>" << m_dHscale << "</Hscale>" << endl;

	if (m_databaseIdSet)
		*dynaml_stream << "    <ClusterID>" << m_msr_db_map.cluster_id << "</ClusterID>" << endl;

	// baseline count
	if (cType == 'X')
		*dynaml_stream << "    <Total>" << bslCount << "</Total>" << endl;

	// write GpsBaselines
	vector<CDnaGpsBaseline>::const_iterator _it_bsl;
	for (_it_bsl=m_vGpsBaselines.begin(); _it_bsl!=m_vGpsBaselines.end(); ++_it_bsl)
		_it_bsl->WriteDynaMLMsr(dynaml_stream, true);

	*dynaml_stream << "  </DnaMeasurement>" << endl;

}
	

void CDnaGpsBaselineCluster::WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement /*= false*/) const
{
	// write GpsBaselines
	vector<CDnaGpsBaseline>::const_iterator _it_bsl;
	for (_it_bsl=m_vGpsBaselines.begin(); _it_bsl!=m_vGpsBaselines.end(); ++_it_bsl)
		_it_bsl->WriteDNAMsr(dynaml_stream, dmw, dml, true);
}
	

void CDnaGpsBaselineCluster::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	vector<CDnaGpsBaseline>::iterator _it_bsl;
	for (_it_bsl=m_vGpsBaselines.begin(); _it_bsl!=m_vGpsBaselines.end(); ++_it_bsl)
		_it_bsl->SimulateMsr(vStations, ellipsoid);
}
	

UINT32 CDnaGpsBaselineCluster::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
{
	m_lclusterID = measRecord->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
	m_lRecordedTotal = measRecord->vectorCount1;
	m_vGpsBaselines.clear();
	m_vGpsBaselines.resize(m_lRecordedTotal);

	m_referenceFrame = datumFromEpsgString<string>(measRecord->epsgCode);
	m_epoch = measRecord->epoch;
	m_epsgCode = measRecord->epsgCode;

	m_dPscale = measRecord->scale1;
	m_dLscale = measRecord->scale2;
	m_dHscale = measRecord->scale3;
	m_dVscale = measRecord->scale4;

	UINT32 measrecordCount = 0;

	// read remaining GpsPoint data and all Covariances from file
	for (UINT32 i=0; i<m_lRecordedTotal; i++)
	{
		if (i > 0)
		{
			// get data relating to Y, sigmaYY, sigmaXY
			if (ifs_msrs->eof() || !ifs_msrs->good())
				throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
			ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));

			// check integrity of the binary file (see WriteBinaryMsr())
			if (measRecord->measType != 'X' && measRecord->measType != 'G')
				throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint element (X block).", 0);
		}

		measrecordCount++;

		m_strType = measRecord->measType;
		m_bIgnore = measRecord->ignore;

		measrecordCount += m_vGpsBaselines.at(i).SetMeasurementRec(ifs_stns, ifs_msrs, measRecord);
	}
	return measrecordCount - 1;
}


UINT32 CDnaGpsBaselineCluster::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr)
{
	m_lclusterID = it_msr->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	m_lRecordedTotal = it_msr->vectorCount1;
	m_vGpsBaselines.clear();
	m_vGpsBaselines.resize(m_lRecordedTotal);

	m_referenceFrame = datumFromEpsgString<string>(it_msr->epsgCode);
	m_epoch = it_msr->epoch;
	m_epsgCode = it_msr->epsgCode;

	m_dPscale = it_msr->scale1;
	m_dLscale = it_msr->scale2;
	m_dHscale = it_msr->scale3;
	m_dVscale = it_msr->scale4;

	UINT32 measrecordCount(it_msr->vectorCount1);

	// read remaining GpsPoint data and all Covariances from file
	for (UINT32 i=0; i<m_lRecordedTotal; i++)
	{
		if (i > 0)
			it_msr++;		

		m_strType = it_msr->measType;
		m_bIgnore = it_msr->ignore;
		m_vGpsBaselines.at(i).SetMeasurementRec(binaryStn, it_msr);
	}
	return measrecordCount - 1;	
}


void CDnaGpsBaselineCluster::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	vector<CDnaGpsBaseline>::const_iterator _it_bsl;
	for (_it_bsl=m_vGpsBaselines.begin(); _it_bsl!=m_vGpsBaselines.end(); ++_it_bsl)
		_it_bsl->WriteBinaryMsr(binary_stream, msrIndex);
}


void CDnaGpsBaselineCluster::SetEpoch(const string& epoch) 
{
	// m_epoch is a member of CDnaGpsBaselineCluster
	m_epoch = epoch;
}


void CDnaGpsBaselineCluster::SetEpsg(const string& epsg) 
{
	// m_epsgCode is a member of CDnaMeasurement
	m_epsgCode = epsg;
}


void CDnaGpsBaselineCluster::SetReferenceFrame(const string& refFrame) 
{
	// m_referenceFrame is a member of CDnaGpsBaselineCluster
	m_referenceFrame = refFrame;
	SetEpsg(epsgStringFromName<string>(refFrame));
}


void CDnaGpsBaselineCluster::SetTotal(const string& str)
{
	m_lRecordedTotal = LongFromString<UINT32>(trimstr(str));
	m_vGpsBaselines.reserve(m_lRecordedTotal);
}

void CDnaGpsBaselineCluster::SetPscale(const string& str)
{
	DoubleFromString(m_dPscale, trimstr(str));
}

void CDnaGpsBaselineCluster::SetLscale(const string& str)
{
	DoubleFromString(m_dLscale, trimstr(str));
}

void CDnaGpsBaselineCluster::SetHscale(const string& str)
{
	DoubleFromString(m_dHscale, trimstr(str));
}

void CDnaGpsBaselineCluster::SetVscale(const string& str)
{
	DoubleFromString(m_dVscale, trimstr(str));
}




}	// namespace measurements
}	// namespace dynadjust
