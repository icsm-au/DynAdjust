//============================================================================
// Name         : dnaconsts-datums.hpp
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
// Description  : DynAdjust constants include file
//============================================================================

#ifndef DNACONSTS_DATUMS_HPP
#define DNACONSTS_DATUMS_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnatypes.hpp>

const UINT16 AGD66_i =			4202;
const UINT16 AGD84_i =			4203;

const UINT16 GDA94_i_2d =		4283;		// LatLon
const UINT16 GDA94_i_xyz =		4938;		// XYZ
const UINT16 GDA94_i =			4939;		// LatLonEht

const UINT16 GDA2020_i_xyz =	7842;		// XYZ
const UINT16 GDA2020_i =		7843;		// LatLonEht
const UINT16 GDA2020_i_2d =		7844;		// LatLon

const UINT16 ITRF2020_i_xyz =   9988;		// XYZ
const UINT16 ITRF2020_i =       9989;		// LatLonEht
const UINT16 ITRF2014_i_xyz =	7789;		// XYZ
const UINT16 ITRF2014_i =		7912;		// LatLonEht
const UINT16 ITRF2008_i_xyz =	5332;		// XYZ
const UINT16 ITRF2008_i =		7911;		// LatLonEht
const UINT16 ITRF2005_i_xyz =	4896;		// XYZ
const UINT16 ITRF2005_i =		7910;		// LatLonEht
const UINT16 ITRF2000_i_xyz =	4919;		// XYZ
const UINT16 ITRF2000_i =		7909;		// LatLonEht
const UINT16 ITRF1988_i_xyz =	4910;		// XYZ
const UINT16 ITRF1988_i =		7900;		// LatLonEht
const UINT16 ITRF1989_i_xyz =	4911;		// XYZ
const UINT16 ITRF1989_i =		7901;		// LatLonEht
const UINT16 ITRF1990_i_xyz =	4912;		// XYZ
const UINT16 ITRF1990_i =		7902;		// LatLonEht
const UINT16 ITRF1991_i_xyz =	4913;		// XYZ
const UINT16 ITRF1991_i =		7903;		// LatLonEht
const UINT16 ITRF1992_i_xyz =	4914;		// XYZ
const UINT16 ITRF1992_i =		7904;		// LatLonEht
const UINT16 ITRF1993_i_xyz =	4915;		// XYZ
const UINT16 ITRF1993_i =		7905;		// LatLonEht
const UINT16 ITRF1994_i_xyz =	4916;		// XYZ
const UINT16 ITRF1994_i =		7906;		// LatLonEht
const UINT16 ITRF1996_i_xyz =	4917;		// XYZ
const UINT16 ITRF1996_i =		7907;		// LatLonEht
const UINT16 ITRF1997_i_xyz =	4918;		// XYZ
const UINT16 ITRF1997_i =		7908;		// LatLonEht

const UINT16 WGS84_transit_i =		7816;		// LatLonEht
const UINT16 WGS84_transit_i_xyz =	7815;		// XYZ
const UINT16 WGS84_G730_i =			7657;		// LatLonEht
const UINT16 WGS84_G730_i_xyz =		7656;		// XYZ
const UINT16 WGS84_G873_i =			7659;		// LatLonEht
const UINT16 WGS84_G873_i_xyz =		7658;		// XYZ
const UINT16 WGS84_G1150_i =		7661;		// LatLonEht
const UINT16 WGS84_G1150_i_xyz =	7660;		// XYZ
const UINT16 WGS84_G1674_i =		7663;		// LatLonEht
const UINT16 WGS84_G1674_i_xyz =	7662;		// XYZ
const UINT16 WGS84_G1762_i =		7665;		// LatLonEht
const UINT16 WGS84_G1762_i_xyz =	7664;		// XYZ
const UINT16 WGS84_G2139_i =		9754;		// LatLonEht
const UINT16 WGS84_G2139_i_xyz =	9753;		// XYZ
const UINT16 WGS84_i =				4979;		// LatLonEht ensemble
const UINT16 WGS84_i_xyz =			4978;		// XYZ ensemble
const UINT16 WGS84_ensemble_i =		6326;		// WGS84 ensemble


const char* const AGD66_c =			"4202";
const char* const AGD84_c =			"4203";
const char* const GDA94_c_xyz =		"4938";
const char* const GDA94_c =			"4939";
const char* const GDA94_c_2d =		"4283";
const char* const GDA2020_c_xyz =	"7842";
const char* const GDA2020_c =		"7843";
const char* const GDA2020_c_2d =	"7844";

// epsg strings for ITRF provide XYZ definition only
const char* const ITRF2020_c =		"9988";
const char* const ITRF2014_c =		"7789";
const char* const ITRF2008_c =		"5332";
const char* const ITRF2005_c =		"4896";
const char* const ITRF2000_c =		"4919";
const char* const ITRF1997_c =		"4918";
const char* const ITRF1996_c =		"4917";
const char* const ITRF1994_c =		"4916";
const char* const ITRF1993_c =		"4915";
const char* const ITRF1992_c =		"4914";
const char* const ITRF1991_c =		"4913";
const char* const ITRF1990_c =		"4912";
const char* const ITRF1989_c =		"4911";
const char* const ITRF1988_c =		"4910";

