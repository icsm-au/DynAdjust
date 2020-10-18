//============================================================================
// Name         : dnatypes.hpp
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
// Description  : DynAdjust data types include file
//============================================================================

#ifndef DNATYPES_H_
#define DNATYPES_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <map>
#include <queue>
#include <vector>
#include <string>
#include <cstring>		// memset
#include <stdio.h>

using namespace std;

#ifdef UINT32
#undef UINT32
#endif

#ifdef UINT16
#undef UINT16
#endif

typedef unsigned int	UINT32, *PUINT32;
typedef const PUINT32	PUINT32_const;
typedef unsigned short	UINT16, *PUINT16;

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

const char LOWER_TRIANGLE = 'L';
const char UPPER_TRIANGLE = 'U';

const UINT16 VALID_STATION = 1;
const UINT16 INVALID_STATION = 0;

typedef vector<char> vchar;
typedef vchar::iterator _it_chr;

typedef vector<double> vdouble;
typedef vdouble::iterator _it_dbl;

typedef vector<bool> vbool;
typedef vbool::iterator _it_bool;
typedef vector<bool>::reference boolRef;

typedef string::iterator _it_str;
typedef string::const_iterator _it_str_const;
typedef vector<string> vstring, *pvstring;
typedef vstring::iterator _it_vstr;
typedef vstring::const_iterator _it_vstr_const;

typedef pair<vchar::iterator, vchar::iterator> _it_pair_vchar;

typedef pair<vstring::iterator, vstring::iterator> _it_pair_vstring;

typedef queue<UINT32> qUINT32;

typedef vector<UINT32> vUINT32, *pvUINT32;
typedef vector<vUINT32> vvUINT32, *pvvUINT32;

typedef vUINT32::iterator it_vUINT32;
typedef vUINT32::const_iterator it_vUINT32_const;
typedef vvUINT32::iterator it_vvUINT32;

typedef pair<string, string> string_string_pair;
typedef pair<string, UINT32> string_uint32_pair;
typedef pair<UINT32, UINT32> uint32_uint32_pair;
typedef pair<UINT32, string> uint32_string_pair;

typedef pair<bool, UINT32> bool_uint32_pair;

typedef pair<string, vstring> string_vstring_pair;

typedef pair<uint32_uint32_pair, UINT32> u32u32_uint32_pair;
typedef pair<UINT32, uint32_uint32_pair> uint32_u32u32_pair;

typedef pair<double, double> doubledouble_pair;
typedef pair<string_string_pair, doubledouble_pair> stringstring_doubledouble_pair;
typedef vector<stringstring_doubledouble_pair> v_stringstring_doubledouble_pair, *pv_stringstring_doubledouble_pair;
typedef v_stringstring_doubledouble_pair::iterator _it_string_doubledouble_pair;

typedef vector<doubledouble_pair> v_doubledouble_pair, *pv_doubledouble_pair;
typedef pair<string, v_doubledouble_pair> string_v_doubledouble_pair;
typedef vector<string_v_doubledouble_pair> v_string_v_doubledouble_pair, *pv_string_v_doubledouble_pair;
typedef v_string_v_doubledouble_pair::iterator it_v_string_v_doubledouble_pair;

typedef pair<uint32_uint32_pair, double> u32u32_double_pair;

typedef pair<string, bool> stringbool_pair;
typedef pair<string_uint32_pair, stringbool_pair> stringuint32_stringbool_pair;

/////////////////////////////////////////////////////////////
// Custom pair type to manage appearances of a station
template <class T1=UINT32, class T2=bool> 
struct stn_appearance_t
{
	typedef T1 station_type;
	typedef T2 appearance_type;

	T1 station_id;
	T2 first_appearance_fwd;
	T2 first_appearance_rev;

	void set_id(const T1& id) { station_id = id; }
	void first_fwd() { first_appearance_fwd = true; }
	void first_rev() { first_appearance_rev = true; }

