//============================================================================
// Name         : dnaellipsoid.hpp
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
// Description  : DynAdjust Ellipsoid Library
//============================================================================

#ifndef DNAELLIPSOID_H_
#define DNAELLIPSOID_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <math.h>
#include <string>
#include <sstream>

#include <include/config/dnatypes.hpp>
#include <include/parameters/dnadatumprojectionparam.hpp>

namespace dynadjust {
namespace datum_parameters {

class CDnaEllipsoid
{
public:

	// predefined ellipsoid
	CDnaEllipsoid(void);
	CDnaEllipsoid(const UINT32& epsgCode);
	CDnaEllipsoid(const std::string& epsgCode);
	CDnaEllipsoid(const CDnaEllipsoid& newEllipsoid);
	
	// user-defined ellipsoid
	CDnaEllipsoid(const double& a, const double& inv_f);
	
	virtual inline ~CDnaEllipsoid(void) {}
//	virtual inline CDnaEllipsoid* clone() const { 
//		return new CDnaEllipsoid(*this); 
//	}

	CDnaEllipsoid& operator=(const CDnaEllipsoid& rhs);
	bool operator==(const CDnaEllipsoid& rhs) const;

	//inline CDnaEllipsoid& operator[](int iIndex) { return this[iIndex]; }

	inline double GetE1() const { return m_dEccen1; }
	inline double GetE2() const { return m_dEccen2; }
	inline double GetE1sqd() const { return m_dEccen1_sqd; }
	inline double GetE2sqd() const { return m_dEccen2_sqd; }
	inline double GetSemiMajor() const { return m_dA; }
	inline double GetSemiMinor() const { return m_dB; }
	inline double GetInverseFlattening() const { return m_dInv_f; }
	inline double GetFlattening() const { return m_dF; }
	inline void SetSemiMajor(const double& a) { m_dA = a; }
	inline void SetInverseFlattening(const double& inv_f) { m_dInv_f = inv_f; }
	
	void SetEllipsoid(const UINT32& epsgCode);
	
private:
	void SetEllipsoidParams(const double& a, const double& inv_f);
	void CalculateEllipsoidParams();
	
	double m_dA;
	double m_dB;
	double m_dInv_f;
	double m_dF;
	double m_dEccen1;
	double m_dEccen1_sqd;
	double m_dEccen2;
	double m_dEccen2_sqd;
	
	// The intended behaviour of this class is that an ellipsoid cannot be
	// changed after construction.
	UINT32 m_epsgCode;
	ELLIPSOID_TYPE m_ellipsoidType;
};

}	// namespace datum_parameters
}	// namespace dynadjust

#endif /* DNADATUMPROJECTION_H_ */
