//============================================================================
// Name         : dnatypeb.hpp
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
// Description  : Type B uncertainty file io and helps
//============================================================================

#ifndef DNAIOTYPEB_H_
#define DNAIOTYPEB_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/config/dnatypes.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/math/dnamatrix_contiguous.hpp>

using namespace std;
using namespace boost;

using namespace dynadjust::math;

namespace dynadjust {
namespace iostreams {


/////////////////////////////////////////////////////////////
// Custom type to manage cartesian plate motion model parameters
template <class T1 = UINT32, class T2 = string, class T3 = matrix_2d, class T4 = vstring>
struct type_b_uncertainty_t
{
	T1 station_id;
	T3 type_b;

	type_b_uncertainty_t()
		: station_id(0) {
		type_b.redim(3, 3);
	}
	type_b_uncertainty_t(const T1& stn, const T3& typeb_local)
		: station_id(stn)
	{
		type_b.redim(3, 3);
		set_typeb_values(typeb_local);
	}
	
	void set_station_id(const T1& stn=0) { station_id =stn; }

	void set_station_id_str(const T2& stn) { station_id = LongFromString<T1>(stn); }

	void set_typeb_values(const T4& typeb_local) 
	{ 
		// 3 dimensions (east, north, up)
		if (typeb_local.size() > 2) {
			type_b.put(0, 0, DoubleFromString<double>(typeb_local.at(0)));
			type_b.put(1, 1, DoubleFromString<double>(typeb_local.at(1)));
			type_b.put(2, 2, DoubleFromString<double>(typeb_local.at(2)));
		}
		// 2 dimensions (east, north)
		else if (typeb_local.size() > 1) {
			type_b.put(0, 0, DoubleFromString<double>(typeb_local.at(0)));
			type_b.put(1, 1, DoubleFromString<double>(typeb_local.at(1)));
		}
		// 1 dimension (up)
		else if (typeb_local.size() > 0) {
			type_b.put(2, 2, DoubleFromString<double>(typeb_local.at(2)));
		}
	}
	void set_typeb_values_all(const T4& typeb_local)
	{
		UINT32 i(0);
		if (typeb_local.at(i).empty())
			type_b.put(i, i, 0.);
		else
			type_b.put(i, i, DoubleFromString<double>(typeb_local.at(i)));
		
		i++;
		if (typeb_local.at(i).empty())
			type_b.put(i, i, 0.);
		else
			type_b.put(i, i, DoubleFromString<double>(typeb_local.at(i)));

		i++;
		if (typeb_local.at(i).empty())
			type_b.put(i, i, 0.);
		else
			type_b.put(i, i, DoubleFromString<double>(typeb_local.at(i)));

	}

};

typedef type_b_uncertainty_t<UINT32, string, matrix_2d, vstring> type_b_uncertainty;
typedef vector<type_b_uncertainty> v_type_b_uncertainty;
typedef v_type_b_uncertainty::iterator it_type_b_uncertainty;
/////////////////////////////////////////////////////////////

class dna_io_tbu : public dna_io_base
{
public:
	dna_io_tbu(void) {};
	dna_io_tbu(const dna_io_tbu& tbu) : dna_io_base(tbu) {};
	virtual ~dna_io_tbu(void) {};

	dna_io_tbu& operator=(const dna_io_tbu& rhs);

	void read_tbu_header(std::ifstream* ptr, string& version, INPUT_DATA_TYPE& idt);

	void load_tbu_file(const string& tbu_filename, v_type_b_uncertainty& type_b_uncertainties);

protected:

};

}	// namespace iostreams
}	// namespace dynadjust

#endif /* DNAIOTYPEB_H_ */