	stn_appearance_t()
		: station_id(0), first_appearance_fwd(false), first_appearance_rev(false) {}
	stn_appearance_t(const T1& id, const T2& f, const T2& r) 
		: station_id(id), first_appearance_fwd(f), first_appearance_rev(r) {}
	template <class U, class V>
	stn_appearance_t (const stn_appearance_t<U, V> &p) 
		: station_id(p.station_id)
		, first_appearance_fwd(p.first_appearance_fwd)
		, first_appearance_rev(p.first_appearance_rev) {}
};

typedef stn_appearance_t<UINT32, bool> stn_appear;
typedef vector<stn_appear> v_stn_appear;
typedef v_stn_appear::iterator it_vstn_appear;
typedef vector<v_stn_appear> vv_stn_appear;
typedef vv_stn_appear::iterator it_vvstn_appear;
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Custom pair type to map the location of stations in blocks
// Since every station in the network will appear in this list,
// valid_station will be used to determine whether this station
// is used or not.
template <class T=UINT32, class U=bool> 
struct stn_block_map_t
{
	T block_no;
	U first_appearance_fwd;
	U first_appearance_rev;
	U valid_stn;

	stn_block_map_t()
		: block_no(0)
		, first_appearance_fwd(false), first_appearance_rev(false)
		, valid_stn(false) {}
	stn_block_map_t(const T& block, const U fwd, const U rev, const U validity)
		: block_no(block)
		, first_appearance_fwd(fwd), first_appearance_rev(rev)
		, valid_stn(validity) {}
	stn_block_map_t (const stn_block_map_t<T> &p) 
		: block_no(p.block_no)
		, first_appearance_fwd(p.first_appearance_fwd)
		, first_appearance_rev(p.first_appearance_rev)
		, valid_stn(p.valid_stn) {}
	void firstAppearanceFwd(const T& block) {
		block_no = block;
		first_appearance_fwd = true;
	}
	void firstAppearanceRev() {
		first_appearance_rev = true;
	}
};

typedef stn_block_map_t<UINT32> stn_block_map;
typedef vector<stn_block_map> v_stn_block_map;
typedef v_stn_block_map::iterator it_vstn_block_map;
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Custom pair type for "free stations" vector -default value is free
template <class T1=UINT32, class T2=bool> 
struct freestnpair_t
{
	typedef T1 stnindex_type;
	typedef T2 free_type;

	T1 stn_index;
	T2 available;
	void consume() { available = false; }
	const T2 isfree() const { return available; }
	freestnpair_t() : stn_index(T1()), available(true) {}
	freestnpair_t(const T1& x, const T2& y) : stn_index(x), available(y) {}
	template <class U, class V>
	freestnpair_t (const freestnpair_t <U, V> &p) : stn_index(p.stn_index), available(p.available) { }
};

typedef freestnpair_t<UINT32, bool> freestn_pair;
typedef vector<freestn_pair> v_freestn_pair;
typedef v_freestn_pair::iterator it_freestn_pair;
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Custom pair type for AML vector -default value is free
template <class T1=UINT32, class T2=bool> 
struct amlpair_t
{
	typedef T1 bmsindex_type;
	typedef T2 free_type;

	T1 bmsr_index;
	T2 available;
	void consume() { available = false; }
	amlpair_t() : bmsr_index(T1()), available(true) {}
	amlpair_t(const T1& x, const T2& y) : bmsr_index(x), available(y) {}
	template <class U, class V>
	amlpair_t (const amlpair_t<U, V> &p) : bmsr_index(p.bmsr_index), available(p.available) { }
};

typedef amlpair_t<UINT32, bool> aml_pair;
typedef vector<aml_pair> v_aml_pair;
typedef v_aml_pair::iterator it_aml_pair;
/////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// Custom pair type for MT adjustments -default value is not adjusted (false)
template <class T1=UINT32, class T2=bool> 
struct sequential_adj_t
{
	typedef T1 blockindex_type;
	typedef T2 adjusted_type;

