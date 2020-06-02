//============================================================================
// Name         : dnagpspoint.hpp
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
// Description  : Interface for the CDnaGpsPoint and CDnaGpsPointCluster classes
//============================================================================

#ifndef DNAGPSPOINT_H_
#define DNAGPSPOINT_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnameasurement.hpp>
#include <include/config/dnatypes.hpp>

using namespace std;

namespace dynadjust {
namespace measurements {

class CDnaGpsPoint : public CDnaMeasurement
{
public:
	CDnaGpsPoint(void);
	CDnaGpsPoint(const CDnaGpsPoint&);
	virtual ~CDnaGpsPoint(void);

	CDnaGpsPoint(const bool bIgnore, const string& strType, const string& strFirstStation);

	virtual inline CDnaGpsPoint* clone() const { return new CDnaGpsPoint(*this); }
	CDnaGpsPoint& operator=(const CDnaGpsPoint& rhs);
	bool operator==(const CDnaGpsPoint& rhs) const;
	virtual bool operator<(const CDnaGpsPoint& rhs) const;

	inline CDnaGpsPoint& operator[](int iIndex) { return this[iIndex]; }

	void AddPointCovariance(const CDnaCovariance* pGpsCovariance);

	inline UINT32 GetClusterID() const { return m_lclusterID; }
	inline string GetCoordType() const { return m_strCoordType; }
	inline string GetTarget() const { return m_strTarget; }

	inline vector<CDnaCovariance>* GetCovariances_ptr() { return &m_vPointCovariances; }

	inline void SetClusterID(const UINT32& id) { m_lclusterID = id; }
	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	void SetX(const string& str);
	void SetY(const string& str);
	void SetZ(const string& str);
	void SetSigmaXX(const string& str);
	void SetSigmaXY(const string& str);
	void SetSigmaXZ(const string& str);
	void SetSigmaYY(const string& str);
	void SetSigmaYZ(const string& str);
	void SetSigmaZZ(const string& str);
	
	void SetReferenceFrame(const string& refFrame);
	void SetEpoch(const string& epoch);
	void SetEpsg(const string& epsg);	
	inline string GetReferenceFrame() const { return m_referenceFrame; }
	inline string GetEpoch() const { return m_epoch; }
		
	void SetPscale(const string& str);
	void SetLscale(const string& str);
	void SetHscale(const string& str);
	void SetVscale(const string& str);

	inline void SetTotal(const UINT32& l) { m_lRecordedTotal = l; }

	inline void SetSigmaXX(const double& dbl) { m_dSigmaXX = dbl; }
	inline void SetSigmaXY(const double& dbl) { m_dSigmaXY = dbl; }
	inline void SetSigmaXZ(const double& dbl) { m_dSigmaXZ = dbl; }
	inline void SetSigmaYY(const double& dbl) { m_dSigmaYY = dbl; }
	inline void SetSigmaYZ(const double& dbl) { m_dSigmaYZ = dbl; }
	inline void SetSigmaZZ(const double& dbl) { m_dSigmaZZ = dbl; }

	inline void SetXAxis(const double& dbl) { m_dX = dbl; }
	inline void SetYAxis(const double& dbl) { m_dY = dbl; }
	inline void SetZAxis(const double& dbl) { m_dZ = dbl; }
	
	inline void SetPscale(const double& dbl) { m_dPscale = dbl; }
	inline void SetLscale(const double& dbl) { m_dLscale = dbl; }
	inline void SetHscale(const double& dbl) { m_dHscale = dbl; }
	inline void SetVscale(const double& dbl) { m_dVscale = dbl; }
	void SetCoordType(const string& str);

	inline void SetRecordedTotal(const UINT32& total) { m_lRecordedTotal = total; }

	_COORD_TYPE_ GetMyCoordTypeC();

	void ReserveGpsCovariancesCount(const UINT32& size);
	void ResizeGpsCovariancesCount(const UINT32& size = 0);

	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const;
	virtual UINT32 CalcBinaryRecordCount() const;
	void coutPointData(ostream &os) const;
	virtual void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord);
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr);
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement = false) const;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement = false) const;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid);

	virtual void SerialiseDatabaseMap(std::ofstream* os);

	string m_strTarget;

	inline double GetVscale() const { return m_dVscale; }
	inline double GetPscale() const { return m_dPscale; }
	inline double GetLscale() const { return m_dLscale; }
	inline double GetHscale() const { return m_dHscale; }

	inline double GetValue() const { return sqrt(m_dX*m_dX + m_dY*m_dY + m_dZ*m_dZ); }			// Virtual magnitude
	inline double GetStdDev() const { return sqrt(m_dSigmaXX + m_dSigmaYY + m_dSigmaZZ); }		// RMS

	//virtual inline void SetDatabaseMap_bmsIndex(const UINT32& bmsIndex) { m_msr_db_map.bms_index = bmsIndex; }
