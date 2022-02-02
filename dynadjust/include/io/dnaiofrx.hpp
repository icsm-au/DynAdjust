//============================================================================
// Name         : dnaiofrx.hpp
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
// Description  : Reference frame substitutions file
//============================================================================

#ifndef DNAIOFRX_H_
#define DNAIOFRX_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>
#include <include/functions/dnatemplatedatetimefuncs.hpp>

namespace dynadjust {
namespace iostreams {

/////////////////////////////////////////////////////////////
// Custom type to manage frame substitutions
template <class T1 = string, class T2 = UINT32, class T3 = double>
struct frame_substitutions_t
{
	//frame_substitutions_t()
	//	: frame_name(""), frame_epsg(0), frame_desc("")
	//	, substitute_name(""), substitute_epsg(0)
	//	, alignment_epoch(date_time::min_date_time), from_epoch(date_time::min_date_time), to_epoch(date_time::min_date_time)
	//{
	//	memset(parameters_, '\0', sizeof(parameters_));
	//}

	void initialise() {
		frame_name = "";
		frame_epsg = 0;
		frame_desc = "";
		substitute_name = "";
		substitute_epsg = 0;
		//alignment_epoch = date_time::min_date_time;
		//from_epoch = date_time::min_date_time;
		//to_epoch = date_time::min_date_time;
		memset(parameters_, '\0', sizeof(parameters_));
	}

    T1 frame_name;              // frame name
    T2 frame_epsg;              // frame epsg code
    T1 frame_desc;              // frame description
    T1 substitute_name;         // substitution frame name
    T2 substitute_epsg;         // substitution frame epsg code
    boost::gregorian::date alignment_epoch;       // substitution alignment epoch
    boost::gregorian::date from_epoch;            // start date of substitution
    boost::gregorian::date to_epoch;              // end date of substitution
    T3 parameters_[7];		    // transformation parameters (if any)
};

typedef frame_substitutions_t<string, UINT32, double> frame_substitutions;
typedef vector<frame_substitutions> v_frame_substitutions;
typedef v_frame_substitutions::iterator it_frame_substitutions;

// Used to sort site tuples by WGS84 starting epoch
template <typename T>
class CompareSubstitutionByEpoch {
public:
	bool operator()(const T& left, const T& right) {
		// if the starting epochs are equal
		if (left.from_epoch == right.from_epoch)
			// sort on ending epochs
			return left.to_epoch < right.to_epoch;
		// sort on starting epoch
		return left.from_epoch < right.from_epoch;
	}
};

template<typename T, typename S>
struct CompareSubstituteOnEPSG
{
	//public:
	bool operator()(const T& lhs, const S& rhs) const {
		return pair_firstless(lhs.frame_name, rhs);
	}
	bool operator()(const S& lhs, const T& rhs) const {
		return pair_firstless(lhs, rhs.frame_name);
	}
	//private:
	bool pair_firstless(const S& s1, const S& s2) const {
		return s1 < s2;
	}
};


// tweak the binary search so it returns the iterator of the object found
template<typename T, typename S>
T binary_search_substitution(T begin, T end, S value)
{
	T i = lower_bound(begin, end, value, CompareSubstituteOnEPSG<frame_substitutions, string>());
	if (i != end && i->frame_name == value)
		return i;
	else
		return end;
}
/////////////////////////////////////////////////////////////

class dna_io_frx : public dna_io_base
{
public:
	dna_io_frx(void) {};
	dna_io_frx(const dna_io_frx&) {};
	virtual ~dna_io_frx(void) {};

	dna_io_frx& operator=(const dna_io_frx& rhs);

    void load_frx_file(const string& frx_filename, v_frame_substitutions& frame_subs);
	

protected:
	
};



}	// namespace measurements
}	// namespace dynadjust

#endif