	T1 block_index;
	T2 adjusted;
	void solution() { adjusted = true; }
	void nosolution() { adjusted = false; }
	sequential_adj_t() 
		: block_index(T1()), adjusted(false) {}
	sequential_adj_t(const T1& x, const T2& y) 
		: block_index(x), adjusted(y) {}
	template <class U, class V>
	sequential_adj_t (const sequential_adj_t<U, V> &p) 
		: block_index(p.block_index), adjusted(p.adjusted) { }
};

typedef sequential_adj_t<UINT32, bool> sequential_adj;
typedef vector<sequential_adj> v_sequential_adj;
typedef v_sequential_adj::iterator it_sequential_adj;
/////////////////////////////////////////////////////////////

typedef map<UINT32, UINT32> uint32_uint32_map;
typedef uint32_uint32_map::iterator it_uint32_uint32_map;
typedef vector<uint32_uint32_map> v_uint32_uint32_map;
typedef v_uint32_uint32_map::iterator it_v_uint32_uint32_map;

typedef vector<string_string_pair> v_string_string_pair, *pv_string_string_pair;
typedef vector<string_vstring_pair> v_string_vstring_pair, *pv_string_vstring_pair;

typedef vector<string_uint32_pair> v_string_uint32_pair, *pv_string_uint32_pair;
typedef vector<uint32_uint32_pair> v_uint32_uint32_pair, *pv_uint32_uint32_pair;
typedef vector<uint32_string_pair> v_uint32_string_pair, *pv_uint32_string_pair;
typedef vector<u32u32_uint32_pair> v_u32u32_uint32_pair;
typedef vector<uint32_u32u32_pair> v_uint32_u32u32_pair;

typedef v_string_vstring_pair::iterator _it_string_vstring_pair;
typedef v_string_uint32_pair::iterator _it_string_uint32_pair;
typedef v_uint32_string_pair::iterator _it_uint32_string_pair;
typedef v_u32u32_uint32_pair::iterator _it_u32u32_uint32_pair;
typedef v_uint32_u32u32_pair::iterator _it_uint32_u32u32_pair;
typedef v_uint32_uint32_pair::iterator _it_uint32_uint32_pair;

typedef vector<v_string_string_pair> vv_string_string_pair;

typedef pair<it_vUINT32_const, it_vUINT32_const> _it_pair_vUINT32_const;
typedef pair<it_vUINT32, it_vUINT32> it_pair_vUINT32;


typedef v_string_string_pair::iterator it_string_pair;
typedef pair<v_string_string_pair::iterator, v_string_string_pair::iterator> it_pair_string;
typedef pair<_it_string_uint32_pair, _it_string_uint32_pair> it_pair_string_vUINT32;
typedef pair<_it_uint32_string_pair, _it_uint32_string_pair> it_pair_uint32_string;

typedef pair<uint32_uint32_map::iterator, uint32_uint32_map::iterator> it_pair_map_vUINT32_vUINT32;

typedef enum _SIGMA_ZERO_STAT_PASS_
{
	test_stat_pass = 0,
	test_stat_warning = 1,
	test_stat_fail = 2
} SIGMA_ZERO_STAT_PASS;

typedef enum _INPUT_FILE_TYPE_
{
	geodesyml = 0,
	dynaml = 1,
	dna = 2,
	csv = 3,
	sinex = 4
} INPUT_FILE_TYPE;

typedef enum _INPUT_DATA_TYPE_
{
	stn_data = 0,
	msr_data = 1,
	stn_msr_data = 2,
	geo_data = 3,
	ren_data = 4,
	unknown = 5
} INPUT_DATA_TYPE;

typedef enum _TIMER_TYPE_
{
	iteration_time = 0,
	total_time = 1
} TIMER_TYPE;

typedef enum _ANGULAR_TYPE_
{
	DMS = 0,
	//#define DMIN 1
	DDEG = 1
} ANGULAR_TYPE;

