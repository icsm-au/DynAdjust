//============================================================================
// Name         : dnaconsts.hpp
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

#ifndef DNACONSTS_HPP
#define DNACONSTS_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnatypes.hpp>
#include <include/parameters/dnaconsts-datums.hpp>
#include <boost/operators.hpp>

const UINT16 BUF_WIDTH(512);

const UINT16 DNA_COUT(1);
const UINT16 GEOLAB_COUT(2);
const UINT16 NEWGAN_COUT(3);
const UINT16 GMT_OUT(4);

const char* const DEFAULT_DATUM(GDA2020_s);
const char* const DEFAULT_EPSG_S(GDA2020_c);
const UINT32 DEFAULT_EPSG_U(GDA2020_i);
const char* const DEFAULT_EPOCH(GDA2020_epoch);

const char* const XYZ_type("XYZ");
const char* const LLh_type("LLh");
const char* const LLH_type("LLH");
const char* const UTM_type("UTM");
const char* const ENU_type("ENU");
const char* const ORTHOMETRIC_type("orthometric");
const char* const ELLIPSOIDAL_type("ellipsoidal");

const double DAYS_PER_YEAR(365.25);

const double PI(3.1415926535897932384626433832795029);
const double HALF_PI(1.5707963267948966192313216916397514);			//  90 degrees
const double THIRD_PI(PI / 3.);										//	60 degrees
const double QUART_PI(0.7853981633974483096156608458198757);		//  45 degrees
const double PI_20(PI / 9.);										//  20 degrees
const double PI_135(PI - QUART_PI);									// 135 degrees
const double TWO_PI(PI+PI);											// 360 degrees

const double DEG_TO_RAD(PI / 180.0);
const double RAD_TO_DEG(180.0 / PI);
const double DEG_TO_SEC(3600.0);						// Constant to convert DDEGREES to SECONDS
const double RAD_TO_SEC(RAD_TO_DEG * DEG_TO_SEC);		// Constant to convert RADIANS to SECONDS
const double SEC_TO_RAD(DEG_TO_RAD / DEG_TO_SEC);		// Constant to convert SECONDS to RADIANS

const double seconds60 = 0.016666666666666667;
const double seconds30 = 0.008333333333333333;
const double seconds15 = 0.004166666666666667;
const double seconds10 = 0.002777777777777778;
const double seconds05 = 0.001388888888888889;

const double INCH_TO_CM(2.54);							// Constant to convert centimetres to inches
const double CM_TO_INCH(1./INCH_TO_CM);					// Constant to convert inches to centimetres

const double CONSTRAINT_X(0.0001 * 0.0001);
const double CONSTRAINT_Y(0.0001 * 0.0001);
const double CONSTRAINT_Z(0.0001 * 0.0001);

const double PRECISION_1E100(1.0e-100);
const double PRECISION_1E35(1.0e-35);
const double PRECISION_1E25(1.0e-25);
const double PRECISION_1E16(1.0e-16);
const double PRECISION_1E15(1.0e-15);
const double PRECISION_1E14(1.0e-14);
const double PRECISION_1E12(1.0e-12);
const double PRECISION_1E11(1.0e-11);
const double PRECISION_1E10(1.0e-10);
const double PRECISION_1E6(1.0e-6);
const double PRECISION_1E5(1.0e-5);
const double PRECISION_1E4(1.0e-4);
const double PRECISION_1E3(1.0e-3);
const double PRECISION_1E2(1.0e-2);

const long MILLI_TO_NANO(1000000);

const double HPOS_UNCERT_Q0(1.96079);
const double HPOS_UNCERT_Q1(0.004071);
const double HPOS_UNCERT_Q2(0.114276);
const double HPOS_UNCERT_Q3(0.371625);

const double E4_SEC_DEFLECTION(0.0001 * SEC_TO_RAD);

const double STN_SEARCH_RADIUS(0.3);
const string STN_SEARCH_RADIUS_STR("0.3");

const double TOLERANCE_SEC_MIN(1.);
const double TOLERANCE_SEC_MAX(7200.);		// 120 minutes (or 2 degrees)
const double TOLERANCE_ZERO(0.00001);

const double UNRELIABLE(999.99);
const double STABLE_LIMIT(700.);

const double MIN_DBL_VALUE(-1.7e308);
const double MAX_DBL_VALUE(+1.7e308);

const UINT32 MAX_UINT32_VALUE(0xffffffff);

// Define commands for dnaplot
#if defined(_WIN32) || defined(__WIN32__)
	const string FOLDER_SLASH("/");
	//const string FOLDER_SLASH("\\");
	const string DELETE_CMD("del /Q /F");
	const string COPY_CMD("copy /Y");
#elif defined(__linux) || defined(sun) || defined(__unix__)
	const string FOLDER_SLASH("/");
	const string DELETE_CMD("rm -f");
	const string COPY_CMD("cp");
#endif


#endif  // DNACONSTS_HPP
