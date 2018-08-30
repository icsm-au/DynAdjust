//============================================================================
// Name         : dnaoptions-helper.hpp
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
// Description  : DynAdjust options helper file
//============================================================================

#ifndef DNAOPTIONS_HELPER_HPP
#define DNAOPTIONS_HELPER_HPP

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnaoptions.hpp>
#include <include/config/dnaoptions-interface.hpp>

template <typename S, typename U>
S adjustmentMode(const U& mode)
{
	switch (mode)
	{
	case SimultaneousMode:
		return MODE_SIMULTANEOUS;
	case PhasedMode:
		return MODE_PHASED;
	case Phased_Block_1Mode:
		return MODE_PHASED_BLOCK1;
	case SimulationMode:
		return MODE_SIMULATION;
	}
	return " ";
}




#endif  // DNAOPTIONS_HPP