typedef enum _DMS_FORMAT_
{
	SEPARATED = 0,
	SEPARATED_WITH_SYMBOLS = 1,
	HP_NOTATION = 2
} DMS_FORMAT;

typedef enum _COORD_TYPE_
{
	XYZ_type_i = 0,		// Cartesian
	LLh_type_i = 1,		// Geographic (ellipsoid height)
	LLH_type_i = 2,		// Geographic (orthometric height)
	UTM_type_i = 3,		// Projection
	ENU_type_i = 4,		// Local
	AED_type_i = 5		// Azimuth, elevation and distance
} COORD_TYPE;

typedef enum _STATION_ELEM_
{
	station_1 = 0,
	station_2 = 1,
	station_3 = 2
} STATION_ELEM;

typedef enum _CART_ELEM_
{
	x_element = 0,
	y_element = 1,
	z_element = 2
} CART_ELEM;

typedef enum _COORDINATE_TYPES_
{
	latitude_t = 0,
	longitude_t = 1,
	easting_t = 2,
	northing_t = 3
} COORDINATE_TYPES;

typedef enum _HEIGHT_SYSTEM_
{
	ORTHOMETRIC_type_i = 0,
	ELLIPSOIDAL_type_i = 1
} HEIGHT_SYSTEM;

typedef enum _MEASUREMENT_STATIONS_ {
	ONE_STATION = 1,
	TWO_STATION = 2,
	THREE_STATION = 3,
	UNKNOWN_TYPE = -1
} MEASUREMENT_STATIONS;

typedef enum _MEASUREMENT_START_ {
	xMeas = 0,
	yMeas = 1,
	zMeas = 2,
	xCov = 3,
	yCov = 4,
	zCov = 5
} MEASUREMENT_START;

enum iosMode
{
	binary = 0,
	ascii = 1
};

enum mtxType
{
	// mtx_full is intended for non-square matrices which are
	// commonly fully populated, such as a vector of estimates.
	// In this case, the whole buffer is copied/stored.
	mtx_full = 0,
	// mtx_lower is intended for square matrices, such as 
	// full variance matrices and normal equations.
	// In this case, data is stored in columns, from left to right
	mtx_lower = 1,
	// mtx_sparse is intended for matrices which have many 
	// zeros, such as design and AtV-1 matrices.
	// In this case, each element is stored with its index
	mtx_sparse = 2
};

typedef enum _STAGE_FILE_
{
	sf_normals = 0,
	sf_normals_r = 1,
	sf_atvinv = 2,
	sf_design = 3,
	sf_meas_minus_comp = 4,
	sf_estimated_stns = 5,
	sf_original_stns = 6,
	sf_rigorous_stns = 7,
	sf_junction_vars = 8,
	sf_junction_vars_f = 9,
	sf_junction_ests_f = 10,
	sf_junction_ests_r = 11,
	sf_rigorous_vars = 12,
	sf_prec_adj_msrs = 13,
	sf_corrections = 14
} STAGE_FILE;

typedef struct {
	double	_fwdChiSquared;
	double	_revChiSquared;
	double	_rigChiSquared;
	int	_degreesofFreedom;
} statSummary_t;
	
typedef struct stationCorrections {
	stationCorrections(const string& stn="")
		: _station(stn), _azimuth(0.), _vAngle(0.), _sDistance(0.)
		, _hDistance(0.), _east(0.), _north(0.), _up(0.) {}

	string	_station;
	double	_azimuth;
	double	_vAngle;
	double	_sDistance;
	double	_hDistance;
	double	_east;
	double	_north;
	double	_up;
} stationCorrections_t;

typedef vector<stationCorrections_t> vstnCor_t, *pvstnCor_t;
typedef vector<vstnCor_t> vvstnCor_t;
typedef vstnCor_t::iterator it_vstnCor_t;

typedef struct {
	string	_station;
	double	_latitude;
	double	_longitude;
	double	_hzPosU;
	double	_vtPosU;
	double	_semimMajor;
	double	_semimMinor;
	double	_orientation;
	double	_xx;
	double	_xy;
	double	_xz;
	double	_yy;
	double	_yz;
	double	_zz;
} stationPosUncertainty_t;

