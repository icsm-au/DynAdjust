//============================================================================
// Name         : dnaplatemotionmodels.hpp
// Author       : Roger Fraser
// Contributors : 
// Version      : 1.00
// Copyright    : Copyright 2018 Geoscience Australia
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
// Description  : Declaration/definition of plate motion model parameter in form of
//				  Helmert rotations
//============================================================================

#ifndef DNAPMM_PARAM_H_
#define DNAPMM_PARAM_H_

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
namespace pmm_parameters {


// AU
template <class T, class S>
struct _au_plate_model_ {
	static const S plate_id;			// two-character plate id
	static const T plate_rotations[3];	// milli-arc-seconds p/yr
};

template <class T, class S>
const S _au_plate_model_<T, S>::plate_id = "AU";

template <class T, class S>
const T _au_plate_model_<T, S>::plate_rotations[3] = {
	// Euler parameters:
	//  - pole latitude:  32.2447
	// 	- pole longitude: 38.2022
	//  - rotation rate:   0.6285
	1.50379,		// x rotation rate
	1.18346,		// y rotation rate
	1.20716			// z rotation rate
};

template <class T, class S>
class AU_PLATE_MOTION_MODEL : public _au_plate_model_<T, S> {
public:
};

////////////////////////////////////////////////////////////////////////////////////


}	// namespace datum_parameters
}	// namespace dynadjust

#endif /* DNAPMM_PARAM_H_ */
