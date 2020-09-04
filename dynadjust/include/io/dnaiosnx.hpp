//============================================================================
// Name         : dnaiosnx.hpp
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
// Description  : DynAdjust SINEX file io operations
//============================================================================

#ifndef DNAIOSNX_H_
#define DNAIOSNX_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/io/dnaiobase.hpp>
#include <include/math/dnamatrix_contiguous.hpp>
#include <include/measurement_types/dnastntally.hpp>
#include <include/measurement_types/dnastation.hpp>
#include <include/measurement_types/dnameasurement.hpp>
#include <include/parameters/dnadatum.hpp>
#include <boost/date_time/local_time/local_time.hpp>

using namespace dynadjust::math;
using namespace dynadjust::datum_parameters;
using namespace dynadjust::measurements;

using namespace boost::local_time;

namespace dynadjust {
namespace iostreams {

typedef enum _SINEX_WARN_TYPE_
{
	excessive_name_chars = 0
} SINEX_WARN_TYPE;


const UINT32 DISCONT_TIME_IMMEMORIAL = 1900;

/////////////////////////////////////////////////////////////
// Custom struct to manage occurrences of a station in a SINEX file
//T1 file_index;
//T1 solution_id;
//T2 site_name;
//T2 formatted_name;
//T2 formatted_date;
//T3 last_occurrence;
template <class T1=UINT32, class T2=string, class T3=bool> 
struct site_id_tuple_t
{
	T1 file_index;
	T1 solution_id;
	T2 site_name;
	T2 formatted_name;
	T2 formatted_date;
	T3 last_occurrence;

	site_id_tuple_t()
		: file_index(0), solution_id(0)
		, site_name(""), formatted_name(""), formatted_date(""), last_occurrence(false) {}
	site_id_tuple_t(const T1& index, const T1& solution,
		const T2& name, const T2& formattedname, const T2& formatteddate, const T3& lastoccurrence) 
		: file_index(index), solution_id(solution)
		, site_name(name), formatted_name(formattedname), formatted_date(formatteddate), last_occurrence(lastoccurrence) {}
	
	template <class A, class B, class C>
	site_id_tuple_t (const site_id_tuple_t<A, B, C> &s) 
		: file_index(s.file_index)
		, solution_id(s.solution_id)
		, site_name(s.site_name)
		, formatted_date(s.formatted_date)
		, formatted_name(s.formatted_name)
		, last_occurrence(s.last_occurrence) {}
};

// Now create types for the site_id variable, container and iterator
typedef site_id_tuple_t<UINT32, string, bool> site_id;
typedef vector<site_id> v_site_id_tuple, *pv_v_site_id_tuple;
typedef v_site_id_tuple::iterator _it_vsite_id_tuple;

// Used to sort site tuples by file order
// Can be used to sort site_id elements or discontinuity elements
template <typename T>
class CompareSiteTuples {
public:
	bool operator()(const T& left, const T& right) {
		// sort on file order
		return left.file_index < right.file_index;
	}
};

// Used to sort site tuples by original name
// Can be used to sort site_id elements or discontinuity elements
template <typename T>
class CompareSiteTuplesByName {
public:
	bool operator()(const T& left, const T& right) {
		// if the station names are equal
		if (equals(left.site_name, right.site_name))
			// sort on file order
			return left.file_index < right.file_index;
		// sort on station name
		return left.site_name < right.site_name;
	}
};

/////////////////////////////////////////////////////////////
// Custom struct to manage site discontinuities
template <class T1=UINT32, class T2=string, class T3=date, class T4=bool> 
struct discontinuity_tuple_t
{
	T1 file_index;
	T1 solution_id;
	T2 site_name;
	T3 date_start;
	T3 date_end;
	T4 discontinuity_exists;

	discontinuity_tuple_t()
		: file_index(0), solution_id(0), site_name("")
		, date_start(date(DISCONT_TIME_IMMEMORIAL, 1, 1)), date_end(date(DISCONT_TIME_IMMEMORIAL, 1, 1)), discontinuity_exists(false) {}
	discontinuity_tuple_t(const T1& index, const T1& solution, const T2& name, 
		const T3& from, const T3& to, const T4& discontinuity) 
		: file_index(index), solution_id(solution), site_name(name)
		, date_start(from), date_end(to), discontinuity_exists(discontinuity) {}