// epsg strings for WGS84 provide XYZ definition only
const char* const WGS84_c =			"4978";
const char* const WGS84_ensemble_c = "6326";
const char* const WGS84_transit_c = "7815";
const char* const WGS84_G730_c =	"7656";
const char* const WGS84_G873_c =	"7658";
const char* const WGS84_G1150_c =	"7660";
const char* const WGS84_G1674_c =	"7662";
const char* const WGS84_G1762_c =	"7664";
const char* const WGS84_G2139_c =	"9753";

const char* const AGD66_epoch =		"1.1.1966";
const char* const AGD84_epoch =		"1.1.1984";
const char* const GDA94_epoch =		"1.1.1994";
const char* const GDA2020_epoch =	"1.1.2020";
const char* const ITRF2020_epoch =	"1.1.2020";
const char* const ITRF2014_epoch =	"1.1.2010";
const char* const ITRF2008_epoch =	"1.1.2008";
const char* const ITRF2005_epoch =	"1.1.2005";
const char* const ITRF2000_epoch =	"1.1.2000";
const char* const ITRF1997_epoch =	"1.1.1997";
const char* const ITRF1996_epoch =	"1.1.1996";
const char* const ITRF1994_epoch =	"1.1.1994";
const char* const ITRF1993_epoch =	"1.1.1993";
const char* const ITRF1992_epoch =	"1.1.1992";
const char* const ITRF1991_epoch =	"1.1.1991";
const char* const ITRF1990_epoch =	"1.1.1990";
const char* const ITRF1989_epoch =	"1.1.1989";
const char* const ITRF1988_epoch =	"1.1.1988";

// epochs for WGS84 are not the reference epoch, but rather, 
// the starting date from which the respective WGS realisation 
// was in use
const char* const WGS84_transit_epoch =	"01.01.1987";
const char* const WGS84_G730_epoch =	"02.01.1994";
const char* const WGS84_G873_epoch =	"29.09.1996";
const char* const WGS84_G1150_epoch =	"20.01.2002";
const char* const WGS84_G1674_epoch =	"07.05.2012";
const char* const WGS84_G1762_epoch =	"16.10.2013";
const char* const WGS84_G2139_epoch =	"03.01.2021";

const char* const AGD66_s =			"AGD66";
const char* const AGD84_s =			"AGD84";
const char* const GDA94_s =			"GDA94";
const char* const GDA2020_s =		"GDA2020";
const char* const ITRF2020_s =		"ITRF2020";
const char* const ITRF2014_s =		"ITRF2014";
const char* const ITRF2008_s =		"ITRF2008";
const char* const ITRF2005_s =		"ITRF2005";
const char* const ITRF2000_s =		"ITRF2000";
const char* const ITRF1997_s =		"ITRF1997";
const char* const ITRF1997_s_brief =  "ITRF97";
const char* const ITRF1996_s =		"ITRF1996";
const char* const ITRF1996_s_brief =  "ITRF96";
const char* const ITRF1994_s =		"ITRF1994";
const char* const ITRF1994_s_brief =  "ITRF94";
const char* const ITRF1993_s =		"ITRF1993";
const char* const ITRF1993_s_brief =  "ITRF93";
const char* const ITRF1992_s =		"ITRF1992";
const char* const ITRF1992_s_brief =  "ITRF92";
const char* const ITRF1991_s =		"ITRF1991";
const char* const ITRF1991_s_brief =  "ITRF91";
const char* const ITRF1990_s =		"ITRF1990";
const char* const ITRF1990_s_brief =  "ITRF90";
const char* const ITRF1989_s =		"ITRF1989";
const char* const ITRF1989_s_brief =  "ITRF89";
const char* const ITRF1988_s =		"ITRF1988";
const char* const ITRF1988_s_brief =  "ITRF88";

// The vague and ambiguous WGS84 frame that some refer to an ensemble!
const char* const WGS84_s =				"WGS84";
const char* const WGS84_alias_s =		"WGS 84";
const char* const WGS84_ensemble_s =	"WGS84 (ensemble)";
// The specific realisations of WGS84
const char* const WGS84_transit_s =			"WGS84 (transit)";
const char* const WGS84_transit_alias_s =	"WGS 84 (transit)";
const char* const WGS84_G730_s =			"WGS84 (G730)";
const char* const WGS84_G730_alias_s =		"WGS 84 (G730)";
const char* const WGS84_G873_s =			"WGS84 (G873)";
const char* const WGS84_G873_alias_s =		"WGS 84 (G873)";
const char* const WGS84_G1150_s =			"WGS84 (G1150)";
const char* const WGS84_G1150_alias_s =		"WGS 84 (G1150)";
const char* const WGS84_G1674_s =			"WGS84 (G1674)";
const char* const WGS84_G1674_alias_s =		"WGS 84 (G1674)";
const char* const WGS84_G1762_s =			"WGS84 (G1762)";
const char* const WGS84_G1762_alias_s =		"WGS 84 (G1762)";
const char* const WGS84_G2139_s =			"WGS84 (G2139)";
const char* const WGS84_G2139_alias_s =		"WGS 84 (G2139)";


#endif  // DNACONSTS_DATUMS_HPP
