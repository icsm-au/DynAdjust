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

#include <include/parameters/dnaellipsoid.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

using namespace dynadjust::epsg;

namespace dynadjust {
namespace datum_parameters {

CDnaEllipsoid::CDnaEllipsoid(void) 
	// default to GRS80
	: m_epsgCode(DEFAULT_EPSG_U)
{
	// default to GRS80
	SetEllipsoid(m_epsgCode);
	CalculateEllipsoidParams();
}

CDnaEllipsoid::CDnaEllipsoid(const UINT32& epsgCode) 
{
	SetEllipsoid(epsgCode);
	CalculateEllipsoidParams();
}

CDnaEllipsoid::CDnaEllipsoid(const std::string& epsgCode) 
{
	UINT32 epsgCode_uint32 = LongFromString<UINT32>(trimstr(epsgCode));
	SetEllipsoid(epsgCode_uint32);
	CalculateEllipsoidParams();
}

CDnaEllipsoid::CDnaEllipsoid(const CDnaEllipsoid& newEllipsoid) 
{
	m_dA = newEllipsoid.m_dA;
	m_dInv_f = newEllipsoid.m_dInv_f;
	m_dB = newEllipsoid.m_dB;
	m_dF = newEllipsoid.m_dF;
	m_dEccen1 = newEllipsoid.m_dEccen1;
	m_dEccen2 = newEllipsoid.m_dEccen2;
	m_dEccen1_sqd = newEllipsoid.m_dEccen1_sqd;
	m_dEccen2_sqd = newEllipsoid.m_dEccen2_sqd;

	m_epsgCode = newEllipsoid.m_epsgCode;
	m_ellipsoidType = newEllipsoid.m_ellipsoidType;
}

// user-defined ellipsoid
CDnaEllipsoid::CDnaEllipsoid(const double& a, const double& inv_f)
{
	m_epsgCode = 0;
	m_ellipsoidType = USER_DEFINED_ELLIPSOID;
	SetEllipsoidParams(a, inv_f);
}

CDnaEllipsoid& CDnaEllipsoid::operator=(const CDnaEllipsoid& rhs) 
{
	if (this == &rhs)	// check for assignment to self!
		return *this;

	m_dA = rhs.m_dA;
	m_dB = rhs.m_dB;
	m_dInv_f = rhs.m_dInv_f;
	m_dF = rhs.m_dF;
	m_dEccen1 = rhs.m_dEccen1;
	m_dEccen2 = rhs.m_dEccen2;
	m_dEccen1_sqd = rhs.m_dEccen1_sqd;
	m_dEccen2_sqd = rhs.m_dEccen2_sqd;

	m_epsgCode = rhs.m_epsgCode;
	m_ellipsoidType = rhs.m_ellipsoidType;

	return *this;
}


bool CDnaEllipsoid::operator== (const CDnaEllipsoid& rhs) const
{
	return (
		m_dA == rhs.m_dA &&
		m_dInv_f == rhs.m_dInv_f &&
		m_epsgCode == rhs.m_epsgCode &&
		m_ellipsoidType == rhs.m_ellipsoidType
		);
}

void CDnaEllipsoid::SetEllipsoid(const UINT32& epsgCode)
{
	m_epsgCode = epsgCode;
	m_ellipsoidType = EPSG_DEFINED_ELLIPSOID;

	epsg_spheroid ellipsoid;
	spheroidFromEpsgCode<UINT32>(epsgCode, ellipsoid);
	SetEllipsoidParams(ellipsoid.semi_major_, ellipsoid.inv_flattening_);
}

void CDnaEllipsoid::SetEllipsoidParams(const double& a, const double& inv_f) 
{
	m_dA = a;
	m_dInv_f = inv_f;
	CalculateEllipsoidParams();
}

void CDnaEllipsoid::CalculateEllipsoidParams()
{
	m_dB = m_dA * (1.0 - (1.0 / m_dInv_f));
	double dAsqd = m_dA * m_dA;
	double dBsqd = m_dB * m_dB;
	m_dEccen1_sqd = (dAsqd - dBsqd) / dAsqd;	// first ecc squared
	m_dEccen1 = sqrt(m_dEccen1_sqd);			// first ecc
	m_dEccen2_sqd = (dAsqd - dBsqd) / dBsqd;	// second ecc squared
	m_dEccen2 = sqrt(m_dEccen2_sqd);			// second ecc
	m_dF = 1.0 / m_dInv_f;
}

}	// namespace datum_parameters
}	// namespace dynadjust