	template <class A, class B, class C, class D>
	discontinuity_tuple_t (const discontinuity_tuple_t<A, B, C, D> &d) 
		: file_index(d.file_index)
		, solution_id(d.solution_id)
		, site_name(d.site_name)
		, date_start(d.date_start)
		, date_end(d.date_end)
		, discontinuity_exists(d.discontinuity_exists) {}
};

// Now create types for the site_id variable, container and iterator
typedef discontinuity_tuple_t<UINT32, string, date, bool> discontinuity_tuple;
typedef vector<discontinuity_tuple> v_discontinuity_tuple, *pv_v_discontinuity_tuple;
typedef v_discontinuity_tuple::iterator _it_vdiscontinuity_tuple;

template<typename T, typename S, typename D>
bool rename_discont_station(T& begin, S& site_name, D& site_date, S& site_renamed)
{
	UINT32 doy, year;
	
	// find the next occurrence of this site
	while (equals(site_name, begin->site_name))
	{
		// Test if the start epoch of this site is within this discontinuity window
		if (site_date >= begin->date_start &&
			site_date < begin->date_end)
		{
			stringstream ss;
			doy = begin->date_start.day_of_year();
			year = begin->date_start.year();
			ss << site_name << "_";

			if (year == DISCONT_TIME_IMMEMORIAL)
			{
				// format using the start epoch
				// year
				ss << dateYear<UINT32>(site_date);
				// doy
				ss << dateDOY<UINT32>(site_date);
			}
			else
			{
				// format using the discontinuity date
				ss << year;
				if (doy < 10)
					ss << "0";
				if (doy < 100)
					ss << "0";
				ss << doy;
			}

			site_renamed = ss.str();

			return true;
		}

		begin++;
	}

	return false;
}

template<typename T, typename S>
struct CompareDiscontinuityOnSite
{
//public:
	bool operator()(const T& lhs, const S& rhs) const {
		return pair_firstless(lhs.site_name, rhs);
	}
	bool operator()(const S& lhs, const T& rhs) const {
		return pair_firstless(lhs, rhs.site_name);
	}
//private:
	bool pair_firstless(const S& s1, const S& s2) const {
		return s1 < s2;
	}
};


// tweak the binary search so it returns the iterator of the object found
template<typename T, typename S>
T binary_search_discontinuity_site(T begin, T end, S value)
{
	T i = lower_bound(begin, end, value, CompareDiscontinuityOnSite<discontinuity_tuple, string>());
	if (i != end && i->site_name == value)
		return i;
	else
		return end;
}


typedef enum _DATE_TERMINAL_TYPE_
{
	date_from = 0,
	date_to = 1
} DATE_TERMINAL_TYPE;
	

typedef enum _DATE_FORMAT_TYPE_
{
	yy_doy = 0,
	yyyy_doy = 1,
	doy_yy = 2,
	doy_yyyy = 3
} DATE_FORMAT_TYPE;

class dna_io_snx : public dna_io_base
{
public:
	dna_io_snx(void) {};
	dna_io_snx(const dna_io_snx&) {};
	virtual ~dna_io_snx(void) {};

	dna_io_snx& operator=(const dna_io_snx& rhs);

	stringstream parse_date_from_string(const string& date_str, DATE_FORMAT_TYPE date_type, DATE_TERMINAL_TYPE terminal_type, const string& separator);
	stringstream parse_date_from_yy_doy(const UINT32& yy, const UINT32& doy, DATE_FORMAT_TYPE date_type, const string& separator);

	void parse_sinex(std::ifstream** snx_file, const string& fileName, vdnaStnPtr* vStations, PUINT32 stnCount, 
					vdnaMsrPtr* vMeasurements, PUINT32 msrCount, PUINT32 clusterID, string* success_msg,
					StnTally& parsestn_tally, MsrTally& parsemsr_tally, UINT32& fileOrder,
					CDnaDatum& datum, bool applyDiscontinuities, 
					v_discontinuity_tuple* stn_discontinuities_, bool& m_discontsSortedbyName,
					UINT32& lineNo, UINT32& columnNo, _PARSE_STATUS_& parseStatus);
	
	void parse_discontinuity_file(std::ifstream* snx_file, const string& fileName,
		v_discontinuity_tuple* stn_discontinuities_, bool& m_discontsSortedbyName,
		UINT32& lineNo, UINT32& columnNo, _PARSE_STATUS_& parseStatus);

	void serialise_sinex(std::ofstream* snx_file, pvstn_t bstRecords, pvmsr_t bmsRecords,
					binary_file_meta_t& bst_meta, binary_file_meta_t& bms_meta,
					matrix_2d* estimates, matrix_2d* variances, const project_settings& p,
					UINT32& measurementParams, UINT32& unknownParams, double& sigmaZero,
					uint32_uint32_map* blockStationsMap, vUINT32* blockStations_,
					const UINT32& blockCount, const UINT32& block,
					const CDnaDatum* datum);

