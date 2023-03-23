//============================================================================
// Name         : dnagpspoint.cpp
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
// Description  : CDnaGpsPoint and CDnaGpsPointCluster implementation file
//============================================================================

#include <include/exception/dnaexception.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/measurement_types/dnagpspoint.hpp>

using namespace dynadjust::exception;
using namespace dynadjust::epsg;
using namespace dynadjust::math;

namespace dynadjust {
namespace measurements {

CDnaGpsPoint::CDnaGpsPoint(void)
	: m_lRecordedTotal(0)
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
	, m_strCoordType("XYZ")
	, m_ctType(XYZ_type_i)
	, m_referenceFrame(DEFAULT_DATUM)
	, m_lclusterID(0)
	
{
	SetEpoch(DEFAULT_EPOCH);
	SetEpsg(epsgStringFromName<string>(m_referenceFrame));

	m_vPointCovariances.clear();
	m_vPointCovariances.reserve(0);
	m_MSmeasurementStations = ONE_STATION;
}


CDnaGpsPoint::~CDnaGpsPoint(void)
{

}

// move constructor
CDnaGpsPoint::CDnaGpsPoint(CDnaGpsPoint&& p)
{
	m_strType = p.m_strType;
	m_strFirst = p.m_strFirst;
	m_lRecordedTotal = p.m_lRecordedTotal;
	m_bIgnore = p.m_bIgnore;

	m_referenceFrame = p.m_referenceFrame;
	m_epsgCode = p.m_epsgCode;
	m_epoch = p.m_epoch;

	m_dX = p.m_dX;
	m_dY = p.m_dY;
	m_dZ = p.m_dZ;
	m_dSigmaXX = p.m_dSigmaXX;
	m_dSigmaXY = p.m_dSigmaXY;
	m_dSigmaXZ = p.m_dSigmaXZ;
	m_dSigmaYY = p.m_dSigmaYY;
	m_dSigmaYZ = p.m_dSigmaYZ;
	m_dSigmaZZ = p.m_dSigmaZZ;

	m_dPscale = p.m_dPscale;
	m_dLscale = p.m_dLscale;
	m_dHscale = p.m_dHscale;
	m_dVscale = p.m_dVscale;

	SetCoordType(p.m_strCoordType);
	
	m_lclusterID = p.m_lclusterID;
	m_MSmeasurementStations = p.m_MSmeasurementStations;

	m_databaseIdSet = p.m_databaseIdSet;
	m_msr_db_map = p.m_msr_db_map;

	m_vPointCovariances = std::move(p.m_vPointCovariances);
}

// move assignment operator
CDnaGpsPoint& CDnaGpsPoint::operator= (CDnaGpsPoint&& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(std::move(rhs));
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

	SetCoordType(rhs.m_strCoordType);
	
	m_lclusterID = rhs.m_lclusterID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	m_vPointCovariances = std::move(rhs.m_vPointCovariances);

	return *this;
}

// copy constructor (disabled)
//CDnaGpsPoint::CDnaGpsPoint(const CDnaGpsPoint& newGpsPoint)
//{
//	m_strType = newGpsPoint.m_strType;
//	m_strFirst = newGpsPoint.m_strFirst;
//	m_lRecordedTotal = newGpsPoint.m_lRecordedTotal;
//	m_bIgnore = newGpsPoint.m_bIgnore;
//
//	m_referenceFrame = newGpsPoint.m_referenceFrame;
//	m_epsgCode = newGpsPoint.m_epsgCode;
//	m_epoch = newGpsPoint.m_epoch;
//
//	m_dX = newGpsPoint.m_dX;
//	m_dY = newGpsPoint.m_dY;
//	m_dZ = newGpsPoint.m_dZ;
//	m_dSigmaXX = newGpsPoint.m_dSigmaXX;
//	m_dSigmaXY = newGpsPoint.m_dSigmaXY;
//	m_dSigmaXZ = newGpsPoint.m_dSigmaXZ;
//	m_dSigmaYY = newGpsPoint.m_dSigmaYY;
//	m_dSigmaYZ = newGpsPoint.m_dSigmaYZ;
//	m_dSigmaZZ = newGpsPoint.m_dSigmaZZ;
//
//	m_dPscale = newGpsPoint.m_dPscale;
//	m_dLscale = newGpsPoint.m_dLscale;
//	m_dHscale = newGpsPoint.m_dHscale;
//	m_dVscale = newGpsPoint.m_dVscale;
//
//	SetCoordType(newGpsPoint.m_strCoordType);
//	
//	m_lclusterID = newGpsPoint.m_lclusterID;
//	m_MSmeasurementStations = newGpsPoint.m_MSmeasurementStations;
//
//	m_databaseIdSet = newGpsPoint.m_databaseIdSet;
//	m_msr_db_map = newGpsPoint.m_msr_db_map;
//
//	m_vPointCovariances = newGpsPoint.m_vPointCovariances;
//}


//CDnaGpsPoint::CDnaGpsPoint(const bool bIgnore, const string& strType, const string& strFirstStation)
//{
//	m_strType = strType;
//	m_strFirst = strFirstStation;
//	m_bIgnore = bIgnore;
//
//	m_referenceFrame = DEFAULT_DATUM;
//	m_epoch = DEFAULT_EPOCH;
//	SetEpsg(epsgStringFromName<string>(m_referenceFrame));
//}

// assignment operator (disabled)
//CDnaGpsPoint& CDnaGpsPoint::operator= (const CDnaGpsPoint& rhs)
//{
//	// check for assignment to self!
//	if (this == &rhs)
//		return *this;
//
//	CDnaMeasurement::operator=(rhs);
//	m_lRecordedTotal = rhs.m_lRecordedTotal;
//
//	m_referenceFrame = rhs.m_referenceFrame;
//	m_epsgCode = rhs.m_epsgCode;
//	m_epoch = rhs.m_epoch;
//
//	m_dX = rhs.m_dX;
//	m_dY = rhs.m_dY;
//	m_dZ = rhs.m_dZ;
//	m_dSigmaXX = rhs.m_dSigmaXX;
//	m_dSigmaXY = rhs.m_dSigmaXY;
//	m_dSigmaXZ = rhs.m_dSigmaXZ;
//	m_dSigmaYY = rhs.m_dSigmaYY;
//	m_dSigmaYZ = rhs.m_dSigmaYZ;
//	m_dSigmaZZ = rhs.m_dSigmaZZ;
//
//	m_dPscale = rhs.m_dPscale;
//	m_dLscale = rhs.m_dLscale;
//	m_dHscale = rhs.m_dHscale;
//	m_dVscale = rhs.m_dVscale;
//
//	SetCoordType(rhs.m_strCoordType);
//	
//	m_lclusterID = rhs.m_lclusterID;
//	m_MSmeasurementStations = rhs.m_MSmeasurementStations;
//
//	m_databaseIdSet = rhs.m_databaseIdSet;
//	m_msr_db_map = rhs.m_msr_db_map;
//
//	m_vPointCovariances = rhs.m_vPointCovariances;
//
//	return *this;
//}


bool CDnaGpsPoint::operator== (const CDnaGpsPoint& rhs) const
{
	return (
		m_strFirst == rhs.m_strFirst &&
		m_strType == rhs.m_strType &&
		m_lRecordedTotal == rhs.m_lRecordedTotal &&
		m_bIgnore == rhs.m_bIgnore &&
		fabs(m_dX - rhs.m_dX) < PRECISION_1E4 &&
		fabs(m_dY - rhs.m_dY) < PRECISION_1E4 &&
		fabs(m_dZ - rhs.m_dZ) < PRECISION_1E4 &&
		iequals(m_referenceFrame, rhs.m_referenceFrame)
		);
}
	

bool CDnaGpsPoint::operator< (const CDnaGpsPoint& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {
			if (m_lRecordedTotal == rhs.m_lRecordedTotal) {
				if (m_bIgnore == rhs.m_bIgnore) {
					if (m_epoch == rhs.m_epoch) {
						if (fabs(m_dX - rhs.m_dX) < PRECISION_1E4) {
							if (fabs(m_dY - rhs.m_dY) < PRECISION_1E4) {
								return m_dZ < rhs.m_dZ; }
							else
								return m_dY < rhs.m_dY; }
						else
							return m_dX < rhs.m_dX; }
					else
						return m_epoch < rhs.m_epoch; }
				else
					return m_bIgnore < rhs.m_bIgnore; }
			else
				return m_lRecordedTotal < rhs.m_lRecordedTotal;	}
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}

void CDnaGpsPoint::ReserveGpsCovariancesCount(const UINT32& size)
{
	m_vPointCovariances.reserve(size);
}


void CDnaGpsPoint::ResizeGpsCovariancesCount(const UINT32& size)
{
	m_vPointCovariances.resize(size);
}


void CDnaGpsPoint::SetCoordType(const string& sType) {
	m_strCoordType = trimstr(sType);
	m_ctType = GetMyCoordTypeC();
}
	

_COORD_TYPE_ CDnaGpsPoint::GetMyCoordTypeC()
{
	return CDnaStation::GetCoordTypeC(m_strCoordType);
	//// case insensitive
	//if (iequals(m_strCoordType, XYZ_type))
	//	return XYZ_type_i;
	//if (iequals(m_strCoordType, UTM_type))
	//	return UTM_type_i;
	//return LLH_type_i;	// default
}
	

void CDnaGpsPoint::AddPointCovariance(const CDnaCovariance* pGpsCovariance)
{
	//CDnaCovariance vcv = (CDnaCovariance&)*pGpsCovariance;
	//m_vPointCovariances.push_back(vcv);
	m_vPointCovariances.push_back(std::move((CDnaCovariance&)*pGpsCovariance));
}
	

UINT32 CDnaGpsPoint::CalcBinaryRecordCount() const
{
	UINT32 RecordCount = 3;
	vector<CDnaCovariance>::const_iterator _it_cov = m_vPointCovariances.begin();
	for (; _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		RecordCount += _it_cov->CalcBinaryRecordCount();
	return RecordCount;
}
	

void CDnaGpsPoint::WriteDynaMLMsr(std::ofstream* dynaml_stream, const string&, bool) const
{
	UINT32 precision(4);
	
	*dynaml_stream << "    <First>" << m_strFirst << "</First>" << endl;
	*dynaml_stream << "    <Clusterpoint>" << endl;
	
	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
	{
		precision = 10;
		*dynaml_stream << "      <X>" << fixed << setprecision(precision) << RadtoDms(m_dX) << "</X>" << endl;
		*dynaml_stream << "      <Y>" << RadtoDmsL(m_dY) << "</Y>" << endl;
	}
	else
	{
		*dynaml_stream << "      <X>" << fixed << setprecision(precision) << m_dX << "</X>" << endl;
		*dynaml_stream << "      <Y>" << m_dY << "</Y>" << endl;
	}

	*dynaml_stream << "      <Z>" << setprecision(4) << m_dZ << "</Z>" << endl;

	if (m_databaseIdSet)
		if (m_msr_db_map.is_msr_id_set)
			*dynaml_stream << "      <MeasurementID>" << m_msr_db_map.msr_id << "</MeasurementID>" << endl;
	
	*dynaml_stream << "      <SigmaXX>" << scientific << setprecision(13) << m_dSigmaXX << "</SigmaXX>" << endl;
	*dynaml_stream << "      <SigmaXY>" << m_dSigmaXY << "</SigmaXY>" << endl;
	*dynaml_stream << "      <SigmaXZ>" << m_dSigmaXZ << "</SigmaXZ>" << endl;
	*dynaml_stream << "      <SigmaYY>" << m_dSigmaYY << "</SigmaYY>" << endl;
	*dynaml_stream << "      <SigmaYZ>" << m_dSigmaYZ << "</SigmaYZ>" << endl;
	*dynaml_stream << "      <SigmaZZ>" << m_dSigmaZZ << "</SigmaZZ>" << endl;
	
	// write GPSPoint covariances
	vector<CDnaCovariance>::const_iterator _it_cov = m_vPointCovariances.begin();
	for (; _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		_it_cov->WriteDynaMLMsr(dynaml_stream);
	
	*dynaml_stream << "    </Clusterpoint>" << endl;
}
	
void CDnaGpsPoint::WriteDNAMsr(std::ofstream* dna_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool) const
{
	*dna_stream << setw(dmw.msr_type) << m_strType;
	if (m_bIgnore)
		*dna_stream << setw(dmw.msr_ignore) << "*";
	else
		*dna_stream << setw(dmw.msr_ignore) << " ";

	*dna_stream << left << setw(dmw.msr_inst) << m_strFirst;
	
	// Print header for first cluster point
	if (m_lRecordedTotal == m_vPointCovariances.size() + 1)
	{
		if (m_ctType == LLH_type_i)
			*dna_stream << left << setw(dmw.msr_targ1) << LLH_type;
		else if (m_ctType == LLh_type_i)
			*dna_stream << left << setw(dmw.msr_targ1) << LLh_type;
		else
			*dna_stream << left << setw(dmw.msr_targ1) << XYZ_type;

		*dna_stream << left << setw(dmw.msr_targ2) << m_lRecordedTotal;

		// print scaling
		*dna_stream << 
			fixed << setprecision(2) << 
			right << setw(dmw.msr_gps_vscale) << double_string_width<double, UINT32, string>(m_dVscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_pscale) << double_string_width<double, UINT32, string>(m_dPscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_lscale) << double_string_width<double, UINT32, string>(m_dLscale, dmw.msr_gps_vscale) <<
			right << setw(dmw.msr_gps_hscale) << double_string_width<double, UINT32, string>(m_dHscale, dmw.msr_gps_vscale);
		
		// print reference frame and epoch
		*dna_stream <<
			right << setw(dmw.msr_gps_reframe) << m_referenceFrame <<
			right << setw(dmw.msr_gps_epoch) << m_epoch;

		// print database ids
		if (m_databaseIdSet)
		{
			if (m_msr_db_map.is_msr_id_set)
				*dna_stream << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
			if (m_msr_db_map.is_cls_id_set)
				*dna_stream << setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
		}
	}
	else
	{
		// print database ids
		if (m_databaseIdSet)
		{
			if (m_msr_db_map.is_msr_id_set)
				*dna_stream << setw(dml.msr_id_msr - dml.msr_targ1) << " " <<
					right << setw(dmw.msr_id_msr) << m_msr_db_map.msr_id;
			if (m_msr_db_map.is_cls_id_set)
				*dna_stream << setw(dmw.msr_id_cluster) << m_msr_db_map.cluster_id;
		}
	}

	*dna_stream << endl;

	UINT32 precision = 4;
	UINT32 pad(dmw.msr_type + dmw.msr_ignore + dmw.msr_inst + dmw.msr_targ1 + dmw.msr_targ2);

	// X
	*dna_stream << setw(pad) << " ";
	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
	{
		precision = 13;
		*dna_stream << right << setw(dmw.msr_gps) << fixed << setprecision(precision) << RadtoDms(m_dX);
	}
	else
		*dna_stream << right << setw(dmw.msr_gps) << fixed << setprecision(precision) << m_dX;

	*dna_stream << right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXX;
	*dna_stream << endl;

	// Y
	*dna_stream << setw(pad) << " ";
	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
		*dna_stream << right << setw(dmw.msr_gps) << fixed << setprecision(precision) << RadtoDmsL(m_dY);
	else
		*dna_stream << right << setw(dmw.msr_gps) << fixed << setprecision(precision) << m_dY;

	*dna_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXY << 
		right << setw(dmw.msr_gps_vcv_2) << m_dSigmaYY;
	*dna_stream << endl;

	// Z
	precision = 4;	// whether XYZ or LLH, precision only needs to be at 4
	*dna_stream << setw(pad) << " ";
	*dna_stream << right << setw(dmw.msr_gps) << fixed << setprecision(precision) << m_dZ;
	*dna_stream << 
		right << setw(dmw.msr_gps_vcv_1) << scientific << setprecision(13) << m_dSigmaXZ <<
		right << setw(dmw.msr_gps_vcv_2) << m_dSigmaYZ << 
		right << setw(dmw.msr_gps_vcv_3) << m_dSigmaZZ;
	*dna_stream << endl;

	// write GPSPoint covariances (not supported by DNA format)
	vector<CDnaCovariance>::const_iterator _it_cov = m_vPointCovariances.begin();
	for (_it_cov=m_vPointCovariances.begin(); _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		_it_cov->WriteDNAMsr(dna_stream, dmw, dml,
			m_msr_db_map, m_databaseIdSet);
}
	

void CDnaGpsPoint::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	m_dX = vStations->at(m_lstn1Index).get()->GetXAxis();
	m_dY = vStations->at(m_lstn1Index).get()->GetYAxis();
	m_dZ = vStations->at(m_lstn1Index).get()->GetZAxis();

	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
	{
		// propagate to geographic
		CartToGeo<double>(m_dX, m_dY, m_dZ, &m_dX, &m_dY, &m_dZ, ellipsoid);

		// Reduce to orthometric height
		if (fabs(vStations->at(m_lstn1Index).get()->GetgeoidSep()) > PRECISION_1E4)
			m_dZ -= vStations->at(m_lstn1Index).get()->GetgeoidSep();

		m_dSigmaXX = 9.402e-009;
		m_dSigmaXY = 5.876e-010;
		m_dSigmaYY = 9.402e-009;
		m_dSigmaXZ = 5.876e-010;
		m_dSigmaYZ = 5.876e-010;
		m_dSigmaZZ = 2.500e-001;		
	}
	else
	{
		m_dSigmaXX = 4.022E-04;                    
		m_dSigmaXY = -1.369E-04;
		m_dSigmaXZ = 3.975E-04;
		m_dSigmaYY = 1.487E-04;
		m_dSigmaYZ = -2.035E-04;
		m_dSigmaZZ = 6.803E-04;
	}

	vector<CDnaCovariance>::iterator _it_cov = m_vPointCovariances.begin();
	for (_it_cov=m_vPointCovariances.begin(); _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		_it_cov->SimulateMsr(vStations, ellipsoid);
}

void CDnaGpsPoint::PopulateMsr(pvstn_t bstRecords, uint32_uint32_map* blockStationsMap, vUINT32* blockStations,
		const UINT32& map_idx, const CDnaDatum* datum, matrix_2d* estimates, matrix_2d* variances)
{
	// populate station coordinate information
	m_strType = 'Y';
	m_bIgnore = false;
	SetCoordType(XYZ_type);

	m_lstn1Index = blockStations->at(map_idx);
	m_strFirst = bstRecords->at(m_lstn1Index).stationName;
	m_MSmeasurementStations = ONE_STATION;

	m_referenceFrame = datum->GetName();
	m_epoch = datum->GetEpoch_s();
	m_epsgCode =  datum->GetEpsgCode_s();

	m_dPscale = 1.0;
	m_dLscale = 1.0;
	m_dHscale = 1.0;
	m_dVscale = 1.0;

	// populate station coordinate estimates and unertainties
	UINT32 mat_idx = (*blockStationsMap)[m_lstn1Index] * 3;
	m_dX = estimates->get(mat_idx, 0);
	m_dY = estimates->get(mat_idx+1, 0);
	m_dZ = estimates->get(mat_idx+2, 0);

	m_dSigmaXX = variances->get(mat_idx, mat_idx);
	m_dSigmaXY = variances->get(mat_idx, mat_idx+1);
	m_dSigmaXZ = variances->get(mat_idx, mat_idx+2);
	m_dSigmaYY = variances->get(mat_idx+1, mat_idx+1);
	m_dSigmaYZ = variances->get(mat_idx+1, mat_idx+2);
	m_dSigmaZZ = variances->get(mat_idx+2, mat_idx+2);

	// populate station covariances
	UINT32 covariance_count = static_cast<UINT32>(blockStations->size()) - map_idx - 1;
	m_vPointCovariances.clear();
	m_vPointCovariances.resize(covariance_count);

	vector<CDnaCovariance>::iterator _it_cov = m_vPointCovariances.begin();
	matrix_2d variances_cart(3, 3);
	UINT32 ic, jc;

	for (ic=map_idx+1; ic<blockStationsMap->size(); ++ic)
	{
		jc = (*blockStationsMap)[blockStations->at(ic)] * 3;
		
		// get cartesian submatrix corresponding to the covariance
		variances->submatrix(mat_idx, jc, &variances_cart, 3, 3);
	
		_it_cov->SetM11(variances_cart.get(0, 0));
		_it_cov->SetM12(variances_cart.get(0, 1));
		_it_cov->SetM13(variances_cart.get(0, 2));
		_it_cov->SetM21(variances_cart.get(1, 0));
		_it_cov->SetM22(variances_cart.get(1, 1));
		_it_cov->SetM23(variances_cart.get(1, 2));
		_it_cov->SetM31(variances_cart.get(2, 0));
		_it_cov->SetM32(variances_cart.get(2, 1));
		_it_cov->SetM33(variances_cart.get(2, 2));
		
		++_it_cov;
	}
}

	

//UINT32 CDnaGpsPoint::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
//{
//	char stationName[STN_NAME_WIDTH];
//
//	// get first station name 
//	m_lstn1Index = measRecord->station1;
//	ifs_stns->seekg(sizeof(UINT32) + measRecord->station1 * sizeof(station_t), ios::beg);
//	ifs_stns->read(reinterpret_cast<char *>(&stationName), sizeof(stationName));
//	m_strFirst = stationName;
//
//	m_dPscale = measRecord->scale1;
//	m_dLscale = measRecord->scale2;
//	m_dHscale = measRecord->scale3;
//	m_dVscale = measRecord->scale4;
//
//	m_epoch = measRecord->epoch;
//	m_epsgCode = measRecord->epsgCode;
//	m_referenceFrame = datumFromEpsgCode<string, UINT32>(LongFromString<UINT32>(measRecord->epsgCode));
//
//	m_lclusterID = measRecord->clusterID;
//	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
//	
//	// X, sigmaXX
//	m_bIgnore = measRecord->ignore;
//	m_strType = measRecord->measType;
//	m_dX = measRecord->term1;
//	m_dSigmaXX = measRecord->term2;
//	m_lRecordedTotal = measRecord->vectorCount1;
//	SetCoordType(measRecord->coordType);
//	
//	UINT32 measrecordCount = 0;
//	
//	// get data relating to Y, sigmaYY, sigmaXY
//	if (ifs_msrs->eof() || !ifs_msrs->good())
//		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
//	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
//	
//	// check integrity of the binary file (see WriteBinaryMsr())
//	if (measRecord->measType != 'Y')
//		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint element (Y block).", 0);
//	
//	m_dY = measRecord->term1;
//	m_dSigmaXY = measRecord->term2;
//	m_dSigmaYY = measRecord->term3;
//
//	measrecordCount++;
//
//	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
//	if (ifs_msrs->eof() || !ifs_msrs->good())
//		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
//	ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
//	
//	// check integrity of the binary file (see WriteBinaryMsr())
//	if (measRecord->measType != 'Y')
//		throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint element (Z block).", 0);
//	
//	m_dZ = measRecord->term1;
//	m_dSigmaXZ = measRecord->term2;
//	m_dSigmaYZ = measRecord->term3;
//	m_dSigmaZZ = measRecord->term4;
//
//	measrecordCount++;
//
//	m_vPointCovariances.clear();
//	m_vPointCovariances.resize(measRecord->vectorCount2);
//
//	// now covariances
//	vector<CDnaCovariance>::iterator _it_cov = m_vPointCovariances.begin();
//	for (; _it_cov!=m_vPointCovariances.end(); ++_it_cov)
//		measrecordCount += _it_cov->SetMeasurementRec(ifs_stns, ifs_msrs, measRecord);
//
//	return measrecordCount;
//}
	

UINT32 CDnaGpsPoint::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr, it_vdbid_t& dbidmap, bool dbidSet)
{
	// first station
	m_lstn1Index = it_msr->station1;
	m_strFirst = binaryStn.at(it_msr->station1).stationName;
	
	m_dPscale = it_msr->scale1;
	m_dLscale = it_msr->scale2;
	m_dHscale = it_msr->scale3;
	m_dVscale = it_msr->scale4;

	m_epoch = it_msr->epoch;
	m_epsgCode = it_msr->epsgCode;
	m_referenceFrame = datumFromEpsgCode<string, UINT32>(LongFromString<UINT32>(it_msr->epsgCode));

	m_lclusterID = it_msr->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	
	// X, sigmaXX
	m_bIgnore = it_msr->ignore;
	m_strType = it_msr->measType;
	m_dX = it_msr->term1;
	m_dSigmaXX = it_msr->term2;
	m_lRecordedTotal = it_msr->vectorCount1;
	SetCoordType(it_msr->coordType);
	
	// get data relating to Y, sigmaYY, sigmaXY
	it_msr++;
	
	m_dY = it_msr->term1;
	m_dSigmaXY = it_msr->term2;
	m_dSigmaYY = it_msr->term3;

	// get data relating to Z, sigmaZZ, sigmaXZ, sigmaYZ
	it_msr++;
	
	m_dZ = it_msr->term1;
	m_dSigmaXZ = it_msr->term2;
	m_dSigmaYZ = it_msr->term3;
	m_dSigmaZZ = it_msr->term4;

	m_vPointCovariances.clear();
	m_vPointCovariances.resize(it_msr->vectorCount2);

	if (dbidSet)
		CDnaMeasurement::SetDatabaseMap(*dbidmap, dbidSet);

	// now covariances
	vector<CDnaCovariance>::iterator _it_cov = m_vPointCovariances.begin();
	for (; _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		_it_cov->SetMeasurementRec(binaryStn, it_msr, dbidmap, dbidSet);

	return it_msr->vectorCount1;
}
	

void CDnaGpsPoint::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	measurement_t measRecord;
	
	// Common
	measRecord.measType = GetTypeC();
	measRecord.measStart = xMeas;
	strcpy(measRecord.coordType, m_strCoordType.c_str());
	measRecord.ignore = m_bIgnore;
	measRecord.station1 = m_lstn1Index;
	measRecord.vectorCount1 = m_lRecordedTotal;									// number of GpsPoints in the cluster
	measRecord.vectorCount2 = static_cast<UINT32>(m_vPointCovariances.size());	// number of Covariances in the GpsPoint

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
	measRecord.term2 = m_dSigmaXY;
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
	measRecord.term2 = m_dSigmaXZ;
	measRecord.term3 = m_dSigmaYZ;
	measRecord.term4 = m_dSigmaZZ;
	measRecord.fileOrder = ((*msrIndex)++);

	binary_stream->write(reinterpret_cast<char *>(&measRecord), sizeof(measurement_t));

	// now write covariance elements
	vector<CDnaCovariance>::const_iterator _it_cov;
	for (_it_cov=m_vPointCovariances.begin(); _it_cov!=m_vPointCovariances.end(); ++_it_cov)
		_it_cov->WriteBinaryMsr(binary_stream, msrIndex, m_epsgCode, m_epoch);
}


void CDnaGpsPoint::SerialiseDatabaseMap(std::ofstream* os)
{
	// X
	CDnaMeasurement::SerialiseDatabaseMap(os);
	
	// Y
	CDnaMeasurement::SerialiseDatabaseMap(os);
	
	// Z
	CDnaMeasurement::SerialiseDatabaseMap(os);

	for_each(m_vPointCovariances.begin(), m_vPointCovariances.end(),
		[this, os](const CDnaCovariance& cov) {
		((CDnaCovariance*)&cov)->SerialiseDatabaseMap(os, m_msr_db_map.msr_id, m_msr_db_map.cluster_id);
	});
}
	

//void CDnaGpsPoint::coutPointData(ostream &os) const
//{
//	os << left << setw(4) << " " << setw(INST_WIDTH) << m_strFirst;
//	os << setw(TARG_WIDTH) << " ";
//	string ignoreFlag = " ";
//	if (m_bIgnore)
//		ignoreFlag = "*";
//	bool bCoords_in_XYZ = true;
//	if (m_strCoordType.compare(LLH_type))
//		bCoords_in_XYZ = false;
//
//	size_t j = m_vPointCovariances.size();
//	os << left << setw(2) << ignoreFlag << setw(8) << setprecision(3) << fixed << m_dPscale << setw(2) << (bCoords_in_XYZ ? "X" : "P") << setw(MSR2_WIDTH) << right << m_dX << setw(VAR_WIDTH) << scientific << m_dSigmaXX << setw(VAR_WIDTH) << m_dSigmaXY << setw(VAR_WIDTH) << m_dSigmaXZ;
//	for (size_t i=0; i<j; i++)
//		os << setw(COV_WIDTH) << m_vPointCovariances[i].GetM11() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM12() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM13();
//	os << endl;
//	os << setw(28) << " " << left << setw(2) << ignoreFlag << setw(8) << fixed << m_dLscale << setw(2) << (bCoords_in_XYZ ? "Y" : "L") << setw(MSR2_WIDTH) << right << m_dY << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << scientific << m_dSigmaYY << setw(VAR_WIDTH) << m_dSigmaYZ;
//	for (size_t i=0; i<j; i++)
//		os << setw(COV_WIDTH) << m_vPointCovariances[i].GetM21() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM22() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM23();
//	os << endl;
//	os << setw(28) << " " << left << setw(2) << ignoreFlag << setw(8) << fixed << m_dHscale << setw(2) << (bCoords_in_XYZ ? "Z" : "H")  << setw(MSR2_WIDTH) << right << m_dZ << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << " " << setw(VAR_WIDTH) << scientific << m_dSigmaZZ;
//	for (size_t i=0; i<j; i++)
//		os << setw(COV_WIDTH) << m_vPointCovariances[i].GetM31() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM32() << setw(COV_WIDTH) << m_vPointCovariances[i].GetM33();
//	os << endl;
//}


// moved to dnameasurement.cpp
//void CDnaGpsPoint::SetEpoch(const string& epoch) 
//{
//	m_epoch = epoch;
//}


void CDnaGpsPoint::SetEpsg(const string& epsg) 
{
	m_epsgCode = epsg;
}


void CDnaGpsPoint::SetReferenceFrame(const string& refFrame) 
{
	m_referenceFrame = refFrame;
	SetEpsg(epsgStringFromName<string>(refFrame));
}


//void CDnaGpsPoint::SetPscale(const string& str)
//{
//	DoubleFromString(m_dPscale, trimstr(str));
//}

//void CDnaGpsPoint::SetLscale(const string& str)
//{
//	DoubleFromString(m_dLscale, trimstr(str));
//}

//void CDnaGpsPoint::SetHscale(const string& str)
//{
//	DoubleFromString(m_dHscale, trimstr(str));
//}

//void CDnaGpsPoint::SetVscale(const string& str)
//{
//	DoubleFromString(m_dVscale, trimstr(str));
//}

void CDnaGpsPoint::SetX(const string& str)
{
	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
	{
		// latitude
		FromDmsString(&m_dX, trimstr(str));
		m_dX = Radians(m_dX);
	}
	else
		DoubleFromString(m_dX, trimstr(str));
}

void CDnaGpsPoint::SetY(const string& str)
{
	if (m_ctType == LLH_type_i || m_ctType == LLh_type_i)
	{
		// longitude
		FromDmsString(&m_dY, trimstr(str));
		m_dY = Radians(m_dY);
	}
	else
		DoubleFromString(m_dY, trimstr(str));
}
	
void CDnaGpsPoint::SetZ(const string& str)
{
	// if (m_ctType == LLH_type_i)
	// then height should be ellipsoid height (but input files show height to be orthometric!)
	DoubleFromString(m_dZ, trimstr(str));
}
	
void CDnaGpsPoint::SetSigmaXX(const string& str)
{
	// if <Coords>LLH</Coords>, then SigmaXX is in radians^2
	// if <Coords>XYZ</Coords>, then SigmaXX is in metres^2
	if (DoubleFromString_ZeroCheck(m_dSigmaXX, trimstr(str)))
		throw XMLInteropException("SetSigmaXX(): Variances cannot be zero.", 0);
}

void CDnaGpsPoint::SetSigmaXY(const string& str)
{
	DoubleFromString(m_dSigmaXY, trimstr(str));
}

void CDnaGpsPoint::SetSigmaXZ(const string& str)
{
	DoubleFromString(m_dSigmaXZ, trimstr(str));
}

void CDnaGpsPoint::SetSigmaYY(const string& str)
{
	// if <Coords>LLH</Coords>, then SigmaYY is in radians^2
	// if <Coords>XYZ</Coords>, then SigmaYY is in metres^2
	if (DoubleFromString_ZeroCheck(m_dSigmaYY, trimstr(str)))
		throw XMLInteropException("SetSigmaYY(): Variances cannot be zero.", 0);
}

void CDnaGpsPoint::SetSigmaYZ(const string& str)
{
	DoubleFromString(m_dSigmaYZ, trimstr(str));
}

void CDnaGpsPoint::SetSigmaZZ(const string& str)
{
	// if <Coords>LLH</Coords>, then SigmaZZ is in radians^2
	// if <Coords>XYZ</Coords>, then SigmaZZ is in metres^2
	if (DoubleFromString_ZeroCheck(m_dSigmaZZ, trimstr(str)))
		throw XMLInteropException("SetSigmaZZ(): Variances cannot be zero.", 0);
}













CDnaGpsPointCluster::CDnaGpsPointCluster(void)
	: m_lRecordedTotal(0)
	, m_dPscale(1.)
	, m_dLscale(1.)
	, m_dHscale(1.)
	, m_dVscale(1.)
	, m_strCoordType("XYZ")
	, m_ctType(XYZ_type_i)
	, m_referenceFrame(DEFAULT_DATUM)
	, m_lclusterID(0)
{
	SetEpoch(DEFAULT_EPOCH);
	SetEpsg(epsgStringFromName<string>(m_referenceFrame));

	m_strType = "Y";
	m_MSmeasurementStations = ONE_STATION;	
}


CDnaGpsPointCluster::~CDnaGpsPointCluster(void)
{

}

// move constructor
CDnaGpsPointCluster::CDnaGpsPointCluster(CDnaGpsPointCluster&& p)
{
	m_strType = p.m_strType;
	m_bIgnore = p.m_bIgnore;
	m_lRecordedTotal = p.m_lRecordedTotal;
	SetCoordType(p.m_strCoordType);
	m_vGpsPoints = std::move(p.m_vGpsPoints);

	m_dPscale = p.m_dPscale;
	m_dLscale = p.m_dLscale;
	m_dHscale = p.m_dHscale;
	m_dVscale = p.m_dVscale;

	m_lclusterID = p.m_lclusterID;
	m_MSmeasurementStations = p.m_MSmeasurementStations;

	m_referenceFrame = p.m_referenceFrame;
	m_epsgCode = p.m_epsgCode;
	m_epoch = p.m_epoch;

	m_databaseIdSet = p.m_databaseIdSet;
	m_msr_db_map = p.m_msr_db_map;

	m_dbidmap = p.m_dbidmap;
}

// move assignment operator
CDnaGpsPointCluster& CDnaGpsPointCluster::operator= (CDnaGpsPointCluster&& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	CDnaMeasurement::operator=(std::move(rhs));
	m_lRecordedTotal = rhs.m_lRecordedTotal;
	SetCoordType(rhs.m_strCoordType);

	m_referenceFrame = rhs.m_referenceFrame;
	m_epsgCode = rhs.m_epsgCode;
	m_epoch = rhs.m_epoch;

	m_dPscale = rhs.m_dPscale;
	m_dLscale = rhs.m_dLscale;
	m_dHscale = rhs.m_dHscale;
	m_dVscale = rhs.m_dVscale;

	m_lclusterID = rhs.m_lclusterID;
	m_MSmeasurementStations = rhs.m_MSmeasurementStations;

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	m_vGpsPoints = std::move(rhs.m_vGpsPoints);

	m_databaseIdSet = rhs.m_databaseIdSet;
	m_msr_db_map = rhs.m_msr_db_map;

	m_dbidmap = rhs.m_dbidmap;

	return *this;
}

// copy constructor (disabled)
//CDnaGpsPointCluster::CDnaGpsPointCluster(const CDnaGpsPointCluster& newGpsPointCluster)
//{
//	m_strType = newGpsPointCluster.m_strType;
//	m_bIgnore = newGpsPointCluster.m_bIgnore;
//	m_lRecordedTotal = newGpsPointCluster.m_lRecordedTotal;
//	SetCoordType(newGpsPointCluster.m_strCoordType);
//	m_vGpsPoints = newGpsPointCluster.m_vGpsPoints;
//
//	m_dPscale = newGpsPointCluster.m_dPscale;
//	m_dLscale = newGpsPointCluster.m_dLscale;
//	m_dHscale = newGpsPointCluster.m_dHscale;
//	m_dVscale = newGpsPointCluster.m_dVscale;
//
//	m_lclusterID = newGpsPointCluster.m_lclusterID;
//	m_MSmeasurementStations = newGpsPointCluster.m_MSmeasurementStations;
//
//	m_referenceFrame = newGpsPointCluster.m_referenceFrame;
//	m_epsgCode = newGpsPointCluster.m_epsgCode;
//	m_epoch = newGpsPointCluster.m_epoch;
//
//	m_databaseIdSet = newGpsPointCluster.m_databaseIdSet;
//	m_msr_db_map = newGpsPointCluster.m_msr_db_map;
//}


//CDnaGpsPointCluster::CDnaGpsPointCluster(const bool bIgnore, const string& strType, const string& strFirstStation)
//{
//	m_strType = strType;
//	m_bIgnore = bIgnore;
//}

CDnaGpsPointCluster::CDnaGpsPointCluster(const UINT32 lclusterID, const string& referenceframe, const string& epoch)
	: m_lRecordedTotal(0)
	, m_dPscale(1.)
	, m_dLscale(1.)
	, m_dHscale(1.)
	, m_dVscale(1.)
	, m_strCoordType("XYZ")
	, m_ctType(XYZ_type_i)
	, m_referenceFrame(referenceframe)
	, m_lclusterID(lclusterID)
{
	SetEpoch(epoch);
	SetEpsg(epsgStringFromName<string>(referenceframe));

	m_strType = "Y";
	m_MSmeasurementStations = ONE_STATION;	
}
	

// assignment operator (disabled)
//CDnaGpsPointCluster& CDnaGpsPointCluster::operator= (const CDnaGpsPointCluster& rhs)
//{
//	// check for assignment to self!
//	if (this == &rhs)
//		return *this;
//
//	CDnaMeasurement::operator=(rhs);
//	m_lRecordedTotal = rhs.m_lRecordedTotal;
//	SetCoordType(rhs.m_strCoordType);
//
//	m_referenceFrame = rhs.m_referenceFrame;
//	m_epsgCode = rhs.m_epsgCode;
//	m_epoch = rhs.m_epoch;
//
//	m_dPscale = rhs.m_dPscale;
//	m_dLscale = rhs.m_dLscale;
//	m_dHscale = rhs.m_dHscale;
//	m_dVscale = rhs.m_dVscale;
//
//	m_lclusterID = rhs.m_lclusterID;
//	m_MSmeasurementStations = rhs.m_MSmeasurementStations;
//
//	m_databaseIdSet = rhs.m_databaseIdSet;
//	m_msr_db_map = rhs.m_msr_db_map;
//
//	m_vGpsPoints = rhs.m_vGpsPoints;
//
//	return *this;
//}


bool CDnaGpsPointCluster::operator== (const CDnaGpsPointCluster& rhs) const
{
	return (
		m_strType == rhs.m_strType &&
		m_strFirst == rhs.m_strFirst &&
		m_bIgnore == rhs.m_bIgnore &&
		m_lRecordedTotal == rhs.m_lRecordedTotal &&
		m_strCoordType == rhs.m_strCoordType &&
		m_dPscale == rhs.m_dPscale &&
		m_dLscale == rhs.m_dLscale &&
		m_dHscale == rhs.m_dHscale &&
		m_dVscale == rhs.m_dVscale &&
		m_vGpsPoints == rhs.m_vGpsPoints
		);
}
	
bool CDnaGpsPointCluster::operator< (const CDnaGpsPointCluster& rhs) const
{
	if (m_strFirst == rhs.m_strFirst) {
		if (m_strType == rhs.m_strType) {
			if (m_bIgnore == rhs.m_bIgnore) {
				if (m_epoch == rhs.m_epoch) {
					if (m_strCoordType == rhs.m_strCoordType) {
						if (m_lRecordedTotal == rhs.m_lRecordedTotal) 
							return m_vGpsPoints < rhs.m_vGpsPoints;
						else
							return m_lRecordedTotal < rhs.m_lRecordedTotal; }
					else
						return m_strCoordType < rhs.m_strCoordType; }
				else
					return m_epoch < rhs.m_epoch; }
			else
				return m_bIgnore < rhs.m_bIgnore; }
		else
			return m_strType < rhs.m_strType; }
	else
		return m_strFirst < rhs.m_strFirst;
}

void CDnaGpsPointCluster::SetCoordType(const string& sType) {
	m_strCoordType = trimstr(sType);
	m_ctType = GetMyCoordTypeC();
}
	

_COORD_TYPE_ CDnaGpsPointCluster::GetMyCoordTypeC()
{
	return CDnaStation::GetCoordTypeC(m_strCoordType);
	//// case insensitive
	//if (iequals(m_strCoordType, XYZ_type))
	//	return XYZ_type_i;
	//if (iequals(m_strCoordType, UTM_type))
	//	return UTM_type_i;
	//return LLH_type_i;	// default
}
	

void CDnaGpsPointCluster::AddGpsPoint(const CDnaMeasurement* pGpsPoint)
{
	//CDnaGpsPoint pnt = (CDnaGpsPoint&)*pGpsPoint;
	//m_vGpsPoints.push_back(pnt);
	m_vGpsPoints.push_back(std::move((CDnaGpsPoint&)*pGpsPoint));
}


//void CDnaGpsPointCluster::ClearPoints()
//{
//	m_vGpsPoints.clear();
//}
	

// void CDnaGpsPointCluster::SetDatabaseMap_bmsIndex(const UINT32& bmsIndex) 
// { 
// 	UINT32 i(bmsIndex);
// 	for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
// 		[this, &i](const CDnaGpsPoint& pnt) {
// 			((CDnaMeasurement*)&pnt)->SetDatabaseMap_bmsIndex(i++);
// 	});
// }
	

void CDnaGpsPointCluster::SerialiseDatabaseMap(std::ofstream* os)
{
	for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
		[this, os](const CDnaGpsPoint& pnt) {
			((CDnaGpsPoint*)&pnt)->SerialiseDatabaseMap(os);
	});
}
	

void CDnaGpsPointCluster::SetDatabaseMaps(it_vdbid_t& dbidmap, bool dbidSet)
{
	m_databaseIdSet = dbidSet;

	if (dbidSet)
	{
		m_dbidmap = dbidmap;

		CDnaMeasurement::SetDatabaseMap(*m_dbidmap, m_databaseIdSet);

		for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
			[this](const CDnaGpsPoint& bsl) {
				((CDnaGpsPoint*)&bsl)->SetDatabaseMap(*m_dbidmap, m_databaseIdSet);
		m_dbidmap += ((CDnaGpsPoint*)&bsl)->GetCovariances_ptr()->size();
			});
	}
}


UINT32 CDnaGpsPointCluster::CalcBinaryRecordCount() const
{
	UINT32 recordCount(0);
	for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
		[&recordCount](const CDnaGpsPoint& pnt) {
			recordCount += pnt.CalcBinaryRecordCount();
	});
	return recordCount;
}
	

void CDnaGpsPointCluster::WriteDynaMLMsr(std::ofstream* dynaml_stream, const string& comment, bool) const
{
	const size_t pntCount = m_vGpsPoints.size();

	if (comment.empty())
	{
		*dynaml_stream << "  <!-- Type " << measurement_name<char, string>(GetTypeC());
		if (pntCount > 1)
			*dynaml_stream << " (set of " << pntCount << ")";
		else
			*dynaml_stream << "  (single)";
		*dynaml_stream << " -->" << endl;
	}
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
	
	// Reference frame and epoch
	*dynaml_stream << "    <ReferenceFrame>" << m_referenceFrame << "</ReferenceFrame>" << endl;
	*dynaml_stream << "    <Epoch>" << m_epoch << "</Epoch>" << endl;

	// Scalars
	*dynaml_stream << "    <Vscale>" << fixed << setprecision(3) << m_dVscale << "</Vscale>" << endl;
	*dynaml_stream << "    <Pscale>" << m_dPscale << "</Pscale>" << endl;
	*dynaml_stream << "    <Lscale>" << m_dLscale << "</Lscale>" << endl;
	*dynaml_stream << "    <Hscale>" << m_dHscale << "</Hscale>" << endl;
	
	if (m_databaseIdSet)
		if (m_msr_db_map.is_cls_id_set)
			*dynaml_stream << "    <ClusterID>" << m_msr_db_map.cluster_id << "</ClusterID>" << endl;
	
	*dynaml_stream << "    <Coords>" << m_strCoordType << "</Coords>" << endl;
	*dynaml_stream << "    <Total>" << pntCount << "</Total>" << endl;
	
	// write GpsPoints
	vector<CDnaGpsPoint>::const_iterator _it_pnt;
	for (_it_pnt=m_vGpsPoints.begin(); _it_pnt!=m_vGpsPoints.end(); ++_it_pnt)
		_it_pnt->WriteDynaMLMsr(dynaml_stream, comment, true);
	
	*dynaml_stream << "  </DnaMeasurement>" << endl;
}

void CDnaGpsPointCluster::WriteDNAMsr(std::ofstream* dna_stream, const dna_msr_fields& dmw, 
		const dna_msr_fields& dml, bool) const
{
	// write GpsPoints
	vector<CDnaGpsPoint>::const_iterator _it_pnt;
	for (_it_pnt=m_vGpsPoints.begin(); _it_pnt!=m_vGpsPoints.end(); ++_it_pnt)
		_it_pnt->WriteDNAMsr(dna_stream, dmw, dml, true);
}

void CDnaGpsPointCluster::SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid)
{
	vector<CDnaGpsPoint>::iterator _it_pnt = m_vGpsPoints.begin();
	for (_it_pnt=m_vGpsPoints.begin(); _it_pnt!=m_vGpsPoints.end(); ++_it_pnt)
		_it_pnt->SimulateMsr(vStations, ellipsoid);
}

