//============================================================================
// Name         : dnaepsg.hpp
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
// Description  : EPSG implementation
//============================================================================

#ifndef DNAEPSG_H_
#define DNAEPSG_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <string>
#include <iomanip>
#include <sstream>
#include <boost/exception_ptr.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <include/parameters/dnaconsts-datums.hpp>
#include <include/parameters/dnadatumprojectionparam.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

using namespace std;
using namespace boost;

namespace dynadjust {
namespace epsg {

// pairs to help parsing epsg codes
typedef pair<string, UINT32> epsg_string_uint32;
typedef pair<UINT32, string> epsg_uint32_string;

// pairs needed for the epsg structs
typedef pair<string, double> epsg_unit;
typedef pair<string, string> epsg_axis;
typedef pair<string, string> epsg_authority;
typedef pair<string, double> epsg_parameter;

typedef struct {
	string			name_;
	double			semi_major_;
	double			inv_flattening_;
	epsg_authority	authority_;
} epsg_spheroid;

typedef struct {
	double parameters[7];
} epsg_tparams;

typedef struct {
	string			name_;
	epsg_spheroid	spheroid_;
	epsg_tparams	towgs84_;
	epsg_authority	authority_;
} epsg_datum;

typedef struct {
	string			name_;
	double			value_;
	epsg_authority	authority_;
} epsg_primem;

typedef struct {
	string			name;
	epsg_datum		datum_;
	epsg_primem		primem_;
	epsg_unit		unit_;
	epsg_axis		axis_1_;
	epsg_axis		axis_2_;
	epsg_axis		axis_3_;
	epsg_authority	authority_;
} epsg_geogcs;

typedef struct {
	string			name_;
	epsg_authority	authority_;
} epsg_projection;

typedef struct {
	string			name_;
	epsg_geogcs		geogcs_;
	epsg_projection	projection_;
	epsg_parameter	central_meridian_;
	epsg_parameter	latitude_of_origin_;
	epsg_parameter	standard_parallel_1_;
	epsg_parameter	false_easting_;
	epsg_parameter	false_northing_;
	epsg_parameter	scale_factor_;
	epsg_parameter	standard_parallel_2_;
	epsg_unit		unit_;
	epsg_axis		axis_1_;
	epsg_axis		axis_2_;
	epsg_authority	authority_;
} epsg_projcs;

// TODO - The remainder of this file uses hard-coded tests, which are NOT an optimal 
// approach. The most suitable approach is to load epsg names and codes from a config file
// or authoritative epsg file into a vector of string/UINT32 pairs and select accordingly.  
// Watch this space.
template <typename U, typename S>
U epsgCodeFromName(const S& datumName)
{
	if (iequals(datumName, AGD66_s))
		return AGD66_i;		
	if (iequals(datumName, AGD84_s))
		return AGD84_i;		
	if (iequals(datumName, GDA94_s))
		return GDA94_i_xyz;	
	if (iequals(datumName, GDA2020_s))
		return GDA2020_i_xyz;
	// ITRF
	if (iequals(datumName, ITRF2020_s))
		return ITRF2020_i_xyz;
	if (iequals(datumName, ITRF2014_s))
		return ITRF2014_i_xyz;
	if (iequals(datumName, ITRF2008_s))
		return ITRF2008_i_xyz;
	if (iequals(datumName, ITRF2005_s))
		return ITRF2005_i_xyz;
	if (iequals(datumName, ITRF2000_s))
		return ITRF2000_i_xyz;
	if (iequals(datumName, ITRF1997_s) || iequals(datumName, ITRF1997_s_brief))
		return ITRF1997_i_xyz;
	if (iequals(datumName, ITRF1996_s) || iequals(datumName, ITRF1996_s_brief))
		return ITRF1996_i_xyz;
	if (iequals(datumName, ITRF1994_s) || iequals(datumName, ITRF1994_s_brief))
		return ITRF1994_i_xyz;
	if (iequals(datumName, ITRF1993_s) || iequals(datumName, ITRF1993_s_brief))
		return ITRF1993_i_xyz;
	if (iequals(datumName, ITRF1992_s) || iequals(datumName, ITRF1992_s_brief))
		return ITRF1992_i_xyz;
	if (iequals(datumName, ITRF1991_s) || iequals(datumName, ITRF1991_s_brief))
		return ITRF1991_i_xyz;
	if (iequals(datumName, ITRF1990_s) || iequals(datumName, ITRF1990_s_brief))
		return ITRF1990_i_xyz;
	if (iequals(datumName, ITRF1989_s) || iequals(datumName, ITRF1989_s_brief))
		return ITRF1989_i_xyz;
	if (iequals(datumName, ITRF1988_s) || iequals(datumName, ITRF1988_s_brief))
		return ITRF1988_i_xyz;
	// WGS84
	if (iequals(datumName, WGS84_s) || 
		iequals(datumName, WGS84_ensemble_s) ||
		iequals(datumName, WGS84_alias_s))
		return WGS84_i_xyz;
	if (iequals(datumName, WGS84_transit_s) ||
		iequals(datumName, WGS84_transit_alias_s))
		return WGS84_transit_i_xyz;
	if (iequals(datumName, WGS84_G730_s) ||
		iequals(datumName, WGS84_G730_alias_s))
		return WGS84_G730_i_xyz;
	if (iequals(datumName, WGS84_G873_s) ||
		iequals(datumName, WGS84_G873_alias_s))
		return WGS84_G873_i_xyz;
	if (iequals(datumName, WGS84_G1150_s) ||
		iequals(datumName, WGS84_G1150_alias_s))
		return WGS84_G1150_i_xyz;
	if (iequals(datumName, WGS84_G1674_s) ||
		iequals(datumName, WGS84_G1674_alias_s))
		return WGS84_G1674_i_xyz;
	if (iequals(datumName, WGS84_G1762_s) ||
		iequals(datumName, WGS84_G1762_alias_s))
		return WGS84_G1762_i_xyz;
	if (iequals(datumName, WGS84_G2139_s) ||
		iequals(datumName, WGS84_G2139_alias_s))
		return WGS84_G2139_i_xyz;
	
	stringstream ss;
	ss << "epsgCodeFromName: " << datumName << " is not a supported reference frame label." << endl;
	throw boost::enable_current_exception(runtime_error(ss.str()));
}

template <typename S>
S epsgStringFromName(const S& datumName)
{
	// Get code and return corresponding string
	// epsgCodeFromname throws on bad datumName
	UINT32 epsgCode = epsgCodeFromName<UINT32, S>(datumName);

	// get string form of epsg code
	// Reduce all forms(2D / 3D geographic and geocentric) to geocentric
	switch (epsgCode)
	{
	case AGD66_i:
		return AGD66_c;
	case AGD84_i:
		return AGD84_c;
	case GDA94_i:
	case GDA94_i_2d:
	case GDA94_i_xyz:
		return GDA94_c;
	case GDA2020_i:
	case GDA2020_i_2d:
	case GDA2020_i_xyz:
		return GDA2020_c;
	// ITRF
	case ITRF2020_i:
	case ITRF2020_i_xyz:
		return ITRF2020_c;
	case ITRF2014_i:
	case ITRF2014_i_xyz:
		return ITRF2014_c;
	case ITRF2008_i:
	case ITRF2008_i_xyz:
		return ITRF2008_c;
	case ITRF2005_i:
	case ITRF2005_i_xyz:
		return ITRF2005_c;
	case ITRF2000_i:
	case ITRF2000_i_xyz:
		return ITRF2000_c;
	case ITRF1997_i:
	case ITRF1997_i_xyz:
		return ITRF1997_c;
	case ITRF1996_i:
	case ITRF1996_i_xyz:
		return ITRF1996_c;
	case ITRF1994_i:
	case ITRF1994_i_xyz:
		return ITRF1994_c;
	case ITRF1993_i:
	case ITRF1993_i_xyz:
		return ITRF1993_c;
	case ITRF1992_i:
	case ITRF1992_i_xyz:
		return ITRF1992_c;
	case ITRF1991_i:
	case ITRF1991_i_xyz:
		return ITRF1991_c;
	case ITRF1990_i:
	case ITRF1990_i_xyz:
		return ITRF1990_c;
	case ITRF1989_i:
	case ITRF1989_i_xyz:
		return ITRF1989_c;
	case ITRF1988_i:
	case ITRF1988_i_xyz:
		return ITRF1988_c;
	// WGS84
	case WGS84_i:
	case WGS84_i_xyz:
	case WGS84_ensemble_i:
		return WGS84_c;
	case WGS84_transit_i:
	case WGS84_transit_i_xyz:
		return WGS84_transit_c;
	case WGS84_G730_i:
	case WGS84_G730_i_xyz:
		return WGS84_G730_c;
	case WGS84_G873_i:
	case WGS84_G873_i_xyz:
		return WGS84_G873_c;
	case WGS84_G1150_i:
	case WGS84_G1150_i_xyz:
		return WGS84_G1150_c;
	case WGS84_G1674_i:
	case WGS84_G1674_i_xyz:
		return WGS84_G1674_c;
	case WGS84_G1762_i:
	case WGS84_G1762_i_xyz:
		return WGS84_G1762_c;
	case WGS84_G2139_i:
	case WGS84_G2139_i_xyz:
		return WGS84_G2139_c;
	}

	stringstream ss;
	ss << "epsgCodeFromName: " << datumName << " is either unknown or not yet supported." << endl;
	throw boost::enable_current_exception(runtime_error(ss.str()));
}

template <typename U>
bool isEpsgDatumStatic(const U& epsgCode)
{
	stringstream ss;

	switch (epsgCode)
	{
	// AGD66
	case AGD66_i:
	// AGD84
	case AGD84_i:
	// GDA94
	case GDA94_i_xyz:
	case GDA94_i_2d:
	case GDA94_i:
	// GDA2020
	case GDA2020_i_xyz:
	case GDA2020_i_2d:
	case GDA2020_i:
	// WGS84 ensemble
	case WGS84_i_xyz:
	case WGS84_i:
	case WGS84_ensemble_i:
		return true;
	// ITRF....
	case ITRF1988_i_xyz:
	case ITRF1988_i:
	case ITRF1989_i_xyz:
	case ITRF1989_i:
	case ITRF1990_i_xyz:
	case ITRF1990_i:
	case ITRF1991_i_xyz:
	case ITRF1991_i:
	case ITRF1992_i_xyz:
	case ITRF1992_i:
	case ITRF1993_i_xyz:
	case ITRF1993_i:
	case ITRF1994_i_xyz:
	case ITRF1994_i:
	case ITRF1996_i_xyz:
	case ITRF1996_i:
	case ITRF1997_i_xyz:
	case ITRF1997_i:
	case ITRF2000_i_xyz:
	case ITRF2000_i:
	case ITRF2005_i:
	case ITRF2005_i_xyz:
	case ITRF2008_i:
	case ITRF2008_i_xyz:
	case ITRF2014_i:
	case ITRF2014_i_xyz:
	case ITRF2020_i:
	case ITRF2020_i_xyz:
		// WGS84 @ epoch
	case WGS84_transit_i_xyz:
	case WGS84_transit_i:
	case WGS84_G730_i_xyz:
	case WGS84_G730_i:
	case WGS84_G873_i_xyz:
	case WGS84_G873_i:
	case WGS84_G1150_i_xyz:
	case WGS84_G1150_i:
	case WGS84_G1674_i_xyz:
	case WGS84_G1674_i:
	case WGS84_G1762_i_xyz:
	case WGS84_G1762_i:
	case WGS84_G2139_i_xyz:
	case WGS84_G2139_i:
		return false;
	default:
		ss << "isEpsgDatumStatic: EPSG code " << epsgCode << " is not a supported EPSG code." << endl;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	return false;
}

template <typename U>
void spheroidFromEpsgCode(const U& epsgCode, epsg_spheroid& ellipsoid)
{
	switch (epsgCode)
	{
	// Unknown datum based on ANS
	case 4003:
	// AGD66
	case AGD66_i:
	// AGD84
	case AGD84_i:
	// Cocos islands
	//case 4708:
		// authority
		ellipsoid.authority_.first = "EPSG";
		ellipsoid.authority_.second = "7003";
		// ellipsoid params
		ellipsoid.inv_flattening_ = ANS_inv_f;
		ellipsoid.name_ = "Australian National Spheroid";
		ellipsoid.semi_major_ = ANS_a;
		break;
	// GDA94
	case GDA94_i_xyz:
	case GDA94_i_2d:
	case GDA94_i:
	// GDA2020
	case GDA2020_i_xyz:
	case GDA2020_i_2d:
	case GDA2020_i:
	// ITRF....
	case ITRF1988_i_xyz:
	case ITRF1988_i:
	case ITRF1989_i_xyz:
	case ITRF1989_i:
	case ITRF1990_i_xyz:
	case ITRF1990_i:
	case ITRF1991_i_xyz:
	case ITRF1991_i:
	case ITRF1992_i_xyz:
	case ITRF1992_i:
	case ITRF1993_i_xyz:
	case ITRF1993_i:
	case ITRF1994_i_xyz:
	case ITRF1994_i:
	case ITRF1996_i_xyz:
	case ITRF1996_i:
	case ITRF1997_i_xyz:
	case ITRF1997_i:
	case ITRF2000_i_xyz:
	case ITRF2000_i:
	case ITRF2005_i_xyz:
	case ITRF2005_i:
	case ITRF2008_i_xyz:
	case ITRF2008_i:
	case ITRF2014_i_xyz:
	case ITRF2014_i:
	case ITRF2020_i_xyz:
	case ITRF2020_i:
		// authority
		ellipsoid.authority_.first = "EPSG";
		ellipsoid.authority_.second = "7019";
		// ellipsoid params
		ellipsoid.inv_flattening_ = GRS80_inv_f;
		ellipsoid.name_ = "GRS 1980";
		ellipsoid.semi_major_ = GRS80_a;
		break;
	// WGS84
	case WGS84_transit_i:
	case WGS84_transit_i_xyz:
	case WGS84_G730_i:
	case WGS84_G730_i_xyz:
	case WGS84_G873_i:
	case WGS84_G873_i_xyz:
	case WGS84_G1150_i:
	case WGS84_G1150_i_xyz:
	case WGS84_G1674_i:
	case WGS84_G1674_i_xyz:
	case WGS84_G1762_i:
	case WGS84_G1762_i_xyz:
	case WGS84_G2139_i:
	case WGS84_G2139_i_xyz:
	case WGS84_ensemble_i:
	case WGS84_i_xyz:
	case WGS84_i:
		// authority
		ellipsoid.authority_.first = "EPSG";
		ellipsoid.authority_.second = "7030";
		// ellipsoid params
		ellipsoid.inv_flattening_ = WGS84_inv_f;
		ellipsoid.name_ = "WGS 84";
		ellipsoid.semi_major_ = WGS84_a;
		break;
	default:
		stringstream ss;
		ss << "spheroidFromEpsgCode: EPSG code " << epsgCode << " is not a supported EPSG code." << endl;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
}

template <typename U>
string referenceepochFromEpsgCode(const U& epsgCode)
{
	switch (epsgCode)
	{
	// AGD66
	case AGD66_i:
		return AGD66_epoch;
	// AGD84
	case AGD84_i:
		return AGD84_epoch;
	// GDA94
	case GDA94_i_xyz:
	case GDA94_i_2d:
	case GDA94_i:
		return GDA94_epoch;
	// GDA2020
	case GDA2020_i_xyz:
	case GDA2020_i_2d:
	case GDA2020_i:
		return GDA2020_epoch;
	// ITRF....
	case ITRF1988_i_xyz:
	case ITRF1988_i:
		return ITRF1988_epoch;
	case ITRF1989_i_xyz:
	case ITRF1989_i:
		return ITRF1989_epoch;
	case ITRF1990_i_xyz:
	case ITRF1990_i:
		return ITRF1990_epoch;
	case ITRF1991_i_xyz:
	case ITRF1991_i:
		return ITRF1991_epoch;
	case ITRF1992_i_xyz:
	case ITRF1992_i:
		return ITRF1992_epoch;
	case ITRF1993_i_xyz:
	case ITRF1993_i:
		return ITRF1993_epoch;
	case ITRF1994_i_xyz:
	case ITRF1994_i:
		return ITRF1994_epoch;
	case ITRF1996_i_xyz:
	case ITRF1996_i:
		return ITRF1996_epoch;
	case ITRF1997_i_xyz:
	case ITRF1997_i:
		return ITRF1997_epoch;
	case ITRF2000_i_xyz:
	case ITRF2000_i:
		return ITRF2000_epoch;
	case ITRF2005_i_xyz:
	case ITRF2005_i:
		return ITRF2005_epoch;
	case ITRF2008_i_xyz:
	case ITRF2008_i:
		return ITRF2008_epoch;
	case ITRF2014_i_xyz:
	case ITRF2014_i:
		return ITRF2014_epoch;
	case ITRF2020_i_xyz:
	case ITRF2020_i:
		return ITRF2020_epoch;
		// WGS84
	case WGS84_transit_i_xyz:
	case WGS84_transit_i:
		return WGS84_transit_epoch;
	case WGS84_G730_i_xyz:
	case WGS84_G730_i:
		return WGS84_G730_epoch;
	case WGS84_G873_i_xyz:
	case WGS84_G873_i:
		return WGS84_G873_epoch;
	case WGS84_G1150_i_xyz:
	case WGS84_G1150_i:
		return WGS84_G1150_epoch;
	case WGS84_G1674_i_xyz:
	case WGS84_G1674_i:
		return WGS84_G1674_epoch;
	case WGS84_G1762_i_xyz:
	case WGS84_G1762_i:
		return WGS84_G1762_epoch;
	case WGS84_i_xyz:
	case WGS84_i:
	case WGS84_G2139_i_xyz:
	case WGS84_G2139_i:
		return WGS84_G2139_epoch;
	default:
		stringstream ss;
		ss << "referenceepochFromEpsgCode: EPSG code " << epsgCode << " is not a supported EPSG code." << endl;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	return "";
}

template <typename S, typename U>
S datumFromEpsgCode(const U& epsgCode)
{
	switch (epsgCode)
	{
	// AGD66
	case AGD66_i:
		return AGD66_s;
	// AGD84
	case AGD84_i:
		return AGD84_s;
	// Cocos islands
	//case 4708:
	//	return "CocosIs1965";
	// GDA94
	case GDA94_i_xyz:
	case GDA94_i_2d:
	case GDA94_i:
		return GDA94_s;
	// GDA2020
	case GDA2020_i_xyz:
	case GDA2020_i_2d:
	case GDA2020_i:
		return GDA2020_s;
	// ITRF....
	case ITRF1988_i_xyz:
	case ITRF1988_i:
		return ITRF1988_s;
	case ITRF1989_i_xyz:
	case ITRF1989_i:
		return ITRF1989_s;
	case ITRF1990_i_xyz:
	case ITRF1990_i:
		return ITRF1990_s;
	case ITRF1991_i_xyz:
	case ITRF1991_i:
		return ITRF1991_s;
	case ITRF1992_i_xyz:
	case ITRF1992_i:
		return ITRF1992_s;
	case ITRF1993_i_xyz:
	case ITRF1993_i:
		return ITRF1993_s;
	case ITRF1994_i_xyz:
	case ITRF1994_i:
		return ITRF1994_s;
	case ITRF1996_i_xyz:
	case ITRF1996_i:
		return ITRF1996_s;
	case ITRF1997_i_xyz:
	case ITRF1997_i:
		return ITRF1997_s;
	case ITRF2000_i_xyz:
	case ITRF2000_i:
		return ITRF2000_s;
	case ITRF2005_i_xyz:
	case ITRF2005_i:
		return ITRF2005_s;
	case ITRF2008_i_xyz:
	case ITRF2008_i:
		return ITRF2008_s;
	case ITRF2014_i_xyz:
	case ITRF2014_i:
		return ITRF2014_s;
	case ITRF2020_i_xyz:
	case ITRF2020_i:
		return ITRF2020_s;
	case WGS84_i_xyz:
	case WGS84_i:
		return WGS84_s;
	case WGS84_transit_i_xyz:
	case WGS84_transit_i:
		return WGS84_transit_s;
	case WGS84_G730_i_xyz:
	case WGS84_G730_i:
		return WGS84_G730_s;
	case WGS84_G873_i_xyz:
	case WGS84_G873_i:
		return WGS84_G873_s;
	case WGS84_G1150_i_xyz:
	case WGS84_G1150_i:
		return WGS84_G1150_s;
	case WGS84_G1674_i_xyz:
	case WGS84_G1674_i:
		return WGS84_G1674_s;
	case WGS84_G1762_i_xyz:
	case WGS84_G1762_i:
		return WGS84_G1762_s;
	case WGS84_G2139_i_xyz:
	case WGS84_G2139_i:
		return WGS84_G2139_s;
	default:
		stringstream ss;
		ss << "datumFromEpsgCode: EPSG code " << epsgCode << " is not a supported EPSG code." << endl;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	return "";
}

template <typename S>
S datumFromEpsgString(const S& epsgCode)
{
	return datumFromEpsgCode<S, UINT32>(LongFromString<UINT32>(epsgCode));
}


template <typename U>
bool validateEpsgCode(const U& epsgCode)
{
	switch (epsgCode)
	{
	// AGD66
	case AGD66_i:
	// AGD84
	case AGD84_i:
	// GDA94
	case GDA94_i_xyz:
	case GDA94_i_2d:
	case GDA94_i:
	// GDA2020
	case GDA2020_i_xyz:
	case GDA2020_i_2d:
	case GDA2020_i:
	// ITRF1988
	case ITRF1988_i_xyz:
	case ITRF1988_i:
	// ITRF1989
	case ITRF1989_i_xyz:
	case ITRF1989_i:
	// ITRF1990
	case ITRF1990_i_xyz:
	case ITRF1990_i:
	// ITRF1991
	case ITRF1991_i_xyz:
	case ITRF1991_i:
	// ITRF1992
	case ITRF1992_i_xyz:
	case ITRF1992_i:
	// ITRF1993
	case ITRF1993_i_xyz:
	case ITRF1993_i:
	// ITRF1994
	case ITRF1994_i_xyz:
	case ITRF1994_i:
	// ITRF1996
	case ITRF1996_i_xyz:
	case ITRF1996_i:
	// ITRF1997
	case ITRF1997_i_xyz:
	case ITRF1997_i:
	// ITRF2000
	case ITRF2000_i_xyz:
	case ITRF2000_i:
	// ITRF2005
	case ITRF2005_i_xyz:
	case ITRF2005_i:
	// ITRF2008
	case ITRF2008_i_xyz:
	case ITRF2008_i:
	// ITRF2014
	case ITRF2014_i_xyz:
	case ITRF2014_i:
	// ITRF2020
	case ITRF2020_i_xyz:
	case ITRF2020_i:
	// WGS84
	case WGS84_i_xyz:
	case WGS84_i:
	case WGS84_ensemble_i:
	case WGS84_transit_i_xyz:
	case WGS84_transit_i:
	case WGS84_G730_i_xyz:
	case WGS84_G730_i:
	case WGS84_G873_i_xyz:
	case WGS84_G873_i:
	case WGS84_G1150_i_xyz:
	case WGS84_G1150_i:
	case WGS84_G1674_i_xyz:
	case WGS84_G1674_i:
	case WGS84_G1762_i_xyz:
	case WGS84_G1762_i:
	case WGS84_G2139_i_xyz:
	case WGS84_G2139_i:
		return true;
	default:
		stringstream ss;
		ss << "validateEpsgCode: EPSG code " << epsgCode << " is not a supported EPSG code." << endl;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}
	return false;
}

template <typename U>
bool isEpsgWGS84Ensemble(const U& epsgCode)
{
	switch (epsgCode)
	{
	// WGS84
	case WGS84_i_xyz:
	case WGS84_i:
	case WGS84_ensemble_i:
		return true;
	default:
		return false;
	}
	return false;
}

template <typename U>
bool isEpsgStringWGS84Ensemble(const U& epsgString)
{	
	if (iequals(epsgString, WGS84_c) ||
		iequals(epsgString, WGS84_ensemble_c))
		return true;

	return false;
}

template <typename U>
bool isEpsgWGS84(const U& epsgCode)
{
	switch (epsgCode)
	{
		// WGS84
	case WGS84_i_xyz:
	case WGS84_i:
	case WGS84_ensemble_i:
	case WGS84_transit_i_xyz:
	case WGS84_transit_i:
	case WGS84_G730_i_xyz:
	case WGS84_G730_i:
	case WGS84_G873_i_xyz:
	case WGS84_G873_i:
	case WGS84_G1150_i_xyz:
	case WGS84_G1150_i:
	case WGS84_G1674_i_xyz:
	case WGS84_G1674_i:
	case WGS84_G1762_i_xyz:
	case WGS84_G1762_i:
	case WGS84_G2139_i_xyz:
	case WGS84_G2139_i:
		return true;
	default:
		return false;
	}
	return false;
}

template <typename U>
bool isEpsgStringWGS84(const U& epsgString)
{
	if (iequals(epsgString, WGS84_c) ||
		iequals(epsgString, WGS84_ensemble_c) ||
		iequals(epsgString, WGS84_transit_c) ||
		iequals(epsgString, WGS84_G730_c) ||
		iequals(epsgString, WGS84_G873_c) ||
		iequals(epsgString, WGS84_G1150_c) ||
		iequals(epsgString, WGS84_G1674_c) ||
		iequals(epsgString, WGS84_G1762_c) ||
		iequals(epsgString, WGS84_G2139_c))
		return true;

	return false;
}


}	// namespace epsg
}	// namespace dynadjust

#endif /* DNAEPSG_H_ */
