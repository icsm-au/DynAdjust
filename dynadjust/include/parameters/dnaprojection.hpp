//============================================================================
// Name         : dnaprojection.hpp
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
// Description  : DynAdjust Projection Library
//============================================================================

#ifndef DNAPROJECTION_H_
#define DNAPROJECTION_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <math.h>
#include <string>
#include <sstream>

using namespace std;

#include <include/config/dnatypes.hpp>
#include <include/parameters/dnadatumprojectionparam.hpp>
	
namespace dynadjust {
namespace datum_parameters {

class CDnaProjection
{
public:

	CDnaProjection(void);
	CDnaProjection(const PROJECTION_NAME& pName);

	virtual inline ~CDnaProjection(void) {}
//	virtual inline CDnaProjection* clone() const { 
//		return new CDnaProjection(*this); 
//	}

private:
	// Disallow use of compiler generated functions. 
	CDnaProjection(const CDnaProjection& newProjection);
	CDnaProjection& operator=(const CDnaProjection& rhs);
	bool operator==(const CDnaProjection& rhs) const;
	
	//inline CDnaProjection& operator[](int iIndex) { return this[iIndex]; }

public:

	void SetProjection(const PROJECTION_NAME& pName);
	
	inline double GetFalseEasting() const { return m_dFalseE; }
	inline double GetFalseNorthing() const { return m_dFalseN; }
	inline double GetCentralScaleFactor() const { return m_dKo; }
	inline double GetZoneWidth() const { return m_dZw; }
	inline double GetLongCentralMeridianZone1() const { return m_dLcmZ1; }
	inline double GetLongCentralMeridianZone0() const { return m_dLcmZ0; }
	inline double GetLongWesternEdgeZone0() const { return m_dLweZ0; }
	
private:
	//void SetProjectionParams(const double& dFalseE, const double& dFalseN, const double& dKo,	
	//	const double& dZw, const double& dLcmZ1, const double& dLweZ0, const double& dLcmZ0);

	double m_dFalseE;				// False Easting
	double m_dFalseN;				// False Northing
	double m_dKo;					// Central scale factor
	double m_dZw;					// Zone width
	double m_dLcmZ1;				// Longitude of central meridian of zone 1
	double m_dLweZ0;				// Longitude of western edge of zone 0
	double m_dLcmZ0;				// Longitude of central meridian of zone 0

	PROJECTION_NAME projectionName;
};

}	// namespace datum_parameters
}	// namespace dynadjust

#endif /* DNADATUMPROJECTION_H_ */
