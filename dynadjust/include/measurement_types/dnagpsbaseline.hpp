//============================================================================
// Name         : dnagpsbaseline.hpp
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
// Description  : Interface for the CDnaGpsBaseline and CDnaGpsBaselineCluster 
//                classes
//============================================================================

#ifndef DNAGPSBASELINE_H_
#define DNAGPSBASELINE_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnameasurement.hpp>

using namespace std;

namespace dynadjust {
namespace measurements {

class CDnaGpsBaseline : public CDnaMeasurement
{
public:
	CDnaGpsBaseline(void);
	CDnaGpsBaseline(const CDnaGpsBaseline&);
	virtual ~CDnaGpsBaseline(void);

	CDnaGpsBaseline(const bool bIgnore, const string& strType, const string& strFirstStation, const string& strSecondStation);

	virtual inline CDnaGpsBaseline* clone() const { return new CDnaGpsBaseline(*this); }
	CDnaGpsBaseline& operator=(const CDnaGpsBaseline& rhs);
	bool operator==(const CDnaGpsBaseline& rhs) const;
	virtual bool operator<(const CDnaGpsBaseline& rhs) const;

	inline CDnaGpsBaseline& operator[](int iIndex) { return this[iIndex]; }

	inline UINT32 GetClusterID() const { return m_lclusterID; }
	inline string GetTarget() const { return m_strTarget; }
	inline vector<CDnaCovariance>* GetCovariances_ptr() { return &m_vGpsCovariances; }

	void AddGpsCovariance(const CDnaCovariance* pGpsCovariance);

	void ReserveGpsCovariancesCount(const UINT32& size);
	void ResizeGpsCovariancesCount(const UINT32& size = 0);

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

	inline void SetPscale(const double& dbl) { m_dPscale = dbl; }
	inline void SetLscale(const double& dbl) { m_dLscale = dbl; }
	inline void SetHscale(const double& dbl) { m_dHscale = dbl; }
	inline void SetVscale(const double& dbl) { m_dVscale = dbl; }

	inline void SetRecordedTotal(const UINT32& total) { m_lRecordedTotal = total; }
	inline void SetTotal(const UINT32& l) { m_lRecordedTotal = l; }

	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const;
	virtual UINT32 CalcBinaryRecordCount() const;
	void coutBaselineData(ostream &os, const int& pad, const UINT16& uType = 0) const;
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

	inline double GetValue() const { return sqrt(m_dX*m_dX + m_dY*m_dY + m_dZ*m_dZ); }			// Magnitude
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

	string	m_referenceFrame;
	string	m_epoch;

	UINT32 m_lclusterID;

	vector<CDnaCovariance> m_vGpsCovariances;
};

// used for Type G or X
class CDnaGpsBaselineCluster : public CDnaMeasurement
{
public:
	CDnaGpsBaselineCluster(void);
	CDnaGpsBaselineCluster(const CDnaGpsBaselineCluster&);
	virtual ~CDnaGpsBaselineCluster(void);

	CDnaGpsBaselineCluster(const bool bIgnore, const string& strType, const string& strFirstStation);
	CDnaGpsBaselineCluster(const UINT32 lclusterID, const string& referenceframe, const string& epoch);

	virtual inline CDnaGpsBaselineCluster* clone() const { return new CDnaGpsBaselineCluster(*this); }
	CDnaGpsBaselineCluster& operator=(const CDnaGpsBaselineCluster& rhs);
	bool operator==(const CDnaGpsBaselineCluster& rhs) const;
	virtual bool operator<(const CDnaGpsBaselineCluster& rhs) const;

	inline CDnaGpsBaselineCluster& operator[](int iIndex) { return this[iIndex]; }

	//inline UINT32 GetNumBaselinens() { return m_vGpsBaselines.size(); }
	inline vector<CDnaGpsBaseline>& GetBaselines() { return m_vGpsBaselines; }
	inline vector<CDnaGpsBaseline>* GetBaselines_ptr() { return &m_vGpsBaselines; }

	inline UINT32 GetClusterID() const { return m_lclusterID; }
	inline string GetTarget() const { return m_strTarget; }
	inline UINT32 GetTotal() const { return m_lRecordedTotal; }
	inline double GetPscale() const { return m_dPscale; }
	inline double GetLscale() const { return m_dLscale; }
	inline double GetHscale() const { return m_dHscale; }
	inline double GetVscale() const { return m_dVscale; }

	void SetReferenceFrame(const string& refFrame);
	void SetEpoch(const string& epoch);
	void SetEpsg(const string& epsg);	
	inline string GetReferenceFrame() const { return m_referenceFrame; }
	inline string GetEpoch() const { return m_epoch; }
		
	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	inline void SetTotal(const UINT32& l) { m_lRecordedTotal = l; }

	void SetTotal(const string& str);
	void SetPscale(const string& str);
	void SetLscale(const string& str);
	void SetHscale(const string& str);
	void SetVscale(const string& str);
	
	inline void SetPscale(const double& dbl) { m_dPscale = dbl; }
	inline void SetLscale(const double& dbl) { m_dLscale = dbl; }
	inline void SetHscale(const double& dbl) { m_dHscale = dbl; }
	inline void SetVscale(const double& dbl) { m_dVscale = dbl; }

	void AddGpsBaseline(const CDnaMeasurement* pGpsBaseline);

	void ClearBaselines();

	void ReserveGpsBaselinesCount(const UINT32& size);

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

	vector<CDnaGpsBaseline> m_vGpsBaselines;

	string	m_referenceFrame;
	string	m_epoch;

	UINT32 m_lclusterID;

};

}	// namespace measurements
}	// namespace dynadjust

#endif /* DNAGPSBASELINE_H_ */
