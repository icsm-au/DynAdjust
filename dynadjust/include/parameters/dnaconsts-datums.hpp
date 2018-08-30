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
const UINT16 GDA94_i_2d =		4283;		// geographic 2D
const UINT16 GDA94_i_xyz =		4938;		// geocentric
const UINT16 GDA94_i =			4939;		// geographic 3D
const UINT16 GDA2020_i_2d =		7844;		// geographic 2D
const UINT16 GDA2020_i_xyz =	7842;		// geocentric
const UINT16 GDA2020_i =		7843;		// geographic 3D
const UINT16 ITRF2014_i =		7789;
const UINT16 ITRF2008_i =		5332;
const UINT16 ITRF2005_i =		4896;
const UINT16 ITRF2000_i_xyz =	4385;
const UINT16 ITRF2000_i =		4919;
const UINT16 ITRF1997_i_xyz =	4338;
const UINT16 ITRF1997_i	=		4918;
const UINT16 ITRF1996_i_xyz =	4337;
const UINT16 ITRF1996_i	=		4917;
const UINT16 ITRF1994_i_xyz =	4336;
const UINT16 ITRF1994_i	=		4916;
const UINT16 ITRF1993_i_xyz =	4335;
const UINT16 ITRF1993_i	=		4915;
const UINT16 ITRF1992_i_xyz =	4334;
const UINT16 ITRF1992_i	=		4914;
const UINT16 ITRF1991_i_xyz =	4333;
const UINT16 ITRF1991_i	=		4913;
const UINT16 ITRF1990_i_xyz =	4332;
const UINT16 ITRF1990_i =		4912;
const UINT16 ITRF1989_i_xyz =	4331;
const UINT16 ITRF1989_i	=		4911;
const UINT16 ITRF1988_i_xyz =	4330;
const UINT16 ITRF1988_i =		4910;
const UINT16 WGS84_i_xyz =		4328;
const UINT16 WGS84_i =			4978;

const char* const AGD66_c =		"4202";
const char* const AGD84_c =		"4203";
const char* const GDA94_c =		"4283";
const char* const GDA94_c_xyz =	"4939";
const char* const GDA2020_c_2d = "7844";
const char* const GDA2020_c_xyz = "7842";
const char* const GDA2020_c =	"7843";
const char* const ITRF2014_c =	"7789";
const char* const ITRF2008_c =	"5332";
const char* const ITRF2005_c =	"4896";
const char* const ITRF2000_c =	"4919";
const char* const ITRF1997_c =	"4918";
const char* const ITRF1996_c =	"4917";
const char* const ITRF1994_c =	"4916";
const char* const ITRF1993_c =	"4915";
const char* const ITRF1992_c =	"4914";
const char* const ITRF1991_c =	"4913";
const char* const ITRF1990_c =	"4912";
const char* const ITRF1989_c =	"4911";
const char* const ITRF1988_c =	"4910";
const char* const WGS84_c =		"4978";	

const char* const AGD66_epoch =		"1.1.1966";
const char* const AGD84_epoch =		"1.1.1984";
const char* const GDA94_epoch =		"1.1.1994";
const char* const GDA2020_epoch =	"1.1.2020";
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

const char* const AGD66_s =			"AGD66";
const char* const AGD84_s =			"AGD84";
const char* const GDA94_s =			"GDA94";
const char* const GDA94_s_3ddeg =	"GDA94 (3D deg)";
const char* const GDA94_s_3d =		"GDA94 (3D)";
const char* const GDA2020_s =		"GDA2020";
const char* const ITRF2014_s =		"ITRF2014";
const char* const ITRF2008_s =		"ITRF2008";
const char* const ITRF2005_s =		"ITRF2005";
const char* const ITRF2000_s_xyz =	"ITRF2000 (geocentric)";
const char* const ITRF2000_s =		"ITRF2000";
const char* const ITRF1997_s_xyz =	"ITRF1997 (geocentric)";
const char* const ITRF1997_s =		"ITRF1997";
const char* const ITRF1997_s_brief =  "ITRF97";
const char* const ITRF1996_s_xyz =	"ITRF1996 (geocentric)";
const char* const ITRF1996_s =		"ITRF1996";
const char* const ITRF1996_s_brief =  "ITRF96";
const char* const ITRF1994_s_xyz =	"ITRF1994 (geocentric)";
const char* const ITRF1994_s =		"ITRF1994";
const char* const ITRF1994_s_brief =  "ITRF94";
const char* const ITRF1993_s_xyz =	"ITRF1993 (geocentric)";
const char* const ITRF1993_s =		"ITRF1993";
const char* const ITRF1993_s_brief =  "ITRF93";
const char* const ITRF1992_s_xyz =	"ITRF1992 (geocentric)";
const char* const ITRF1992_s =		"ITRF1992";
const char* const ITRF1992_s_brief =  "ITRF92";
const char* const ITRF1991_s_xyz =	"ITRF1991 (geocentric)";
const char* const ITRF1991_s =		"ITRF1991";
const char* const ITRF1991_s_brief =  "ITRF91";
const char* const ITRF1990_s_xyz =	"ITRF1990 (geocentric)";
const char* const ITRF1990_s =		"ITRF1990";
const char* const ITRF1990_s_brief =  "ITRF90";
const char* const ITRF1989_s_xyz =	"ITRF1989 (geocentric)";
const char* const ITRF1989_s =		"ITRF1989";
const char* const ITRF1989_s_brief =  "ITRF89";
const char* const ITRF1988_s_xyz =	"ITRF1988 (geocentric)";
const char* const ITRF1988_s =		"ITRF1988";
const char* const ITRF1988_s_brief =  "ITRF88";
const char* const WGS84_s_xyz =		"WGS84 (geocentric)";
const char* const WGS84_s =			"WGS84";


#endif  // DNACONSTS_DATUMS_HPP