void CDnaGpsPointCluster::PopulateMsr(pvstn_t bstRecords, uint32_uint32_map* blockStationsMap, vUINT32* blockStations,
		const UINT32& block, const CDnaDatum* datum, matrix_2d* estimates, matrix_2d* variances)
{
	m_strType = 'Y';
	m_bIgnore = false;
	SetCoordType(XYZ_type);

	m_lclusterID = block;
	m_MSmeasurementStations = ONE_STATION;
	m_lRecordedTotal = static_cast<UINT32>(blockStationsMap->size());
	m_vGpsPoints.clear();
	m_vGpsPoints.resize(m_lRecordedTotal);

	m_referenceFrame = datum->GetName();
	m_epoch = datum->GetEpoch_s();
	m_epsgCode =  datum->GetEpsgCode_s();

	m_dPscale = 1.0;
	m_dLscale = 1.0;
	m_dHscale = 1.0;
	m_dVscale = 1.0;

	vector<CDnaGpsPoint>::iterator _it_pnt = m_vGpsPoints.begin();
	for (UINT32 i(0); i<m_lRecordedTotal; ++i)
	{
		_it_pnt->SetClusterID(block);
		_it_pnt->SetTotal(static_cast<UINT32>(blockStations->size()));
		_it_pnt->PopulateMsr(bstRecords, blockStationsMap, blockStations,
			i, datum, estimates, variances);
		++_it_pnt;
	}
}
	


