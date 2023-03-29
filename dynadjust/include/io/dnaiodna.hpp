//============================================================================
// Name         : dnaiodna.hpp
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
// Description  : DNA file io helper
//============================================================================

#ifndef DNAIODNA_H_
#define DNAIODNA_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <include/config/dnatypes.hpp>
#include <include/config/dnaconsts-iostream.hpp>
#include <include/io/dnaiobase.hpp>
#include <include/io/dnaiodnatypes.hpp>
#include <include/functions/dnaintegermanipfuncs.hpp>
#include <include/functions/dnatemplatestnmsrfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnatemplatefuncs.hpp>
#include <include/measurement_types/dnastation.hpp>
#include <include/measurement_types/dnameasurement.hpp>
#include <include/parameters/dnadatum.hpp>

namespace dynadjust {
namespace iostreams {

class dna_io_dna : public dna_io_base
{
public:
	dna_io_dna(void)
		: pv_msr_db_map_(0)
		, m_databaseIDsSet_(false) {};
	dna_io_dna(const dna_io_dna& dna) 
		: dna_io_base(dna) 
		, pv_msr_db_map_(0)
		, m_databaseIDsSet_(false) {};	  
	virtual ~dna_io_dna(void) {};

	dna_io_dna& operator=(const dna_io_dna& rhs);

	void read_ren_file(const string& filename, pv_string_vstring_pair stnRenaming);

	void write_dna_files(vdnaStnPtr* vStations, vdnaMsrPtr* vMeasurements, 
		const string& stnfilename, const string& msrfilename, const string& networkname,
		const CDnaDatum& datum, const CDnaProjection& projection, bool flagUnused,
		const string& stn_comment, const string& msr_comment);
	
	void write_dna_files(pvstn_t vbinary_stn, pvmsr_t vbinary_msr, 
		const string& stnfilename, const string& msrfilename, const string& networkname,
		const CDnaDatum& datum, const CDnaProjection& projection, bool flagUnused,
		const string& stn_comment, const string& msr_comment);

	// CDnaStation
	void write_stn_file(vdnaStnPtr* vStations, const string& stnfilename, const string& networkname,
		const CDnaDatum& datum, const CDnaProjection& projection, bool flagUnused,
		const string& comment);
	// station_t
	void write_stn_file(pvstn_t vbinary_stn, const string& stnfilename, const string& networkname,
		const CDnaDatum& datum, const CDnaProjection& projection, bool flagUnused,
		const string& comment);
	
	// CDnaMeasurement
	void write_msr_file(vdnaMsrPtr* vMeasurements, const string& msrfilename, const string& networkname,
		const CDnaDatum& datum, const string& comment);

	// measurement_t
	void write_msr_file(const vstn_t& binaryStn, pvmsr_t vbinary_msr, const string& msrfilename, const string& networkname,
		const CDnaDatum& datum, const string& comment);

	void read_dna_header(std::ifstream* ptr, string& version, INPUT_DATA_TYPE& idt,
		CDnaDatum& referenceframe, bool user_supplied_frame, bool override_input_frame,
		string& fileEpsg, string& fileEpoch, string& geoidversion, UINT32& count);

	inline const dna_stn_fields	dna_stn_positions() { return dsl_; }
	inline const dna_stn_fields	dna_stn_widths() { return dsw_; }
	inline const dna_msr_fields	dna_msr_positions() { return dml_; }
	inline const dna_msr_fields	dna_msr_widths() { return dmw_; }

	void set_dbid_ptr(pv_msr_database_id_map pv_msr_db_map);
	
protected:

private:

	void create_file_stn(std::ofstream* ptr, const string& filename);
	void create_file_msr(std::ofstream* ptr, const string& filename);
	void create_file_pointer(std::ofstream* ptr, const string& filename);
	void open_file_pointer(std::ifstream* ptr, const string& filename);

	void write_stn_header_data(std::ofstream* ptr, const string& networkname, const string& datum,
		const string& epoch, const size_t& count, const string& comment);
	void write_stn_header(std::ofstream* ptr, vdnaStnPtr* vStations, const string& networkname,
		const CDnaDatum& datum, bool flagUnused, const string& comment);
	void write_stn_header(std::ofstream* ptr, pvstn_t vbinary_stn, const string& networkname,
		const CDnaDatum& datum, bool flagUnused, const string& comment);

	void write_msr_header_data(std::ofstream* ptr, const string& networkname, const string& datum,
		const string& epoch, const size_t& count, const string& comment);
	void write_msr_header(std::ofstream* ptr, vdnaMsrPtr* vMeasurements, const string& networkname,
		const CDnaDatum& datum, const string& comment);
	void write_msr_header(std::ofstream* ptr, pvmsr_t vbinary_msrn, const string& networkname,
		const CDnaDatum& datum, const string& comment);

	void read_ren_data(std::ifstream* ptr, pv_string_vstring_pair stnRenaming);
	
	void prepare_sort_list(const UINT32 count);

	dna_stn_fields			dsl_, dsw_;
	dna_msr_fields			dml_, dmw_;

	vUINT32					vStationList_;

	pv_msr_database_id_map	pv_msr_db_map_;
	bool					m_databaseIDsSet_;
};

}	// namespace iostreams
}	// namespace dynadjust
	
#endif // DNAIODNA_H_
