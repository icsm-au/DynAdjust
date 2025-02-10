//============================================================================
// Name         : dnaparallelfuncs.hpp
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
// Description  : Basic template functions to help parallel processing
//============================================================================

#ifndef DNAPARALLELFUNCS_H_
#define DNAPARALLELFUNCS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <algorithm>
#include <functional>

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <include/config/dnatypes.hpp>

template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
	const UINT32 length(std::distance(first, last));
	if (!length)
		return;
	const UINT32 min_per_thread=25;

	if (length < (2 * min_per_thread))
	{
		for_each(first, last, f);
	}
	else
	{
		Iterator const mid_point(first + length / 2);
		boost::future<void> first_half(boost::async(&parallel_for_each<Iterator, Func>, first, mid_point, f));
		parallel_for_each(mid_point, last, f);
		first_half.get();
	}
}


#endif /* DNAPARALLELFUNCS_H_ */
