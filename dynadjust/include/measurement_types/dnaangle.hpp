//============================================================================
// Name         : dnaangle.hpp
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
// Description  : Interface for the CDnaAngle class
//============================================================================

#ifndef DNAANGLE_H_
#define DNAANGLE_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnameasurement.hpp>
#include <include/functions/dnatemplatefuncs.hpp>

using namespace std;

namespace dynadjust {
namespace measurements {

class CDnaAngle : public CDnaMeasurement
{
public:
	CDnaAngle(void);
	CDnaAngle(const CDnaAngle&);
	virtual ~CDnaAngle(void);

	CDnaAngle(const bool strIgnore, const string& strFirst, const string& strTarget, const string& strTarget2, const double& drValue, const double& dStdDev);

	virtual inline CDnaAngle* clone() const { return new CDnaAngle(*this); }
	CDnaAngle& operator=(const CDnaAngle& rhs);
	bool operator==(const CDnaAngle& rhs) const;
	virtual bool operator<(const CDnaAngle& rhs) const;

	inline CDnaAngle& operator[](int iIndex) { return this[iIndex]; }

	inline string GetTarget() const { return m_strTarget; }
	inline string GetTarget2() const { return m_strTarget2; }
	inline double GetValue() const { return m_drValue; }
	inline double GetStdDev() const { return m_dStdDev; }
	
	inline void SetTarget(const string& str) { m_strTarget = trimstr(str); }
	inline void SetTarget2(const string& str) { m_strTarget2 = trimstr(str); }

	void SetValue(const string& str);
	void SetStdDev(const string& str);
	
	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const;
	inline virtual UINT32 CalcBinaryRecordCount() const { return 1; }
	virtual void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord);
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr);
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement = false) const;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement = false) const;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid);

protected:
	string	m_strTarget;
	string	m_strTarget2;
	double	m_drValue;
	double	m_dStdDev;
};

}	// namespace measurements
}	// namespace dynadjust

#endif /* DNAANGLE_H_ */