protected:

	UINT32 m_lRecordedTotal;

	double m_dX;
	double m_dY;
	double m_dZ;
	double m_dSigmaXX;
	double m_dSigmaXY;
	double m_dSigmaXZ;
	double m_dSigmaYY;
	double m_dSigmaYZ;
	double m_dSigmaZZ;

	double m_dPscale;
	double m_dLscale;
	double m_dHscale;
	double m_dVscale;
	string m_strCoordType;

	COORD_TYPE m_ctType;

	string	m_referenceFrame;
	string	m_epoch;

	UINT32 m_lclusterID;

	vector<CDnaCovariance> m_vPointCovariances;
};


class CDnaGpsPointCluster : public CDnaMeasurement
{
public:
	CDnaGpsPointCluster(void);
	CDnaGpsPointCluster(const CDnaGpsPointCluster&);
	virtual ~CDnaGpsPointCluster(void);

	CDnaGpsPointCluster(const bool bIgnore, const string& strType, const string& strFirstStation);

	CDnaGpsPointCluster(const UINT32 lclusterID, const string& referenceframe, const string& epoch);

	virtual inline CDnaGpsPointCluster* clone() const { return new CDnaGpsPointCluster(*this); }
	CDnaGpsPointCluster& operator=(const CDnaGpsPointCluster& rhs);
	bool operator==(const CDnaGpsPointCluster& rhs) const;
	virtual bool operator<(const CDnaGpsPointCluster& rhs) const;

	inline CDnaGpsPointCluster& operator[](int iIndex) { return this[iIndex]; }

	//inline UINT32 GetNumPoints() { return m_vGpsPoints.size(); }
	inline vector<CDnaGpsPoint>& GetPoints() { return m_vGpsPoints; }
	inline vector<CDnaGpsPoint>* GetPoints_ptr() { return &m_vGpsPoints; }

	inline UINT32 GetClusterID() const { return m_lclusterID; }
	inline string GetCoordType() const { return m_strCoordType; }
	inline UINT32 GetTotal() const { return m_lRecordedTotal; }
	inline string GetTarget() const { return m_strTarget; }
	inline double GetPscale() const { return m_dPscale; }
	inline double GetLscale() const { return m_dLscale; }
	inline double GetHscale() const { return m_dHscale; }
	inline double GetVscale() const { return m_dVscale; }

	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	void SetCoordType(const string& str);

	_COORD_TYPE_ GetMyCoordTypeC();

	void SetReferenceFrame(const string& refFrame);
	void SetEpoch(const string& epoch);
	void SetEpsg(const string& epsg);	
	inline string GetReferenceFrame() const { return m_referenceFrame; }
	inline string GetEpoch() const { return m_epoch; }
	
	inline void SetPoints(const vector<CDnaGpsPoint>& d) { m_vGpsPoints = d; }
	void SetTotal(const string& str);
	void SetPscale(const string& str);
	void SetLscale(const string& str);
	void SetHscale(const string& str);
	void SetVscale(const string& str);

	inline void SetTotal(const UINT32& u) { m_lRecordedTotal = u; }

	inline void SetPscale(const double& dbl) { m_dPscale = dbl; }
	inline void SetLscale(const double& dbl) { m_dLscale = dbl; }
	inline void SetHscale(const double& dbl) { m_dHscale = dbl; }
	inline void SetVscale(const double& dbl) { m_dVscale = dbl; }

	void AddGpsPoint(const CDnaMeasurement* pGpsPoint);
	void ClearPoints();

	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const;
	virtual UINT32 CalcBinaryRecordCount() const;
	//virtual UINT32 CalcDbidRecordCount() const;
	virtual void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord);
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr);
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement = false) const;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement = false) const;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid);

	virtual void SerialiseDatabaseMap(std::ofstream* os);
	//virtual void SetDatabaseMap_bmsIndex(const UINT32& bmsIndex);

protected:

	string m_strTarget;
	UINT32 m_lRecordedTotal;
	double m_dPscale;
	double m_dLscale;
	double m_dHscale;
	double m_dVscale;
	string m_strCoordType;

	COORD_TYPE m_ctType;

	vector<CDnaGpsPoint> m_vGpsPoints;

	string	m_referenceFrame;
	string	m_epoch;

	UINT32 m_lclusterID;
};

}	// namespace measurements
}	// namespace dynadjust

#endif /* DNAGPSPOINT_H_ */