typedef vector<stationPosUncertainty_t> vstnPU_t, *pvstnPU_t;
typedef vector<vstnPU_t> vvstnPU_t;
typedef vstnPU_t::iterator it_vstnPU_t;

typedef struct {
	string	_networkName;
	vstring	_initialStns;
	UINT32	_minInnerStns;
	UINT32	_maxTotalStns;
	UINT16	_sortStnsByMsrs;
	bool	_quiet;
	UINT16	_verbose;			// Give detailed information about what dnainterop is doing.
								// 0: No information (default)
								// 1: Helpful information
								// 2: Extended information
								// 3: Debug level information
} segmentParam_t;

typedef struct block_meta {
	block_meta() : _blockIsolated(false), _blockFirst(false), _blockLast(false), _blockIntermediate(false) {}

	bool	_blockIsolated;		// Does this block comprise a single contiguous network (an isolated block)
								//		True:	A single block in isolation
								//		False:	One of two or more segmented blocks
	bool	_blockFirst;		// Is this the first block in a list, or an only block?
	bool	_blockLast;			// Is this the last block in a list, or an only block?
	bool	_blockIntermediate;	// Not the first or last, but an intermediate block of a list of three or more
} blockMeta_t;

const UINT16 STN_NAME_WIDTH(31);
const UINT16 STN_DESC_WIDTH(129);
const UINT16 STN_CONST_WIDTH(4);
const UINT16 STN_TYPE_WIDTH(4);
const UINT16 STN_EPSG_WIDTH(7);
const UINT16 STN_EPOCH_WIDTH(12);

// data struct for storing station information to binary station file
typedef struct stn_t {
	stn_t(const short& u=0)	
		: suppliedStationType(LLH_type_i), initialLatitude(0.), currentLatitude(0.), initialLongitude(0.), currentLongitude(0.)
		, initialHeight(0.), currentHeight(0.), suppliedHeightRefFrame(ELLIPSOIDAL_type_i)
		, geoidSep(0.), meridianDef(0.), verticalDef(0.), zone(u)
		, fileOrder(0), nameOrder(0), clusterID(0), unusedStation(FALSE) 
	{
		memset(stationName, '\0', sizeof(stationName));
		memset(stationConst, '\0', sizeof(stationConst));
		memset(stationType, '\0', sizeof(stationType));
		memset(description, '\0', sizeof(description));
		// GDA2020, lat, long, height
		memset(epsgCode, '\0', sizeof(epsgCode));
		sprintf(epsgCode, "7843");
		memset(epoch, '\0', sizeof(epoch));
	}

	char	stationName[STN_NAME_WIDTH];	// 30 characters
	char	stationConst[STN_CONST_WIDTH];	// constraint: lat, long, height
	char	stationType[STN_TYPE_WIDTH];	// type: LLH, UTM, XYZ
	UINT16	suppliedStationType;			// type supplied by the user (required for adjust)
	double	initialLatitude;				// initial estimate
	double	currentLatitude;				// current estimate
	double	initialLongitude;
	double	currentLongitude;
	double	initialHeight;					// initialHeight and currentHeight are always assumed to be
	double	currentHeight;					// ellipsoidal (ELLIPSOIDAL_type_i).  If the user runs geoid using
											// the convert option, then suppliedHeightRefFrame is set to
											// ORTHOMETRIC_type_i and currentHeight is set to (initialHeight + N). 
	UINT16	suppliedHeightRefFrame;			// Used to signify which reference frame supplied height refers to
	float	geoidSep; 					 	// ellipsoid / geoid separation
	double	meridianDef;					// deflection in meridian (N/S)
	double	verticalDef;					// deflection in vertical (E/W)
	short	zone;
	char	description[STN_DESC_WIDTH];	// 128 characters
	UINT32	fileOrder;						// original file order
	UINT32	nameOrder;						// station name sorted position
	UINT32	clusterID;						// cluster ID (which cluster this station belongs to)
	UINT16	unusedStation;					// is this station unused?
	char	epsgCode[STN_EPSG_WIDTH];		// epsg ID, i.e. NNNNN (where NNNNN is in the range 0-32767)
	char	epoch[STN_EPOCH_WIDTH];			// date, i.e. "DD.MM.YYYY" (10 chars)
											// if datum is dynamic, Epoch is YYYY MM DD
											// if datum is static, Epoch is ignored
} station_t;


