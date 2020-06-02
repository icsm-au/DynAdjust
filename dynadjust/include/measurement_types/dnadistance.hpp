//============================================================================
// Name         : dnadistance.hpp
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
// Description  : Interface for the CDnaDistance class
//============================================================================

#ifndef DNADISTANCE_H_
#define DNADISTANCE_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnameasurement.hpp>

using namespace std;

namespace dynadjust {
namespace measurements {

class CDnaDistance : public CDnaMeasurement
{
public:
	CDnaDistance(void);
	CDnaDistance(const CDnaDistance&);
	virtual ~CDnaDistance(void);

	CDnaDistance(const bool bIgnore, const string& strType, const string& strFirst, const string& strTarget, const double& dValue, const double& dStdDev);

	virtual inline CDnaDistance* clone() const { return new CDnaDistance(*this); }
	CDnaDistance& operator=(const CDnaDistance& rhs);
	bool operator==(const CDnaDistance& rhs) const;
	virtual bool operator<(const CDnaDistance& rhs) const;

	inline CDnaDistance& operator[](int iIndex) { return this[iIndex]; }

	inline string GetTarget() const { return m_strTarget; }
	inline double GetValue() const { return m_dValue; }
	inline double GetStdDev() const { return m_dStdDev; }
	inline float GetInstrHeight() const { return m_fInstHeight; }
	inline float GetTargetHeight() const { return m_fTargHeight; }


	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	void SetValue(const string& str);
	void SetStdDev(const string& str);
	void SetInstrumentHeight(const string& str);
	void SetTargetHeight(const string& str);

	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const;
	inline virtual UINT32 CalcBinaryRecordCount() const { return 1; }
	virtual void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord);
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr);
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement = false) const;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement = false) const;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid);

protected:
	string m_strTarget;
	double m_dValue;
	double	m_dStdDev;
	float	m_fInstHeight;
	float	m_fTargHeight;
};

}	// namespace measurements
}	// namespace dynadjust

#endif /* DNADISTANCE_H_ */
