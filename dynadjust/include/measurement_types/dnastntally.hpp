//============================================================================
// Name         : dnastntally.hpp
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
// Description  : Interface for the StnTally class
//============================================================================

#ifndef DNASTNTALLY_H
#define DNASTNTALLY_H

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/measurement_types/dnastation.hpp>

using namespace std;

namespace dynadjust { namespace measurements {

class StnTally {

public:
	StnTally();
	void initialise();

	StnTally& operator+=(const StnTally& rhs);
	StnTally& operator-=(const StnTally& rhs);
	const StnTally operator+(const StnTally& rhs) const;
	const StnTally operator-(const StnTally& rhs) const;
	
	UINT32 TotalCount();
	void addstation(const string& constraint);
	void removestation(const string& constraint);
	void coutSummary(ostream &os, const string& title);
	void CreateTally(const vdnaStnPtr& vStations);

	UINT32 CCC, FFF, CCF, CFF, FFC, FCC, CFC, FCF;
};
	

	
}	// namespace measurements
}	// namespace dynadjust

#endif // ifndef DNASTNTALLY_H
