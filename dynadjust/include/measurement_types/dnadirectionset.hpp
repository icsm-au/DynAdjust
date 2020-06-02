//============================================================================
// Name         : dnadirectionset.hpp
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
// Description  : Interface for the CDnaDirectionSet class
//============================================================================

#ifndef DNADIRECTIONSET_H_
#define DNADIRECTIONSET_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnastation.hpp>
#include <include/measurement_types/dnadirection.hpp>
#include <include/measurement_types/dnameasurement.hpp>


#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

namespace dynadjust {
namespace measurements {

class CDnaDirectionSet : public CDnaMeasurement
{
public:
	CDnaDirectionSet(void);
	CDnaDirectionSet(const CDnaDirectionSet&);
	virtual ~CDnaDirectionSet(void);

	CDnaDirectionSet(const UINT32 lsetID);

	CDnaDirectionSet(bool bIgnore,
			const string& strFirst, const string& strTarget,
			const double& drValue, const double& dStdDev,
			const float& fInstrHeight, const float& fTargetHeight);

	virtual inline CDnaDirectionSet* clone() const { return new CDnaDirectionSet(*this); }
	CDnaDirectionSet& operator=(const CDnaDirectionSet& rhs);
	CDnaDirectionSet* operator=(const CDnaDirectionSet* rhs);
	bool operator==(const CDnaDirectionSet& rhs) const;
	virtual bool operator<(const CDnaDirectionSet& rhs) const;

	inline CDnaDirectionSet& operator[](int iIndex) { return this[iIndex]; }

	void LoadDirectionSet(const char* const, const int&, const string&, const string&, bool, const int&);

	inline UINT32 GetClusterID() const { return m_lsetID; }
	inline string GetTarget() const { return m_strTarget; }
	inline UINT32 GetTotal() const { return m_lRecordedTotal; }
	inline double GetValue() const { return m_drValue; }
	inline double GetStdDev() const { return m_dStdDev; }
	
	inline size_t GetNumDirections() const { return m_vTargetDirections.size(); }
	inline vector<CDnaDirection> GetDirections() const { return m_vTargetDirections; }
	inline vector<CDnaDirection>* GetDirections_ptr() { return &m_vTargetDirections; }

	inline void SetClusterID(const UINT32& id) { m_lsetID = id; }
	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	//inline void SetTotal(const UINT32& l) { m_lRecordedTotal = l; }
	inline void SetDirections(const vector<CDnaDirection>& d) { m_vTargetDirections = d; }

	void SetTotal(const string& str);
	void SetValue(const string& str);
	void SetStdDev(const string& str);
	
	void AddDirection(const CDnaMeasurement* pDirection);
	void ClearDirections();
	bool IsRepeatedDirection(string);

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

	string m_strTarget;

protected:
	double m_drValue;
	double m_dStdDev;
	UINT32 m_lRecordedTotal;
	vector<CDnaDirection> m_vTargetDirections;
	UINT32 m_lsetID;
};

}	// namespace measurements
}	// namespace dynadjust

#endif /* DNADIRECTIONSET_H_ */