//UINT32 CDnaGpsPointCluster::SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord)
//{
//	m_lclusterID = measRecord->clusterID;
//	m_MSmeasurementStations = (MEASUREMENT_STATIONS)measRecord->measurementStations;
//	m_lRecordedTotal = measRecord->vectorCount1;
//	m_vGpsPoints.clear();
//	m_vGpsPoints.resize(m_lRecordedTotal);
//
//	m_referenceFrame = datumFromEpsgCode<string, UINT32>(LongFromString<UINT32>(measRecord->epsgCode));
//	m_epoch = measRecord->epoch;
//	m_epsgCode = measRecord->epsgCode;
//
//	m_dPscale = measRecord->scale1;
//	m_dLscale = measRecord->scale2;
//	m_dHscale = measRecord->scale3;
//	m_dVscale = measRecord->scale4;
//
//	UINT32 measrecordCount = 0;
//
//	// read remaining GpsPoint data and all Covariances from file
//	for (UINT32 i=0; i<m_lRecordedTotal; i++)
//	{
//		if (i > 0)
//		{
//			// get data relating to Y, sigmaYY, sigmaXY
//			if (ifs_msrs->eof() || !ifs_msrs->good())
//				throw XMLInteropException("SetMeasurementRec(): Errors were encountered when reading from the binary measurement file.", 0);
//			ifs_msrs->read(reinterpret_cast<char *>(measRecord), sizeof(measurement_t));
//			
//			// check integrity of the binary file (see WriteBinaryMsr())
//			if (measRecord->measType != 'Y')
//				throw XMLInteropException("SetMeasurementRec(): Errors were encountered when attempting to read the next GpsPoint element (X block).", 0);
//		}
//
//		measrecordCount++;
//
//		m_strType = measRecord->measType;
//		m_bIgnore = measRecord->ignore;
//		SetCoordType(measRecord->coordType);
//	
//		measrecordCount += m_vGpsPoints.at(i).SetMeasurementRec(ifs_stns, ifs_msrs, measRecord);
//	}
//
//	return measrecordCount - 1;	
//}
	

