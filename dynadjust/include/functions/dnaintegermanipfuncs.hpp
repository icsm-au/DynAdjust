//============================================================================
// Name         : dnaintegermanipfuncs.hpp
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
// Description  : Integer (signed and unsigned) manipulation functions
//============================================================================

#ifndef DNAINTEGERFUNCS_H_
#define DNAINTEGERFUNCS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <vector>

#include <include/config/dnatypes.hpp>

template <class T>
void initialiseIncrementingIntegerVector(std::vector<T>& vt, const T& size)
{
	vt.resize(size);
	T t(0);
	for_each(vt.begin(), vt.end(),
		[&t](T& t_){
			t_ = t++;
	});
}

template <class T>
void initialiseIncrementingIntegerVector(std::vector<T>* vt, const T& size)
{
	initialiseIncrementingIntegerVector(*vt, size);
}

template <class T>
std::vector<T> createIncrementingIntegerVector(const T& size)
{
	std::vector<T> vt(size);
	initialiseIncrementingIntegerVector(vt, size);
	return vt;
}

#endif /* DNAINTEGERFUNCS_H_ */