typedef vector<station_t> vstn_t, *pvstn_t;
typedef vstn_t::iterator it_vstn_t;
typedef vstn_t::const_iterator it_vstn_t_const;
typedef vector<statSummary_t> vsummary_t, *pvsummary_t;

const UINT32 MOD_NAME_WIDTH(20);
const UINT32 FILE_NAME_WIDTH(256);

typedef struct input_file_meta {
	char	filename[FILE_NAME_WIDTH+1];	// Input file path
	char	epsgCode[STN_EPSG_WIDTH+1];		// Input file epsg ID, i.e. NNNNN (where NNNNN is in the range 0-32767). "Mixed" if stations are on different reference frames
	char	epoch[STN_EPOCH_WIDTH+1];	// Input file epoch
	UINT16	filetype;						// Input file type (geodesyml, dynaml, dna, csv, sinex)
	UINT16	datatype;						// Input data type (station, measurement, both)
} input_file_meta_t;

typedef struct binary_file_meta {
	binary_file_meta () 
		: reduced(false), inputFileMeta(NULL) {}
	binary_file_meta (const string& app_name) 
		: reduced(false), inputFileMeta(NULL) {
			sprintf(modifiedBy, "%s", app_name.c_str());
	}
	~binary_file_meta() {
		if (inputFileMeta != NULL)
			delete []inputFileMeta;
	}
	UINT32				binCount;						// number of records in the binary file
	bool				reduced;						// indicates whether the data is reduced(true) or raw(false)
	char				modifiedBy[MOD_NAME_WIDTH+1];	// the program that modified this file
	char				epsgCode[STN_EPSG_WIDTH+1];		// epsg ID, i.e. NNNNN (where NNNNN is in the range 0-32767). "Mixed" if stations are on different reference frames
	char				epoch[STN_EPOCH_WIDTH+1];		// date, i.e. "DD.MM.YYYY" (10 chars)
	UINT16				inputFileCount;					// Number of source file metadata elements
	input_file_meta_t*	inputFileMeta;					// Source file metadata
} binary_file_meta_t;

typedef vector<input_file_meta_t> vifm_t;
typedef vifm_t::iterator it_vifm_t;
typedef vector<binary_file_meta_t> vbfm_t;
typedef vbfm_t::iterator it_vbfm_t;

template <typename S>
S formatStnMsrFileSourceString(const vifm_t* vfile_meta, const size_t& file_type)
{
	string source_files("");
	bool this_file;

	for (UINT32 i(0); i<vfile_meta->size(); ++i)
	{
		this_file= false;
		switch (file_type)
		{
		case stn_data:
			if (vfile_meta->at(i).datatype == stn_data || 
				vfile_meta->at(i).datatype == stn_msr_data)
				this_file= true;
			break;
		case msr_data:
			if (vfile_meta->at(i).datatype == msr_data || 
				vfile_meta->at(i).datatype == stn_msr_data)
				this_file= true;
			break;
		case stn_msr_data:
			if (vfile_meta->at(i).datatype == stn_data || 
				vfile_meta->at(i).datatype == msr_data || 
				vfile_meta->at(i).datatype == stn_msr_data)
				this_file= true;
			break;
		}

		if (this_file)
		{
			source_files += vfile_meta->at(i).filename;
			source_files += " ";
		}
	}

	return source_files;
}

#endif // DNATYPES_H_
