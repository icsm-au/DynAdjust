//============================================================================
// Name         : dnatransformationparameters.hpp
// Author       : Roger Fraser
// Contributors : Joshua Batchelor
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
// Description  : Declaration/definition of 7/14 parameters
//============================================================================

#ifndef DNATRANSFORM_PARAM_H_
#define DNATRANSFORM_PARAM_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__)
	#endif
#endif

#include <string>
#include <iomanip>
#include <sstream>
#include <boost/exception_ptr.hpp>

#include <include/parameters/dnaepsg.hpp>

using namespace std;
using namespace boost;

using namespace dynadjust::epsg;

namespace dynadjust {
namespace datum_parameters {

typedef enum
{
	__paramForward__ = 0,
	__paramReverse__ = 1
} transformationDirection;

typedef enum
{
	__static_to_static__ =   0,
	__static_to_dynamic__ =  1,
	__static_to_step__ =     2,
	__dynamic_to_step__ =    3,
	__dynamic_to_static__ =  4,
	__dynamic_to_dynamic__ = 5,
	__step_to_dynamic__ =    6,
	__step_to_static__ =     7,
	__plate_motion_model__ = 8
} transformationType;

template <class S=string, class U>
S TransformationType(const U& u)
{
	switch (u)
	{
		case __static_to_static__:
			return "SS";
		case __static_to_dynamic__:
			return "SD";
		case __static_to_step__:
			// static (S) to intermediate (I)
			return "SI";
		case __dynamic_to_step__:
			// dynamic (D) to intermediate (I)
			return "DI";
		case __dynamic_to_static__:
			return "DS";
		case __dynamic_to_dynamic__:
			return "DD";
		case __step_to_dynamic__:
			// intermediate (I) to dynamic (D)
			return "ID";
		case __step_to_static__:
			// intermediate (I) to static (S)
			return "IS";
		case __plate_motion_model__:
			return "PM";
		default:
			return " ";
	}
}

typedef enum
{
	__frame_frame_same__ = 0,
	__frame_frame_diff__ = 1
} frameSimilarity;

typedef enum
{
	__epoch_epoch_same__ = 0,
	__epoch_epoch_diff__ = 1
} epochSimilarity;


typedef struct transformation_parameter_set_ {
	uint32_uint32_pair			from_to_;				// integer epsg codes for 'from' and 'to' datums
	double						parameters_[14];		// the transformation parameters
	double						reference_epoch_;		// reference epoch for the parameters
	UINT32						reference_frame_;		// reference frame for the parameters
	transformationDirection		paramDirection_;		// the direction of the current set
	void reverse() {
		parameters_[0] *= -1.;
		parameters_[1] *= -1.;
		parameters_[2] *= -1.;
		parameters_[3] *= -1.;
		parameters_[4] *= -1.;
		parameters_[5] *= -1.;
		parameters_[6] *= -1.;
		parameters_[7] *= -1.;
		parameters_[8] *= -1.;
		parameters_[9] *= -1.;
		parameters_[10] *= -1.;
		parameters_[11] *= -1.;
		parameters_[12] *= -1.;
		parameters_[13] *= -1.;
	}
	void add(const double* parameters) {
		parameters_[0] += parameters[0];
		parameters_[1] += parameters[1];
		parameters_[2] += parameters[2];
		parameters_[3] += parameters[3];
		parameters_[4] += parameters[4];
		parameters_[5] += parameters[5];
		parameters_[6] += parameters[6];
		parameters_[7] += parameters[7];
		parameters_[8] += parameters[8];
		parameters_[9] += parameters[9];
		parameters_[10] += parameters[10];
		parameters_[11] += parameters[11];
		parameters_[12] += parameters[12];
		parameters_[13] += parameters[13];
	}
} transformation_parameter_set;

typedef vector<transformation_parameter_set> vtransparams;


////////////////////////////////////////////////////////////////////////////////////
//             SPECIAL NOTE ON ROTATION RATES FOR IERS PARAMETERS
//         ----------------------------------------------------------
//
// There are two different ways of applying the sign conventions for the rotations.
// In both cases a positive rotation is an anti-clockwise rotation, when viewed along
// the positive axis towards the origin but:
//
//   1. The IERS assumes the rotations to be of the points around the 
//      cartesian axes, while;
//   2. The method historically used in Australia assumes the rotations 
//      to be of the Cartesian axes around the points.
//
// Although these two conventions exist, to enforce the property that all rotations 
// describe anticlockwise rotation as positive when viewed along the axis towards the 
// origin, the rotation of the coordinate axes around the points should be a 
// skew-symmetric matrix with the opposite sign to the rotation of the point(s)
// around the coordinate axis.
//
// To avoid confusion and to achieve consistency in the implementation of the 
// transformation functions, all published IERS rotations and associated rotation rates
// have been reversed.
//
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// ITRF2000 <-> ITRF...
// http://itrf.ensg.ign.fr/doc_ITRF/Transfo-ITRF2000_ITRFs.txt

////////////////////////////////////////////////////////////////////////////////////
//             SPECIAL NOTE ON IERS PARAMETERS for ITRF2000 -> ITRFxx
//         ----------------------------------------------------------
//
// Despite what the website says, the linear values for translation are in centimetres,
// not millimetres.
//
// To avoid confusion and to achieve consistency in the implementation of the 
// transformation functions, all published IERS translations and associated 
// translation rates have been converted to millimetres.
//
////////////////////////////////////////////////////////////////////////////////////

// ITRF 2000 -> ITRF 1997
template <class T, class U>
struct _itrf2000_to_itrf1997_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1997_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1997_<T, U>::reference_epoch = 1997.0;

template <class T, class U>
const T _itrf2000_to_itrf1997_<T, U>::transformationParameters[14] =
{
	  6.7,		// x translation (millimetres) - CONVERTED FROM CM
	  6.1,		// y translation (millimetres) - CONVERTED FROM CM
	-18.5,		// z translation (millimetres) - CONVERTED FROM CM
	 1.55,		// scale (ppb)
	  0.0,		// x rotation (milli-arc-seconds)
	  0.0,		// y rotation (milli-arc-seconds)
	  0.0,		// z rotation (milli-arc-seconds)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1997 : public _itrf2000_to_itrf1997_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1996
template <class T, class U>
struct _itrf2000_to_itrf1996_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1996_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1996_<T, U>::reference_epoch = 1997.0;

template <class T, class U>
const T _itrf2000_to_itrf1996_<T, U>::transformationParameters[14] =
{
	  6.7,		// x translation (millimetres) - CONVERTED FROM CM
	  6.1,		// y translation (millimetres) - CONVERTED FROM CM
	-18.5,		// z translation (millimetres) - CONVERTED FROM CM
	 1.55,		// scale (ppb)
	  0.0,		// x rotation (milli-arc-seconds)
	  0.0,		// y rotation (milli-arc-seconds)
	  0.0,		// z rotation (milli-arc-seconds)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1996 : public _itrf2000_to_itrf1996_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1994
template <class T, class U>
struct _itrf2000_to_itrf1994_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1994_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1994_<T, U>::reference_epoch = 1997.0;

template <class T, class U>
const T _itrf2000_to_itrf1994_<T, U>::transformationParameters[14] =
{
	  6.7,		// x translation (millimetres) - CONVERTED FROM CM
	  6.1,		// y translation (millimetres) - CONVERTED FROM CM
	-18.5,		// z translation (millimetres) - CONVERTED FROM CM
	 1.55,		// scale (ppb)
	  0.0,		// x rotation (milli-arc-seconds)
	  0.0,		// y rotation (milli-arc-seconds)
	  0.0,		// z rotation (milli-arc-seconds)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1994 : public _itrf2000_to_itrf1994_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1993
template <class T, class U>
struct _itrf2000_to_itrf1993_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1993_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1993_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1993_<T, U>::transformationParameters[14] =
{
	 12.7,		// x translation (millimetres) - CONVERTED FROM CM
	  6.5,		// y translation (millimetres) - CONVERTED FROM CM
	-20.9,		// z translation (millimetres) - CONVERTED FROM CM
	 1.95,		// scale (ppb)
	 0.39,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.80,		// y rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 1.14,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	 -2.9,		// x translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -0.2,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -0.6,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.11,		// x rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.19,		// y rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.07		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1993 : public _itrf2000_to_itrf1993_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1992
template <class T, class U>
struct _itrf2000_to_itrf1992_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1992_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1992_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1992_<T, U>::transformationParameters[14] =
{
	 14.7,		// x translation (millimetres) - CONVERTED FROM CM
	 13.5,		// y translation (millimetres) - CONVERTED FROM CM
	-13.9,		// z translation (millimetres) - CONVERTED FROM CM
	 0.75,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.18,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1992	: public _itrf2000_to_itrf1992_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1991
template <class T, class U>
struct _itrf2000_to_itrf1991_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1991_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1991_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1991_<T, U>::transformationParameters[14] =
{
	 26.7,		// x translation (millimetres) - CONVERTED FROM CM
	 27.5,		// y translation (millimetres) - CONVERTED FROM CM
	-19.9,		// z translation (millimetres) - CONVERTED FROM CM
	 2.15,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.18,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1991	: public _itrf2000_to_itrf1991_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1990
template <class T, class U>
struct _itrf2000_to_itrf1990_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1990_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1990_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1990_<T, U>::transformationParameters[14] =
{
	 24.7,		// x translation (millimetres) - CONVERTED FROM CM
	 23.5,		// y translation (millimetres) - CONVERTED FROM CM
	-35.9,		// z translation (millimetres) - CONVERTED FROM CM
	 2.45,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.18,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1990 : public _itrf2000_to_itrf1990_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1989
template <class T, class U>
struct _itrf2000_to_itrf1989_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1989_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1989_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1989_<T, U>::transformationParameters[14] =
{
	 29.7,		// x translation (millimetres) - CONVERTED FROM CM
	 47.5,		// y translation (millimetres) - CONVERTED FROM CM
	-73.9,		// z translation (millimetres) - CONVERTED FROM CM
	 5.85,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.18,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1989 : public _itrf2000_to_itrf1989_<T, U>
{
public:
};

// ITRF 2000 -> ITRF 1988
template <class T, class U>
struct _itrf2000_to_itrf1988_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_itrf1988_<T, U>::reference_frame = ITRF2000_i;

template <class T, class U>
const T _itrf2000_to_itrf1988_<T, U>::reference_epoch = 1988.0;

template <class T, class U>
const T _itrf2000_to_itrf1988_<T, U>::transformationParameters[14] =
{
	 24.7,		// x translation (millimetres) - CONVERTED FROM CM
	 11.5,		// y translation (millimetres) - CONVERTED FROM CM
	-97.9,		// z translation (millimetres) - CONVERTED FROM CM
	 8.95,		// scale (ppb)
	-0.10,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.18,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr) - CONVERTED FROM CM
	 -1.4,		// z translation rate (millimetres p/yr) - CONVERTED FROM CM
	 0.01,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2000_ITRF1988 : public _itrf2000_to_itrf1988_<T, U>
{
public:
};

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// ITRF2005 <-> ITRF...
// http://itrf.ensg.ign.fr/doc_ITRF/Transfo-ITRF2005_ITRF2000.txt

// ITRF 2005 -> ITRF 2000
template <class T, class U>
struct _itrf2005_to_itrf2000_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2005_to_itrf2000_<T, U>::reference_frame = ITRF2005_i;

template <class T, class U>
const T _itrf2005_to_itrf2000_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2005_to_itrf2000_<T, U>::transformationParameters[14] =
{
	  0.1,		// x translation (millimetres)
	 -0.8,		// y translation (millimetres)
	 -5.8,		// z translation (millimetres)
	 0.40,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	 -0.2,		// x translation rate (millimetres p/yr)
	  0.1,		// y translation rate (millimetres p/yr)
	 -1.8,		// z translation rate (millimetres p/yr)
	 0.08,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2005_ITRF2000 : public _itrf2005_to_itrf2000_<T, U>
{
public:
};
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// ITRF2008 <-> ITRF...
// http://itrf.ensg.ign.fr/doc_ITRF/Transfo-ITRF2008_ITRFs.txt

// ITRF 2008 -> ITRF 2005
template <class T, class U>
struct _itrf2008_to_itrf2005_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf2005_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf2005_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf2005_<T, U>::transformationParameters[14] =
{
	 -2.0,		// x translation (millimetres). Note this differs from Zuheir's JOG paper DOI 10.1007/s00190-011-0444-4
	 -0.9,		// y translation (millimetres)
	 -4.7,		// z translation (millimetres)
	 0.94,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.3,		// x translation rate (millimetres p/yr)
	  0.0,		// y translation rate (millimetres p/yr)
	  0.0,		// z translation rate (millimetres p/yr)
	 0.00,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2008_ITRF2005 : public _itrf2008_to_itrf2005_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 2000
template <class T, class U>
struct _itrf2008_to_itrf2000_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf2000_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf2000_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf2000_<T, U>::transformationParameters[14] =
{
	 -1.9,		// x translation (millimetres)
	 -1.7,		// y translation (millimetres)
	-10.5,		// z translation (millimetres)
	 1.34,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.1,		// x translation rate (millimetres p/yr)
	  0.1,		// y translation rate (millimetres p/yr)
	 -1.8,		// z translation rate (millimetres p/yr)
	 0.08,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2008_ITRF2000 : public _itrf2008_to_itrf2000_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1997
template <class T, class U>
struct _itrf2008_to_itrf1997_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1997_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1997_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1997_<T, U>::transformationParameters[14] =
{
	  4.8,		// x translation (millimetres)
	  2.6,		// y translation (millimetres)
	-33.2,		// z translation (millimetres)
	 2.92,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1997 : public _itrf2008_to_itrf1997_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1996
template <class T, class U>
struct _itrf2008_to_itrf1996_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1996_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1996_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1996_<T, U>::transformationParameters[14] =
{
	  4.8,		// x translation (millimetres)
	  2.6,		// y translation (millimetres)
	-33.2,		// z translation (millimetres)
	 2.92,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1996 : public _itrf2008_to_itrf1996_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1994
template <class T, class U>
struct _itrf2008_to_itrf1994_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1994_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1994_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1994_<T, U>::transformationParameters[14] =
{
	  4.8,		// x translation (millimetres)
	  2.6,		// y translation (millimetres)
	-33.2,		// z translation (millimetres)
	 2.92,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1994 : public _itrf2008_to_itrf1994_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1993
template <class T, class U>
struct _itrf2008_to_itrf1993_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1993_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1993_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1993_<T, U>::transformationParameters[14] =
{
	-24.0,		// x translation (millimetres)
	  2.4,		// y translation (millimetres)
	-38.6,		// z translation (millimetres)
	 3.41,		// scale (ppb)
	 1.71,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 1.48,		// y rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.30,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	 -2.8,		// x translation rate (millimetres p/yr)
	 -0.1,		// y translation rate (millimetres p/yr)
	 -2.4,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.11,		// x rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.19,		// y rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.07		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1993 : public _itrf2008_to_itrf1993_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1992
template <class T, class U>
struct _itrf2008_to_itrf1992_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1992_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1992_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1992_<T, U>::transformationParameters[14] =
{
	 12.8,		// x translation (millimetres)
	  4.6,		// y translation (millimetres)
	-41.2,		// z translation (millimetres)
	 2.21,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1992 : public _itrf2008_to_itrf1992_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1991
template <class T, class U>
struct _itrf2008_to_itrf1991_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1991_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1991_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1991_<T, U>::transformationParameters[14] =
{
	 24.8,		// x translation (millimetres)
	 18.6,		// y translation (millimetres)
	-47.2,		// z translation (millimetres)
	 3.61,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1991 : public _itrf2008_to_itrf1991_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1990
template <class T, class U>
struct _itrf2008_to_itrf1990_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1990_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1990_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1990_<T, U>::transformationParameters[14] =
{
	 22.8,		// x translation (millimetres)
	 14.6,		// y translation (millimetres)
	-63.2,		// z translation (millimetres)
	 3.91,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1990 : public _itrf2008_to_itrf1990_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1989
template <class T, class U>
struct _itrf2008_to_itrf1989_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1989_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1989_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1989_<T, U>::transformationParameters[14] =
{
	 27.8,		// x translation (millimetres)
	 38.6,		// y translation (millimetres)
   -101.2,		// z translation (millimetres)
	 7.31,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1989 : public _itrf2008_to_itrf1989_<T, U>
{
public:
};

// ITRF 2008 -> ITRF 1988
template <class T, class U>
struct _itrf2008_to_itrf1988_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_itrf1988_<T, U>::reference_frame = ITRF2008_i;

template <class T, class U>
const T _itrf2008_to_itrf1988_<T, U>::reference_epoch = 2000.0;

template <class T, class U>
const T _itrf2008_to_itrf1988_<T, U>::transformationParameters[14] =
{
	 22.8,		// x translation (millimetres)
	  2.6,		// y translation (millimetres)
   -125.2,		// z translation (millimetres)
	10.41,		// scale (ppb)
	-0.10,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.06,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.2,		// z translation rate (millimetres p/yr)
	 0.09,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2008_ITRF1988 : public _itrf2008_to_itrf1988_<T, U>
{
public:
};

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// ITRF2014 <-> ITRF...
// http://itrf.ensg.ign.fr/doc_ITRF/Transfo-ITRF2014_ITRFs.txt

// ITRF 2014 -> ITRF 2008
template <class T, class U>
struct _itrf2014_to_itrf2008_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf2008_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf2008_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf2008_<T, U>::transformationParameters[14] =
{
	  1.6,		// x translation (millimetres)
	  1.9,		// y translation (millimetres)
	  2.4,		// z translation (millimetres)
	-0.02,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)
	
	  0.0,		// x translation rate (millimetres p/yr)
	  0.0,		// y translation rate (millimetres p/yr)
	 -0.1,		// z translation rate (millimetres p/yr)
	 0.03,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2014_ITRF2008 : public _itrf2014_to_itrf2008_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 2005
template <class T, class U>
struct _itrf2014_to_itrf2005_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf2005_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf2005_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf2005_<T, U>::transformationParameters[14] =
{
	  2.6,		// x translation (millimetres)
	  1.0,		// y translation (millimetres)
	 -2.3,		// z translation (millimetres)
	 0.92,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)
	 
	  0.3,		// x translation rate (millimetres p/yr)
	  0.0,		// y translation rate (millimetres p/yr)
	 -0.1,		// z translation rate (millimetres p/yr)
	 0.03,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2014_ITRF2005 : public _itrf2014_to_itrf2005_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 2000
template <class T, class U>
struct _itrf2014_to_itrf2000_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf2000_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf2000_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf2000_<T, U>::transformationParameters[14] =
{
	  0.7,		// x translation (millimetres)
	  1.2,		// y translation (millimetres)
	-26.1,		// z translation (millimetres)
	 2.12,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	  0.1,		// y translation rate (millimetres p/yr)
	 -1.9,		// z translation rate (millimetres p/yr)
	 0.11,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2014_ITRF2000 : public _itrf2014_to_itrf2000_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1997
template <class T, class U>
struct _itrf2014_to_itrf1997_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1997_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1997_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1997_<T, U>::transformationParameters[14] =
{
	  7.4,		// x translation (millimetres)
	 -0.5,		// y translation (millimetres)
	-62.8,		// z translation (millimetres)
	 3.80,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1997 : public _itrf2014_to_itrf1997_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1996
template <class T, class U>
struct _itrf2014_to_itrf1996_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1996_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1996_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1996_<T, U>::transformationParameters[14] =
{
	  7.4,		// x translation (millimetres)
	 -0.5,		// y translation (millimetres)
	-62.8,		// z translation (millimetres)
	 3.80,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1996 : public _itrf2014_to_itrf1996_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1994
template <class T, class U>
struct _itrf2014_to_itrf1994_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1994_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1994_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1994_<T, U>::transformationParameters[14] =
{
	  7.4,		// x translation (millimetres)
	 -0.5,		// y translation (millimetres)
	-62.8,		// z translation (millimetres)
	 3.80,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1994 : public _itrf2014_to_itrf1994_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1993
template <class T, class U>
struct _itrf2014_to_itrf1993_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1993_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1993_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1993_<T, U>::transformationParameters[14] =
{
	-50.4,		// x translation (millimetres)
	  3.3,		// y translation (millimetres)
	-60.2,		// z translation (millimetres)
	 4.29,		// scale (ppb)
	 2.81,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 3.38,		// y rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.40,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	 -2.8,		// x translation rate (millimetres p/yr)
	 -0.1,		// y translation rate (millimetres p/yr)
	 -2.5,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.11,		// x rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.19,		// y rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.07		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1993 : public _itrf2014_to_itrf1993_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1992
template <class T, class U>
struct _itrf2014_to_itrf1992_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1992_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1992_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1992_<T, U>::transformationParameters[14] =
{
	 15.4,		// x translation (millimetres)
	  1.5,		// y translation (millimetres)
	-70.8,		// z translation (millimetres)
	 3.09,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1992	 : public _itrf2014_to_itrf1992_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1991
template <class T, class U>
struct _itrf2014_to_itrf1991_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1991_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1991_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1991_<T, U>::transformationParameters[14] =
{
	 27.4,		// x translation (millimetres)
	 15.5,		// y translation (millimetres)
	-76.8,		// z translation (millimetres)
	 4.49,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1991 : public _itrf2014_to_itrf1991_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1990
template <class T, class U>
struct _itrf2014_to_itrf1990_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1990_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1990_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1990_<T, U>::transformationParameters[14] =
{
	 25.4,		// x translation (millimetres)
	 11.5,		// y translation (millimetres)
	-92.8,		// z translation (millimetres)
	 4.79,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 
	  0.1,		// x translation rate (millimetres p/yr)
	 -0.5,		// y translation rate (millimetres p/yr)
	 -3.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1990 : public _itrf2014_to_itrf1990_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1989
template <class T, class U>
struct _itrf2014_to_itrf1989_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1989_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1989_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1989_<T, U>::transformationParameters[14] =
{
	  30.4,		// x translation (millimetres)
	  35.5,		// y translation (millimetres)
	-130.8,		// z translation (millimetres)
	  8.19,		// scale (ppb)
	  0.00,		// x rotation (milli-arc-seconds)
	  0.00,		// y rotation (milli-arc-seconds)
	 -0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	  
	   0.1,		// x translation rate (millimetres p/yr)
	  -0.5,		// y translation rate (millimetres p/yr)
	  -3.3,		// z translation rate (millimetres p/yr)
	  0.12,		// scale rate (ppb p/yr)
	  0.00,		// x rotation rate (milli-arc-seconds p/yr)
	  0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 -0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1989 : public _itrf2014_to_itrf1989_<T, U>
{
public:
};

// ITRF 2014 -> ITRF 1988
template <class T, class U>
struct _itrf2014_to_itrf1988_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_itrf1988_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_itrf1988_<T, U>::reference_epoch = 2010.0;

template <class T, class U>
const T _itrf2014_to_itrf1988_<T, U>::transformationParameters[14] =
{
	  25.4,		// x translation (millimetres)
	  -0.5,		// y translation (millimetres)
	-154.8,		// z translation (millimetres)
	 11.29,		// scale (ppb)
	 -0.10,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	  0.00,		// y rotation (milli-arc-seconds)
	 -0.26,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	  
	   0.1,		// x translation rate (millimetres p/yr)
	  -0.5,		// y translation rate (millimetres p/yr)
	  -3.3,		// z translation rate (millimetres p/yr)
	  0.12,		// scale rate (ppb p/yr)
	  0.00,		// x rotation rate (milli-arc-seconds p/yr)
	  0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 -0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2014_ITRF1988 : public _itrf2014_to_itrf1988_<T, U>
{
public:
};

////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// ITRF2014 <-> GDA2020
//
// ICSM Release note (March 2017)
// 'file://C:\Data\GEODESY\ICSM\DATUM\GDA2020 media\GDA2020 Release Note.docx'

// ITRF2014 -> GDA2020
template <class T, class U>
struct _itrf2014_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_gda2020_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2014_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2014_to_gda2020_<T, U>::transformationParameters[14] =
{
	    0.0,	// x translation (millimetres)
	    0.0,	// y translation (millimetres)
	    0.0,	// z translation (millimetres)
	    0.0,	// scale (ppb)
	    0.0,	// x rotation (milli-arc-seconds)
	    0.0,	// y rotation (milli-arc-seconds)
	    0.0,	// z rotation (milli-arc-seconds)
	    
	    0.0,	// x translation rate (millimetres p/yr)
	    0.0,	// y translation rate (millimetres p/yr)
	    0.0,	// z translation rate (millimetres p/yr)
	    0.0,	// scale rate (ppb p/yr)
	// Euler parameters:
	//  - pole latitude:  32.2447
	// 	- pole longitude: 38.2022
	//  - rotation rate:   0.6285
	1.50379,	// x rotation rate (milli-arc-seconds p/yr)
	1.18346,	// y rotation rate (milli-arc-seconds p/yr)
	1.20716		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2014_GDA2020 : public _itrf2014_to_gda2020_<T, U>
{
public:
};
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// ITRF2020 <-> ITRF...
// https://itrf.ign.fr/docs/solutions/itrf2020/Transfo-ITRF2020_TRFs.txt

// ITRF 2020 -> ITRF 2014
template <class T, class U>
struct _itrf2020_to_itrf2014_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf2014_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf2014_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf2014_<T, U>::transformationParameters[14] =
{
	 -1.4,		// x translation (millimetres)
	 -0.9,		// y translation (millimetres)
	  1.4,		// z translation (millimetres)
	-0.42,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.1,		// y translation rate (millimetres p/yr)
	  0.2,		// z translation rate (millimetres p/yr)
	 0.00,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2020_ITRF2014 : public _itrf2020_to_itrf2014_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 2008
template <class T, class U>
struct _itrf2020_to_itrf2008_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf2008_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf2008_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf2008_<T, U>::transformationParameters[14] =
{
	  0.2,		// x translation (millimetres)
	  1.0,		// y translation (millimetres)
	  3.3,		// z translation (millimetres)
	-0.29,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.0,		// x translation rate (millimetres p/yr)
	 -0.1,		// y translation rate (millimetres p/yr)
	  0.1,		// z translation rate (millimetres p/yr)
	 0.03,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2020_ITRF2008 : public _itrf2020_to_itrf2008_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 2005
template <class T, class U>
struct _itrf2020_to_itrf2005_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf2005_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf2005_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf2005_<T, U>::transformationParameters[14] =
{
	  2.7,		// x translation (millimetres)
	  0.1,		// y translation (millimetres)
	 -1.4,		// z translation (millimetres)
	 0.65,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.3,		// x translation rate (millimetres p/yr)
	 -0.1,		// y translation rate (millimetres p/yr)
	  0.1,		// z translation rate (millimetres p/yr)
	 0.03,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2020_ITRF2005 : public _itrf2020_to_itrf2005_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 2000
template <class T, class U>
struct _itrf2020_to_itrf2000_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf2000_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf2000_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf2000_<T, U>::transformationParameters[14] =
{
	 -0.2,		// x translation (millimetres)
	  0.8,		// y translation (millimetres)
	-34.2,		// z translation (millimetres)
	 2.25,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	 0.00,		// z rotation (milli-arc-seconds)

	  0.1,		// x translation rate (millimetres p/yr)
	  0.0,		// y translation rate (millimetres p/yr)
	 -1.7,		// z translation rate (millimetres p/yr)
	 0.11,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 0.00		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2020_ITRF2000 : public _itrf2020_to_itrf2000_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1997
template <class T, class U>
struct _itrf2020_to_itrf1997_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1997_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1997_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1997_<T, U>::transformationParameters[14] =
{
	  6.5,		// x translation (millimetres)
	 -3.9,		// y translation (millimetres)
	-77.9,		// z translation (millimetres)
	 3.98,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1997 : public _itrf2020_to_itrf1997_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1996
template <class T, class U>
struct _itrf2020_to_itrf1996_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1996_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1996_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1996_<T, U>::transformationParameters[14] =
{
	  6.5,		// x translation (millimetres)
	 -3.9,		// y translation (millimetres)
	-77.9,		// z translation (millimetres)
	 3.98,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1996 : public _itrf2020_to_itrf1996_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1994
template <class T, class U>
struct _itrf2020_to_itrf1994_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1994_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1994_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1994_<T, U>::transformationParameters[14] =
{
	  6.5,		// x translation (millimetres)
	 -3.9,		// y translation (millimetres)
	-77.9,		// z translation (millimetres)
	 3.98,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1994 : public _itrf2020_to_itrf1994_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1993
template <class T, class U>
struct _itrf2020_to_itrf1993_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1993_<T, U>::reference_frame = ITRF2014_i;

template <class T, class U>
const T _itrf2020_to_itrf1993_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1993_<T, U>::transformationParameters[14] =
{
	-65.8,		// x translation (millimetres)
	  1.9,		// y translation (millimetres)
	-71.3,		// z translation (millimetres)
	 4.47,		// scale (ppb)
	 3.36,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 4.33,		// y rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.75,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	 -2.8,		// x translation rate (millimetres p/yr)
	 -0.2,		// y translation rate (millimetres p/yr)
	 -2.3,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.11,		// x rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	 0.19,		// y rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
	-0.07		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1993 : public _itrf2020_to_itrf1993_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1992
template <class T, class U>
struct _itrf2020_to_itrf1992_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1992_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1992_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1992_<T, U>::transformationParameters[14] =
{
	 14.5,		// x translation (millimetres)
	 -1.9,		// y translation (millimetres)
	-85.9,		// z translation (millimetres)
	 3.27,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1992 : public _itrf2020_to_itrf1992_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1991
template <class T, class U>
struct _itrf2020_to_itrf1991_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1991_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1991_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1991_<T, U>::transformationParameters[14] =
{
	 26.5,		// x translation (millimetres)
	 12.1,		// y translation (millimetres)
	-91.9,		// z translation (millimetres)
	 4.67,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1991 : public _itrf2020_to_itrf1991_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1990
template <class T, class U>
struct _itrf2020_to_itrf1990_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1990_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1990_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1990_<T, U>::transformationParameters[14] =
{
	 24.5,		// x translation (millimetres)
	  8.1,		// y translation (millimetres)
   -107.9,		// z translation (millimetres)
	 4.97,		// scale (ppb)
	 0.00,		// x rotation (milli-arc-seconds)
	 0.00,		// y rotation (milli-arc-seconds)
	-0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	  0.1,		// x translation rate (millimetres p/yr)
	 -0.6,		// y translation rate (millimetres p/yr)
	 -3.1,		// z translation rate (millimetres p/yr)
	 0.12,		// scale rate (ppb p/yr)
	 0.00,		// x rotation rate (milli-arc-seconds p/yr)
	 0.00,		// y rotation rate (milli-arc-seconds p/yr)
	-0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1990 : public _itrf2020_to_itrf1990_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1989
template <class T, class U>
struct _itrf2020_to_itrf1989_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1989_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1989_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1989_<T, U>::transformationParameters[14] =
{
	  29.5,		// x translation (millimetres)
	  32.1,		// y translation (millimetres)
	-145.9,		// z translation (millimetres)
	  8.37,		// scale (ppb)
	  0.00,		// x rotation (milli-arc-seconds)
	  0.00,		// y rotation (milli-arc-seconds)
	 -0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	   0.1,		// x translation rate (millimetres p/yr)
	  -0.6,		// y translation rate (millimetres p/yr)
	  -3.1,		// z translation rate (millimetres p/yr)
	  0.12,		// scale rate (ppb p/yr)
	  0.00,		// x rotation rate (milli-arc-seconds p/yr)
	  0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 -0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1989 : public _itrf2020_to_itrf1989_<T, U>
{
public:
};

// ITRF 2020 -> ITRF 1988
template <class T, class U>
struct _itrf2020_to_itrf1988_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_itrf1988_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_itrf1988_<T, U>::reference_epoch = 2015.0;

template <class T, class U>
const T _itrf2020_to_itrf1988_<T, U>::transformationParameters[14] =
{
	  24.5,		// x translation (millimetres)
	  -3.9,		// y translation (millimetres)
	-169.9,		// z translation (millimetres)
	 11.47,		// scale (ppb)
	 -0.10,		// x rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)
	  0.00,		// y rotation (milli-arc-seconds)
	 -0.36,		// z rotation (milli-arc-seconds) - REVERSED (SEE SPECIAL NOTE ABOVE)

	   0.1,		// x translation rate (millimetres p/yr)
	  -0.6,		// y translation rate (millimetres p/yr)
	  -3.1,		// z translation rate (millimetres p/yr)
	  0.12,		// scale rate (ppb p/yr)
	  0.00,		// x rotation rate (milli-arc-seconds p/yr)
	  0.00,		// y rotation rate (milli-arc-seconds p/yr)
	 -0.02		// z rotation rate (milli-arc-seconds p/yr) - REVERSED (SEE SPECIAL NOTE ABOVE)
};

template <class T, class U>
class ITRF2020_ITRF1988 : public _itrf2020_to_itrf1988_<T, U>
{
public:
};

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// ITRF2020 <-> GDA2020
//
// Computed by:
//   1. Propagating ITRF2020 parameters to reference epoch 2020.0
//   2. Adding AU PMM rates to ITRF2020 rates
// 

// ITRF2020 -> GDA2020
template <class T, class U>
struct _itrf2020_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2020_to_gda2020_<T, U>::reference_frame = ITRF2020_i;

template <class T, class U>
const T _itrf2020_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2020_to_gda2020_<T, U>::transformationParameters[14] =
{
	   -1.4,	// x translation (millimetres)
	   -1.4,	// y translation (millimetres)
		2.4,	// z translation (millimetres)
	  -0.42,	// scale (ppb)
		0.0,	// x rotation (milli-arc-seconds)
		0.0,	// y rotation (milli-arc-seconds)
		0.0,	// z rotation (milli-arc-seconds)

		// ITRF2020 translation rates:
		0.0,	// x translation rate (millimetres p/yr)
	   -0.1,	// y translation rate (millimetres p/yr)
		0.2,	// z translation rate (millimetres p/yr)
		0.0,	// scale rate (ppb p/yr)
		// AU PMM rotation rates:
		1.50379,	// x rotation rate (milli-arc-seconds p/yr)
		1.18346,	// y rotation rate (milli-arc-seconds p/yr)
		1.20716		// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2020_GDA2020 : public _itrf2020_to_gda2020_<T, U>
{
public:
};
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// PLATE MOTION MODELS

// AUS
template <class T, class U>
class AUS_PLATE_MOTION_MODEL : public _itrf2014_to_gda2020_<T, U>
{
public:
};


////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// ITRFxxxx <-> GDA2020
// Dawson Fraser
// file://C:\Data\GEODESY\ICSM\DATUM\gda2020-itrf-transformation-parameters.xlsx

// ITRF1996 -> GDA2020
template <class T, class U>
struct _itrf1996_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf1996_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _itrf1996_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf1996_to_gda2020_<T, U>::transformationParameters[14] =
{
	-480.710,	// x translation (millimetres)
	  75.160,	// y translation (millimetres)
	 574.710,	// z translation (millimetres)
	  6.9950,	// scale (ppb)
	 10.2995,	// x rotation (milli-arc-seconds)
	 21.7458,	// y rotation (milli-arc-seconds)
	  9.8292,	// z rotation (milli-arc-seconds)

	 -21.800,	// x translation rate (millimetres p/yr)
	   4.710,	// y translation rate (millimetres p/yr)
	  26.270,	// z translation rate (millimetres p/yr)
	  0.3880,	// scale rate (ppb p/yr)
	  2.0203,	// x rotation rate (milli-arc-seconds p/yr)
	  2.1735,	// y rotation rate (milli-arc-seconds p/yr)
	  1.6290	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF1996_GDA2020 : public _itrf1996_to_gda2020_<T, U>
{
public:
};

// ITRF1997 -> GDA2020
template <class T, class U>
struct _itrf1997_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf1997_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _itrf1997_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf1997_to_gda2020_<T, U>::transformationParameters[14] =
{
	-176.680,	// x translation (millimetres)
	 -29.130,	// y translation (millimetres)
	 226.990,	// z translation (millimetres)
	 -3.1170,	// scale (ppb)
	  1.3427,	// x rotation (milli-arc-seconds)
	  6.1880,	// y rotation (milli-arc-seconds)
	  3.9809,	// z rotation (milli-arc-seconds)

	  -8.600,	// x translation rate (millimetres p/yr)
	   0.360,	// y translation rate (millimetres p/yr)
	  11.250,	// z translation rate (millimetres p/yr)
	  0.0070,	// scale rate (ppb p/yr)
	  1.6394,	// x rotation rate (milli-arc-seconds p/yr)
	  1.5198,	// y rotation rate (milli-arc-seconds p/yr)
	  1.3801	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF1997_GDA2020 : public _itrf1997_to_gda2020_<T, U>
{
public:
};

// ITRF 2000 -> GDA2020
template <class T, class U>
struct _itrf2000_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _itrf2000_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2000_to_gda2020_<T, U>::transformationParameters[14] =
{
    -105.520,	// x translation (millimetres)
      51.580,	// y translation (millimetres)
     231.680,	// z translation (millimetres)
      3.5500,	// scale (ppb)
      4.2175,	// x rotation (milli-arc-seconds)
      6.3941,	// y rotation (milli-arc-seconds)
      0.8617,	// z rotation (milli-arc-seconds)
    
      -4.660,   // x translation rate (millimetres p/yr) 
       3.550,	// y translation rate (millimetres p/yr)
      11.240,	// z translation rate (millimetres p/yr)
      0.2490,	// scale rate (ppb p/yr)
      1.7454,	// x rotation rate (milli-arc-seconds p/yr)
      1.4868,	// y rotation rate (milli-arc-seconds p/yr)
      1.2240	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2000_GDA2020 : public _itrf2000_to_gda2020_<T, U>
{
public:
};

// ITRF 2005 -> GDA2020
template <class T, class U>
struct _itrf2005_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2005_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _itrf2005_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2005_to_gda2020_<T, U>::transformationParameters[14] =
{
      40.320,	// x translation (millimetres)
     -33.850,	// y translation (millimetres)
     -16.720,	// z translation (millimetres)
      4.2860,	// scale (ppb)
     -1.2893,	// x rotation (milli-arc-seconds)
     -0.8492,	// y rotation (milli-arc-seconds)
     -0.3342,	// z rotation (milli-arc-seconds)
    
       2.250,	// x translation rate (millimetres p/yr) 
      -0.620,	// y translation rate (millimetres p/yr)
      -0.560,	// z translation rate (millimetres p/yr)
      0.2940,	// scale rate (ppb p/yr)
      1.4707,	// x rotation rate (milli-arc-seconds p/yr)
      1.1443,	// y rotation rate (milli-arc-seconds p/yr)
      1.1701	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2005_GDA2020 : public _itrf2005_to_gda2020_<T, U>
{
public:
};

// ITRF 2008 -> GDA2020
template <class T, class U>
struct _itrf2008_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _itrf2008_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2008_to_gda2020_<T, U>::transformationParameters[14] =
{
      13.790,	// x translation (millimetres)
       4.550,	// y translation (millimetres)
      15.220,	// z translation (millimetres)
      2.5500,	// scale (ppb)
      0.2808,	// x rotation (milli-arc-seconds)
      0.2677,	// y rotation (milli-arc-seconds)
     -0.4638,	// z rotation (milli-arc-seconds)
      
       1.420,   // x translation rate (millimetres p/yr) 
       1.340,	// y translation rate (millimetres p/yr)
       0.900,	// z translation rate (millimetres p/yr)
      0.1090,	// scale rate (ppb p/yr)
      1.5461,	// x rotation rate (milli-arc-seconds p/yr)
      1.1820,	// y rotation rate (milli-arc-seconds p/yr)
      1.1551	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2008_GDA2020 : public _itrf2008_to_gda2020_<T, U>
{
public:
};

////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// GDA94 <-> GDA2020
// PCG In-confidence PowerPoint (Perth PGC FTF March 2016)
// file://C:\Data\GEODESY\ICSM\DATUM\2016-03-09-pcg-perth\GDA94toGDA2020Transformation.pptx
//
// Replaced by ICSM Release note (March 2017)
// 'file://C:\onedrive\GEODESY\ICSM\DATUM\GDA2020%20media\GDA2020%20Release%20Note.docx'

// GDA94 -> GDA2020
template <class T, class U>
struct _gda94_to_gda2020_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _gda94_to_gda2020_<T, U>::reference_frame = GDA2020_i;

template <class T, class U>
const T _gda94_to_gda2020_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _gda94_to_gda2020_<T, U>::transformationParameters[14] =
{
	   61.55,	// x translation (millimetres)
	  -10.87,	// y translation (millimetres)
	  -40.19,	// z translation (millimetres)
	  -9.994,	// scale (ppb)
	-39.4924,	// x rotation (milli-arc-seconds)
	-32.7221,	// y rotation (milli-arc-seconds)
	-32.8979,	// z rotation (milli-arc-seconds)

		 0.0,	// x translation rate (millimetres p/yr)
		 0.0,	// y translation rate (millimetres p/yr)
		 0.0,	// z translation rate (millimetres p/yr)
		 0.0,	// scale rate (ppb p/yr)
		 0.0,	// x rotation rate (milli-arc-seconds p/yr)
		 0.0,	// y rotation rate (milli-arc-seconds p/yr)
		 0.0	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class GDA94_GDA2020 : public _gda94_to_gda2020_<T, U>
{
public:
};

// ITRF2014 at epoch 2020.0 -> GDA94.  
// Effectively, these are the same as GDA2020 -> GDA94 parameters, plus
// the rotation rates from the Australian Plate Motion Model.
template <class T, class U>
struct _itrf2014_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2014_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf2014_to_gda94_<T, U>::reference_epoch = 2020.0;

template <class T, class U>
const T _itrf2014_to_gda94_<T, U>::transformationParameters[14] =
{
	  -61.55,	// x translation (millimetres)
	   10.87,	// y translation (millimetres)
	   40.19,	// z translation (millimetres)
	   9.994,	// scale (ppb)
	 39.4924,	// x rotation (milli-arc-seconds)
	 32.7221,	// y rotation (milli-arc-seconds)
	 32.8979,	// z rotation (milli-arc-seconds)

		 0.0,	// x translation rate (millimetres p/yr)
		 0.0,	// y translation rate (millimetres p/yr)
		 0.0,	// z translation rate (millimetres p/yr)
		 0.0,	// scale rate (ppb p/yr)
	 1.50379,	// x rotation rate (milli-arc-seconds p/yr)
	 1.18346,	// y rotation rate (milli-arc-seconds p/yr)
	 1.20716	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2014_GDA94 : public _itrf2014_to_gda94_<T, U>
{
public:
};
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// GDA94 <-> ITRF... (Dawson and Woods)
// http://www.ga.gov.au/webtemp/image_cache/GA19050.pdf

// ITRF 2008 -> GDA94
template <class T, class U>
struct _itrf2008_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2008_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf2008_to_gda94_<T, U>::reference_epoch = 1994.0;

template <class T, class U>
const T _itrf2008_to_gda94_<T, U>::transformationParameters[14] =
{
	  -84.68,	// x translation (millimetres)
	  -19.42,	// y translation (millimetres)
	   32.01,	// z translation (millimetres)
	  9.7100,	// scale (ppb)
	 -0.4254,	// x rotation (milli-arc-seconds)
	  2.2578,	// y rotation (milli-arc-seconds)
	  2.4015,	// z rotation (milli-arc-seconds)

	   1.420,	// x translation rate (millimetres p/yr)
	   1.340,	// y translation rate (millimetres p/yr)
	   0.900,	// z translation rate (millimetres p/yr)
	  0.1090,	// scale rate (ppb p/yr)
	  1.5461,	// x rotation rate (milli-arc-seconds p/yr)
	  1.1820,	// y rotation rate (milli-arc-seconds p/yr)
	  1.1551 	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2008_GDA94 : public _itrf2008_to_gda94_<T, U>
{
public:
};

// ITRF 2005 -> GDA94
template <class T, class U>
struct _itrf2005_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2005_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf2005_to_gda94_<T, U>::reference_epoch = 1994.0;

template <class T, class U>
const T _itrf2005_to_gda94_<T, U>::transformationParameters[14] =
{
	  -79.73,	// x translation (millimetres)
	   -6.86,	// y translation (millimetres)
	   38.03,	// z translation (millimetres)
	  6.6360,	// scale (ppb)
	 -0.0351,	// x rotation (milli-arc-seconds)
	  2.1211,	// y rotation (milli-arc-seconds)
	  2.1411,	// z rotation (milli-arc-seconds)

	    2.25,	// x translation rate (millimetres p/yr)
	   -0.62,	// y translation rate (millimetres p/yr)
	   -0.56,	// z translation rate (millimetres p/yr)
	  0.2940,	// scale rate (ppb p/yr)
	  1.4707,	// x rotation rate (milli-arc-seconds p/yr)
	  1.1443,	// y rotation rate (milli-arc-seconds p/yr)
	  1.1701 	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2005_GDA94 : public _itrf2005_to_gda94_<T, U>
{
public:
};

// ITRF 2000 -> GDA94
template <class T, class U>
struct _itrf2000_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf2000_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf2000_to_gda94_<T, U>::reference_epoch = 1994.0;

template <class T, class U>
const T _itrf2000_to_gda94_<T, U>::transformationParameters[14] =
{
	  -45.91,	// x translation (millimetres)
	  -29.85,	// y translation (millimetres)
	  -20.37,	// z translation (millimetres)
	  7.0700,	// scale (ppb)
	 -1.6705,	// x rotation (milli-arc-seconds)
	  0.4594,	// y rotation (milli-arc-seconds)
	  1.9356,	// z rotation (milli-arc-seconds)

	   -4.66,	// x translation rate (millimetres p/yr)
	    3.55,	// y translation rate (millimetres p/yr)
	   11.24,	// z translation rate (millimetres p/yr)
	  0.2490,	// scale rate (ppb p/yr)
	  1.7454,	// x rotation rate (milli-arc-seconds p/yr)
	  1.4868,	// y rotation rate (milli-arc-seconds p/yr)
	  1.2240 	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF2000_GDA94 : public _itrf2000_to_gda94_<T, U>
{
public:
};

// ITRF 1997 -> GDA94
template <class T, class U>
struct _itrf1997_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf1997_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf1997_to_gda94_<T, U>::reference_epoch = 1994.0;

template <class T, class U>
const T _itrf1997_to_gda94_<T, U>::transformationParameters[14] =
{
	  -14.63,	// x translation (millimetres)
	  -27.62,	// y translation (millimetres)
	  -25.32,	// z translation (millimetres)
	  6.6950,	// scale (ppb)
	 -1.7893,	// x rotation (milli-arc-seconds)
	 -0.6047,	// y rotation (milli-arc-seconds)
	  0.9962,	// z rotation (milli-arc-seconds)

	   -8.60,	// x translation rate (millimetres p/yr)
	    0.36,	// y translation rate (millimetres p/yr)
	   11.25,	// z translation rate (millimetres p/yr)
	  0.0070,	// scale rate (ppb p/yr)
	  1.6394,	// x rotation rate (milli-arc-seconds p/yr)
	  1.5198,	// y rotation rate (milli-arc-seconds p/yr)
	  1.3801	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF1997_GDA94 : public _itrf1997_to_gda94_<T, U>
{
public:
};

// ITRF 1996 -> GDA94
template <class T, class U>
struct _itrf1996_to_gda94_
{
	static const U reference_frame;
	static const T reference_epoch;
	static const T transformationParameters[14];
};

template <class T, class U>
const U _itrf1996_to_gda94_<T, U>::reference_frame = GDA94_i;

template <class T, class U>
const T _itrf1996_to_gda94_<T, U>::reference_epoch = 1994.0;

template <class T, class U>
const T _itrf1996_to_gda94_<T, U>::transformationParameters[14] =
{
	   24.54,	// x translation (millimetres)
	  -36.43,	// y translation (millimetres)
	  -68.12,	// z translation (millimetres)
	  6.9010,	// scale (ppb)
	 -2.7359,	// x rotation (milli-arc-seconds)
	 -2.0431,	// y rotation (milli-arc-seconds)
	  0.3731,	// z rotation (milli-arc-seconds)

	  -21.80,	// x translation rate (millimetres p/yr)
	    4.71,	// y translation rate (millimetres p/yr)
	   26.27,	// z translation rate (millimetres p/yr)
	  0.3880,	// scale rate (ppb p/yr)
	  2.0203,	// x rotation rate (milli-arc-seconds p/yr)
	  2.1735,	// y rotation rate (milli-arc-seconds p/yr)
	  1.6290 	// z rotation rate (milli-arc-seconds p/yr)
};

template <class T, class U>
class ITRF1996_GDA94 : public _itrf1996_to_gda94_<T, U>
{
public:
};




}	// namespace datum_parameters
}	// namespace dynadjust

#endif /* DNATRANSFORM_PARAM_H_ */