UINT32 CDnaGpsPointCluster::SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr, it_vdbid_t& dbidmap, bool dbidSet)
{
	m_lclusterID = it_msr->clusterID;
	m_MSmeasurementStations = (MEASUREMENT_STATIONS)it_msr->measurementStations;
	m_lRecordedTotal = it_msr->vectorCount1;
	m_vGpsPoints.clear();
	m_vGpsPoints.resize(m_lRecordedTotal);

	m_referenceFrame = datumFromEpsgCode<string, UINT32>(LongFromString<UINT32>(it_msr->epsgCode));
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
		{
			it_msr++;
			if (dbidSet)
				dbidmap++;
		}

		m_strType = it_msr->measType;
		m_bIgnore = it_msr->ignore;
		SetCoordType(it_msr->coordType);
		
		m_vGpsPoints.at(i).SetMeasurementRec(binaryStn, it_msr, dbidmap, dbidSet);
	}

	return measrecordCount - 1;	
}
	

void CDnaGpsPointCluster::WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const
{
	vector< CDnaGpsPoint >::const_iterator _it_pnt;
	for (_it_pnt=m_vGpsPoints.begin(); _it_pnt!=m_vGpsPoints.end(); ++_it_pnt)
		_it_pnt->WriteBinaryMsr(binary_stream, msrIndex);

	//for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
	//	[this, &binary_stream, &msrIndex] (CDnaGpsPoint p) {
	//		p.WriteBinaryMsr(binary_stream, msrIndex); 
	//});
}