	inline bool warnings_exist() { return !warningMessages_.empty(); }
	
	void print_warnings(std::ofstream* warning_file, const string& fileName);

protected:

	// Read functions
	void parse_sinex_data(std::ifstream** snx_file, const string& fileName, vdnaStnPtr* vStations, PUINT32 stnCount, 
			vdnaMsrPtr* vMeasurements, PUINT32 msrCount, PUINT32 clusterID, string* success_msg,
			StnTally& parsestn_tally, MsrTally& parsemsr_tally, CDnaDatum& datum, 
			v_discontinuity_tuple* stn_discontinuities_, bool& m_discontsSortedbyName,
			UINT32& fileOrder, UINT32& lineNo, UINT32& columnNo);
	
	bool parse_sinex_header(std::ifstream** snx_file, CDnaDatum& datum, UINT32& lineNo, UINT32& columnNo);
	
	void parse_sinex_block(std::ifstream** snx_file, const char* sinexRec, vdnaStnPtr* vStations, PUINT32 stnCount, 
			vdnaMsrPtr* vMeasurements, PUINT32 msrCount, PUINT32 clusterID, string* success_msg,
			StnTally& parsestn_tally, MsrTally& parsemsr_tally, CDnaDatum& datum, v_discontinuity_tuple* stn_discontinuities_,
			UINT32& fileOrder, UINT32& lineNo, UINT32& columnNo);
	
	void parse_sinex_discontinuities(std::ifstream* snx_file, 
			v_discontinuity_tuple* stn_discontinuities_, bool& m_discontsSortedbyName,
			UINT32& lineNo, UINT32& columnNo);

	void parse_sinex_sites(std::ifstream** snx_file, UINT32& lineNo, UINT32& columnNo);
	void parse_sinex_epochs(std::ifstream** snx_file, UINT32& lineNo, UINT32& columnNo);
	void reduce_sinex_sites();

	void parse_sinex_stn(std::ifstream** snx_file, const char* sinexRec, vdnaStnPtr* vStations, PUINT32 stnCount,
			StnTally& parsestn_tally, CDnaDatum& datum, 
			v_discontinuity_tuple* stn_discontinuities_, UINT32& fileOrder, UINT32& lineNo, UINT32& columnNo);
	
	void parse_sinex_msr(std::ifstream** snx_file, const char* sinexRec, vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, PUINT32 clusterID, PUINT32 msrCount,
			MsrTally& parsemsr_tally, CDnaDatum& datum, v_discontinuity_tuple* stn_discontinuities_, UINT32& lineNo);

	void format_station_names(v_discontinuity_tuple* stn_discontinuities, bool& m_discontsSortedbyName,
		vdnaStnPtr* vStations, PUINT32 stnCount, 
		vdnaMsrPtr* vMeasurements, PUINT32 msrCount);

	// Write functions
	void serialise_meta(std::ofstream* snx_file, 
			binary_file_meta_t& bst_meta, binary_file_meta_t& bms_meta,
			const project_settings& p, const CDnaDatum* datum);
	void serialise_statistics(std::ofstream* snx_file);
	void serialise_site_id(std::ofstream* snx_file, pvstn_t bstRecords);
	void serialise_solution_estimates(std::ofstream* snx_file, pvstn_t bstRecords, pvmsr_t bmsRecords,
			matrix_2d* estimates, matrix_2d* variances, const CDnaDatum* datum);
	void serialise_solution_variances(std::ofstream* snx_file, pvstn_t bstRecords, pvmsr_t bmsRecords,
			matrix_2d* variances);
	void print_line(std::ofstream* snx_file);

	//string format_exponent(string value);
	void print_matrix_index(std::ofstream* snx_file, const UINT32& row, const UINT32& col);
	void add_warning(const string& message, SINEX_WARN_TYPE warning);

	UINT32					blockCount_;
	UINT32					block_;
	UINT32					measurementParams_;
	UINT32					unknownParams_;
	double					sigmaZero_;

	uint32_uint32_map*		blockStationsMap_;
	vUINT32*				blockStations_;

	vector<string>			warningMessages_;

	date					averageEpoch_;

	UINT32					uniqueStationCount_;
	bool					containsVelocities_;
	bool					containsDiscontinuities_;
	bool					applyDiscontinuities_;
	bool					siteIDsRead_;
	bool					solutionEpochsRead_;

	v_site_id_tuple			siteOccurrence_;
};

}	// namespace dnaiostreams
}	// namespace dynadjust

#endif
