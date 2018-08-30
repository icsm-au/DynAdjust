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

#include <include/parameters/dnaprojection.hpp>

namespace dynadjust {
namespace datum_parameters {

CDnaProjection::CDnaProjection(void) 
	: m_dFalseE(FALSE_E)
	, m_dFalseN(FALSE_N)		// default to UTM
	, m_dKo(K0), m_dZw(ZW), m_dLcmZ1(LCMZ1), m_dLweZ0(LWEZ0), m_dLcmZ0(LCMZ0)
	, projectionName(UTM)
{

}

CDnaProjection::CDnaProjection(const PROJECTION_NAME& pName) 
{
	SetProjection(pName);
}

CDnaProjection::CDnaProjection(const CDnaProjection& newProjection) 
{
	m_dFalseE = newProjection.m_dFalseE;
	m_dFalseN = newProjection.m_dFalseN;
	m_dKo = newProjection.m_dKo;	
	m_dZw = newProjection.m_dZw;	
	m_dLcmZ1 = newProjection.m_dLcmZ1;
	m_dLweZ0 = newProjection.m_dLweZ0;
	m_dLcmZ0 = newProjection.m_dLcmZ0;
	projectionName = newProjection.projectionName;
}

CDnaProjection& CDnaProjection::operator=(const CDnaProjection& rhs) 
{
	if (this == &rhs)	// check for assignment to self!
		return *this;
	m_dFalseE = rhs.m_dFalseE;
	m_dFalseN = rhs.m_dFalseN;
	m_dKo = rhs.m_dKo;	
	m_dZw = rhs.m_dZw;	
	m_dLcmZ1 = rhs.m_dLcmZ1;
	m_dLweZ0 = rhs.m_dLweZ0;
	m_dLcmZ0 = rhs.m_dLcmZ0;
	projectionName = rhs.projectionName;

	return *this;
}

bool CDnaProjection::operator==(const CDnaProjection& rhs) const
{
	return (
		m_dFalseE == rhs.m_dFalseE &&
		m_dFalseN == rhs.m_dFalseN &&
		m_dKo == rhs.m_dKo &&
		m_dZw == rhs.m_dZw &&
		m_dLcmZ1 == rhs.m_dLcmZ1 &&
		m_dLweZ0 == rhs.m_dLweZ0 &&
		m_dLcmZ0 == rhs.m_dLcmZ0 &&
		projectionName == rhs.projectionName
		);
}

void CDnaProjection::SetProjection(const PROJECTION_NAME& pName) 
{
	switch (pName)
	{
	case UTM:
	default:
		m_dFalseE = FALSE_E;
		m_dFalseN = FALSE_N;
		m_dKo = K0;	
		m_dZw = ZW;	
		m_dLcmZ1 = LCMZ1;
		m_dLweZ0 = LWEZ0;
		m_dLcmZ0 = LCMZ0;
		projectionName = pName;
		break;
	}
}

void CDnaProjection::SetProjectionParams(const double& dFalseE, const double& dFalseN, const double& dKo,	
	const double& dZw, const double& dLcmZ1, const double& dLweZ0, const double& dLcmZ0) 
{
	m_dFalseE = dFalseE;
	m_dFalseN = dFalseN;
	m_dKo = dKo;	
	m_dZw = dZw;	
	m_dLcmZ1 = dLcmZ1;
	m_dLweZ0 = dLweZ0;
	m_dLcmZ0 = dLcmZ0;

	if (m_dFalseE == FALSE_E &&
		m_dFalseN == FALSE_N &&
		m_dKo == K0 &&
		m_dZw == ZW &&
		m_dLcmZ1 == LCMZ1 &&
		m_dLweZ0 == LWEZ0 &&
		m_dLcmZ0 == LCMZ0)
		projectionName = UTM;
	else
		projectionName = USER_DEFINED_PROJECTION;
}

}	// namespace datum_parameters
}	// namespace dynadjust