// moved to dnameasurement.cpp
//void CDnaGpsPointCluster::SetEpoch(const string& epoch) 
//{
//	//for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
//	//	[this, &epoch] (CDnaGpsPoint &p) {
//	//		p.SetEpoch(epoch); 
//	//});
//	m_epoch = epoch;
//}


void CDnaGpsPointCluster::SetEpsg(const string& epsg) 
{
	//for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
	//	[this, &epsg] (CDnaGpsPoint &p) {
	//		p.SetEpsg(epsg); 
	//});
	m_epsgCode = epsg;
}


void CDnaGpsPointCluster::SetReferenceFrame(const string& refFrame) 
{
	//for_each(m_vGpsPoints.begin(), m_vGpsPoints.end(),
	//	[this, &refFrame] (CDnaGpsPoint &p) {
	//		p.SetReferenceFrame(refFrame); 
	//});
	m_referenceFrame = refFrame;
	SetEpsg(epsgStringFromName<string>(refFrame));
}


void CDnaGpsPointCluster::SetTotal(const string& str)
{
	m_lRecordedTotal = LongFromString<UINT32>(trimstr(str));
	m_vGpsPoints.reserve(m_lRecordedTotal);
}


void CDnaGpsPointCluster::SetPscale(const string& str)
{
	DoubleFromString(m_dPscale, trimstr(str));
}

void CDnaGpsPointCluster::SetLscale(const string& str)
{
	DoubleFromString(m_dLscale, trimstr(str));
}

void CDnaGpsPointCluster::SetHscale(const string& str)
{
	DoubleFromString(m_dHscale, trimstr(str));
}

void CDnaGpsPointCluster::SetVscale(const string& str)
{
	DoubleFromString(m_dVscale, trimstr(str));
}


}	// namespace measurements
}	// namespace dynadjust
