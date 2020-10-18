//============================================================================
// Name         : dnameasurement.hpp
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
// Description  : Interface for the msr_t struct and CDnaMeasurement, 
//                CDnaCovariance and MsrTally classes
//============================================================================

#ifndef DNAMEASUREMENT_H
#define DNAMEASUREMENT_H

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <stdio.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/config/dnatypes.hpp>
#include <include/measurement_types/dnastation.hpp>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

enum {
	INST_WIDTH = 12,
	TARG_WIDTH = 12,
	STN_WIDTH = 16,
	MEAS_WIDTH = 18,
	MSR2_WIDTH = 12,
	VAR_WIDTH = 13,
	COV_WIDTH = 13
};

namespace dynadjust { namespace measurements {

template<typename C, typename S>
S measurement_name(const C& type)
{
	switch (type)
	{
	case 'A':
		return "(A) Horizontal angle";
	case 'B':
		return "(B) Geodetic azimuth";
	case 'K':
		return "(K) Astronomic azimuth";
	case 'C':
		return "(C) Chord dist";
	case 'E':
		return "(E) Ellipsoid arc";
	case 'M':
		return "(M) Mean sea level arc";
	case 'S':
		return "(S) Slope distance";
	case 'D':
		return "(D) Directions";
	case 'G':
		return "(G) GPS baseline";
	case 'X':
		return "(X) GPS baseline cluster";
	case 'H':
		return "(H) Orthometric height";
	case 'I':
		return "(I) Astronomic latitude";
	case 'J':
		return "(J) Astronomic longitude";
	case 'P':
		return "(P) Geodetic latitude";
	case 'Q':
		return "(Q) Geodetic longitude";
	case 'L':
		return "(L) Level difference";
	case 'R':
		return "(R) Ellipsoidal height";
	case 'V':
		return "(V) Zenith angle";
	case 'Z':
		return "(Z) Vertical angle";
	case 'Y':
		return "(Y) GPS point cluster";
	}
	return "";
}


// forward declarations
class CDnaGpsBaseline;
class CDnaGpsPoint;
class CDnaDirection;
class CDnaMeasurement;
class CDnaCovariance;

// measurement types
typedef boost::shared_ptr<CDnaMeasurement> dnaMsrPtr;
typedef vector<dnaMsrPtr> vdnaMsrPtr, *pvdnaMsrPtr;						// vector of dnaMsrPtr
typedef vdnaMsrPtr::iterator _it_vdnamsrptr;
typedef vdnaMsrPtr::const_iterator _it_vdnamsrptr_const;

typedef boost::shared_ptr<CDnaCovariance> dnaCovariancePtr;
typedef boost::shared_ptr< vector<CDnaCovariance> > vecCovariancePtr;

// data struct for storing measurement information to binary measurement file
typedef struct msr_t {
	msr_t()
		: measType('\0'), measStart(0), measurementStations(1), ignore(false), station1(0), station2(0)
		, station3(0), vectorCount1(0), vectorCount2(0), clusterID(0), fileOrder(0)
		, term1(0.), term2(0.), term3(0.), term4(0.), scale1(1.), scale2(1.), scale3(1.), scale4(1.)
		, measAdj(0.), measCorr(0.), measAdjPrec(0.), residualPrec(0.)
		, NStat(0.), TStat(0.), PelzerRel(0.)
		, preAdjCorr(0.), preAdjMeas(0.) 
	{
			memset(coordType, '\0', sizeof(coordType));
			// GDA94, lat, long, height
			memset(epsgCode, '\0', sizeof(epsgCode));
			sprintf(epsgCode, DEFAULT_EPSG_S);
			memset(epoch, '\0', sizeof(epoch));
	}

	char	measType;				// 'A', 'S', 'X', ... , etc.
	char	measStart;				// Start of a measurement (0=start or X, 1=Y, 2=Z, 3=covX, 4=covY, 5=covZ)
	char	measurementStations;	// One-, two- or three-station measurement
	char	epsgCode[7];			// epsg ID, i.e. NNNNN (where NNNNN is in the range 0-32767)
	char	epoch[12];				// date, i.e. "DD.MM.YYYY" (10 chars)
									// if datum is dynamic, Epoch is YYYY MM DD
									// if datum is static, Epoch is ignored
	char	coordType[4];			// "LLH", "UTM", ... , etc.
	bool	ignore;
	UINT32	station1;        		// stations 1, 2 and 3 are indices to
	UINT32	station2;       		// record numbers in the station
	UINT32	station3;				// record file.
	UINT32	vectorCount1;			// number of directions, GpsPoints or GpsBaselines
	UINT32	vectorCount2;			// number of covariances for GpsPoint or GpsBaseline
	UINT32	clusterID;				// cluster ID (which cluster this measurement belongs to)
	UINT32	fileOrder;				// original file order
	double	term1;					// measurement, X, Y, Z, dX, dY, dZ value
									// direction
	double	term2;					// measurement, XX, XY or XZ variance
									// direction variance
	double	term3;					// instrument height, YY or YZ variance
									// derived angle
	double	term4;					// target height or ZZ variance
									// derived angle variance
	double	scale1;					// phi, n or X scalar
									// derived angle covariance
	double	scale2;					// lambda, e or Y scalar
	double	scale3;					// height, up or Z scalar
	double	scale4;					// matrix scalar
	double	measAdj;
	double	measCorr;
	double	measAdjPrec;
	double	residualPrec;
	double	NStat;
	double	TStat;
	double	PelzerRel;
	double	preAdjCorr;
	double	preAdjMeas;
} measurement_t;

typedef vector<measurement_t> vmsr_t, *pvmsr_t;
typedef vmsr_t::iterator it_vmsr_t, *pit_vmsr_t;
typedef vmsr_t::const_iterator it_vmsr_t_const;

typedef struct scl_t {
	scl_t()
		: station1(""), station2("")
		, v_scale(1.), p_scale(1.), l_scale(1.), h_scale(1.) 
	{}

	string	station1;		
	string	station2;		
	double	v_scale;	// phi, n or X scalar
	double	p_scale;	// lambda, e or Y scalar
	double	l_scale;	// height, up or Z scalar
	double	h_scale;	// matrix scalar
} scalar_t;

typedef vector<scalar_t> vscl_t, *pvscl_t;
typedef vscl_t::iterator it_vscl_t, *pit_vscl_t;
typedef vscl_t::const_iterator it_vscl_t_const;


class CDnaCovariance
{
public:
	CDnaCovariance(void);
	CDnaCovariance(const CDnaCovariance& newCovariance);
	virtual ~CDnaCovariance();

	virtual inline CDnaCovariance* clone() const { return new CDnaCovariance(*this); }
	CDnaCovariance& operator=(const CDnaCovariance& rhs);
	bool operator==(const CDnaCovariance& rhs) const;

	inline CDnaCovariance& operator[](int iIndex) { return this[iIndex]; }

	inline void SetType(const string& str) { m_strType = trimstr(str); }
	inline char GetTypeC() const { return (m_strType.c_str())[0]; }

	// m_bIgnore used only to 'split' cluster measurements
	inline void SetIgnore(const bool bval) { m_bIgnore = bval; }
	inline bool GetIgnore() const { return m_bIgnore; }

	void coutCovarianceData(ostream &os) const;
	inline virtual UINT32 CalcBinaryRecordCount() const { return 3; }
	void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex, const string& epsgCode, const string& epoch) const;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord);
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr);
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream) const;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, 
		const dna_msr_fields& dmw, const dna_msr_fields& dml, 
		const msr_database_id_map& dbidmap, bool dbidSet) const;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid);

	void SerialiseDatabaseMap(std::ofstream* os, const UINT32& msr_id, const UINT32& cluster_id);

	inline void SetClusterID(const UINT32& id) { m_lclusterID = id; }
	inline void SetStn1Index(const UINT32& stn) { m_lstn1Index = stn; }
	inline void SetStn2Index(const UINT32& stn) { m_lstn2Index = stn; }
	void SetM11(const string& str);
	void SetM12(const string& str);
	void SetM13(const string& str);
	void SetM21(const string& str);
	void SetM22(const string& str);
	void SetM23(const string& str);
	void SetM31(const string& str);
	void SetM32(const string& str);
	void SetM33(const string& str);

	inline void SetM11(const double& dbl) { m_dM11 = dbl; }
	inline void SetM12(const double& dbl) { m_dM12 = dbl; }
	inline void SetM13(const double& dbl) { m_dM13 = dbl; }
	inline void SetM21(const double& dbl) { m_dM21 = dbl; }
	inline void SetM22(const double& dbl) { m_dM22 = dbl; }
	inline void SetM23(const double& dbl) { m_dM23 = dbl; }
	inline void SetM31(const double& dbl) { m_dM31 = dbl; }
	inline void SetM32(const double& dbl) { m_dM32 = dbl; }
	inline void SetM33(const double& dbl) { m_dM33 = dbl; }

	inline UINT32 GetClusterID() const { return m_lclusterID; }
	inline UINT32 GetStn1Index() const { return m_lstn1Index; }
	inline UINT32 GetStn2Index() const { return m_lstn2Index; }
	inline double GetM11() const { return m_dM11; }
	inline double GetM12() const { return m_dM12; }
	inline double GetM13() const { return m_dM13; }
	inline double GetM21() const { return m_dM21; }
	inline double GetM22() const { return m_dM22; }
	inline double GetM23() const { return m_dM23; }
	inline double GetM31() const { return m_dM31; }
	inline double GetM32() const { return m_dM32; }
	inline double GetM33() const { return m_dM33; }

protected:
	bool	m_bIgnore;
	UINT32	 m_lstn1Index;        			// This is an index to the record number in the station file
	UINT32	 m_lstn2Index;
	string	 m_strType;
	double	 m_dM11;
	double	 m_dM12;
	double	 m_dM13;
	double	 m_dM21;
	double	 m_dM22;
	double	 m_dM23;
	double	 m_dM31;
	double	 m_dM32;
	double	 m_dM33;
	UINT32	m_lclusterID;
};



// Abstract class for all measurement types
class CDnaMeasurement
{
public:
	CDnaMeasurement();
	CDnaMeasurement(const CDnaMeasurement&);
	virtual ~CDnaMeasurement();

	CDnaMeasurement& operator=(const CDnaMeasurement& rhs);
	virtual CDnaMeasurement* clone() const = 0;  // The Virtual (Copy) Constructor

	inline string GetType() const { return m_strType; }
	inline char GetTypeC() const { return (m_strType.c_str())[0]; }
	inline bool GetIgnore() const { return m_bIgnore; }
	inline bool NotIgnored() const { return m_bIgnore == false; }
	inline string GetFirst() const { return m_strFirst; }
	inline MEASUREMENT_STATIONS GetMsrStnCount() const { return m_MSmeasurementStations; }

	inline string GetEpsg() const { return m_epsgCode; }
	inline string GetSource() const { return m_sourceFile; }

	void SetType(const string& str);
	inline void SetIgnore(const bool bval) { m_bIgnore = bval; }
	inline void SetFirst(const string& str) { m_strFirst = trimstr(str); }
	
	inline void SetEpsg(const string& e) { m_epsgCode = trimstr(e); }
	inline void SetSource(const string& source) { m_sourceFile = source; }
	
	inline void SetMsrIndex(const UINT32& order) { m_lmeasurementIndex = order; }
	inline void SetStn1Index(const UINT32& order) { m_lstn1Index = order; }
	inline void SetStn2Index(const UINT32& order) { m_lstn2Index = order; }
	inline void SetStn3Index(const UINT32& order) { m_lstn3Index = order; }

	inline UINT32 GetMsrIndex() const { return m_lmeasurementIndex; }
	//inline UINT32& GetBMSFIndex() const { return m_lmeasurementIndex * sizeof(measurement_t); }
	inline UINT32 GetStn1Index() const { return m_lstn1Index; }
	inline UINT32 GetStn2Index() const { return m_lstn2Index; }
	inline UINT32 GetStn3Index() const { return m_lstn3Index; }

	inline double GetMeasAdj() const { return m_measAdj; }
	inline double GetMeasCorr() const { return m_measCorr; }
	inline double GetMeasAdjPrec() const { return m_measAdjPrec; }
	inline double GetResidualPrec() const { return m_residualPrec; }
	inline double GetPreAdjCorr() const { return m_preAdjCorr; }

	// pure virtual functions overridden by specialised classes
	virtual void coutMeasurementData(ostream &os, const UINT16& uType = 0) const = 0;
	virtual UINT32 CalcBinaryRecordCount() const = 0;
	//virtual UINT32 CalcDbidRecordCount() const;
	virtual void WriteBinaryMsr(std::ofstream* binary_stream, PUINT32 msrIndex) const = 0;
	virtual UINT32 SetMeasurementRec(std::ifstream* ifs_stns, std::ifstream* ifs_msrs, measurement_t* measRecord) = 0;
	virtual UINT32 SetMeasurementRec(const vstn_t& binaryStn, it_vmsr_t& it_msr) = 0;
	virtual void WriteDynaMLMsr(std::ofstream* dynaml_stream, bool bSubMeasurement = false) const = 0;
	virtual void WriteDNAMsr(std::ofstream* dynaml_stream, const dna_msr_fields& dmw, const dna_msr_fields& dml, bool bSubMeasurement = false) const = 0;
	virtual void SimulateMsr(vdnaStnPtr* vStations, const CDnaEllipsoid* ellipsoid) = 0;

	// virtual functions overridden by specialised classes
	virtual inline UINT32 GetClusterID() const { return 0; }
	virtual inline string GetCoordType() const { return m_strType; }
	virtual inline float GetInstrHeight() const { return 0; }
	virtual inline double GetVscale() const { return 0; }
	virtual inline double GetPscale() const { return 0; }
	virtual inline double GetLscale() const { return 0; }
	virtual inline double GetHscale() const { return 0; }
	virtual inline double GetStdDev() const { return 0; }
	virtual inline string GetTarget() const { return ""; }
	virtual inline string GetTarget2() const { return ""; }
	virtual inline float GetTargetHeight() const { return 0; }
	virtual inline UINT32 GetTotal() const { return 0; }
	virtual inline double GetValue() const { return 0; }

	virtual inline string GetReferenceFrame() const { return ""; }
	virtual inline string GetEpoch() const { return ""; }	
	
	virtual inline vector<CDnaGpsBaseline>* GetBaselines_ptr() { return 0; }
	virtual inline vector<CDnaDirection>* GetDirections_ptr() { return 0; }
	virtual inline vector<CDnaGpsPoint>* GetPoints_ptr() { return 0; }
	virtual inline vector<CDnaCovariance>* GetCovariances_ptr() { return 0; }

	virtual void AddDirection(const CDnaMeasurement* pDirection) {}
	virtual void AddGpsBaseline(const CDnaMeasurement* pGpsBaseline) {}
	virtual void AddGpsPoint(const CDnaMeasurement* pGpsPoint) {}
	virtual void AddGpsCovariance(const CDnaCovariance* pGpsCovariance) {}
	virtual void AddPointCovariance(const CDnaCovariance* pGpsCovariance) {}

	virtual void ReserveDirectionsCount(const UINT32& size) {}
	virtual void ReserveGpsBaselinesCount(const UINT32& size) {}
	virtual void ReserveGpsPointsCount(const UINT32& size) {}
	virtual void ReserveGpsCovariancesCount(const UINT32& size) {}
	virtual void ResizeGpsCovariancesCount(const UINT32& size = 0) {}

	virtual void SetRecordedTotal(const UINT32& total) {}
	virtual void SetClusterID(const UINT32& id) {}
	virtual void SetCoordType(const string& str) {}
	virtual void SetHscale(const string& str) {}
	virtual void SetHscale(const double& dbl) {}
	virtual void SetInstrumentHeight(const string& str) {}
	
	virtual void SetReferenceFrame(const string& str) {}
	virtual void SetEpoch(const string& str) {}
	
	virtual void SetLscale(const string& str) {}
	virtual void SetLscale(const double& dbl) {}
	virtual void SetPscale(const string& str) {}
	virtual void SetPscale(const double& dbl) {}
	virtual void SetSigmaXX(const string& str) {}
	virtual void SetSigmaXY(const string& str) {}
	virtual void SetSigmaXZ(const string& str) {}
	virtual void SetSigmaYY(const string& str) {}
	virtual void SetSigmaYZ(const string& str) {}
	virtual void SetSigmaZZ(const string& str) {}
	virtual void SetStdDev(const string& str) {}
	virtual void SetTarget(const string& str) {}
	virtual void SetTarget2(const string& str) {}
	virtual void SetTargetHeight(const string& str) {}
	virtual void SetTotal(const string& str) {}
	virtual void SetValue(const string& str) {}
	virtual void SetVscale(const string& str) {}
	virtual void SetVscale(const double& dbl) {}
	virtual void SetX(const string& str) {}
	virtual void SetY(const string& str) {}
	virtual void SetZ(const string& str) {}

	virtual void SetXAxis(const double& dbl) {}
	virtual void SetYAxis(const double& dbl) {}
	virtual void SetZAxis(const double& dbl) {}
	virtual void SetSigmaXX(const double& dbl) {}
	virtual void SetSigmaXY(const double& dbl) {}
	virtual void SetSigmaXZ(const double& dbl) {}
	virtual void SetSigmaYY(const double& dbl) {}
	virtual void SetSigmaYZ(const double& dbl) {}
	virtual void SetSigmaZZ(const double& dbl) {}
	virtual void SetTotal(const UINT32& u) {}
	
	virtual void PreferGMeasurements() {}

	virtual void coutBaselineData(ostream &os, const int& pad, const UINT16& uType = 0) {}

	void SetMeasurementDBID(const string& str);
	void SetClusterDBID(const string& str);
	
	inline void SetClusterDBID(const UINT32& u) { m_msr_db_map.cluster_id = u; }
	inline void SetMeasurementDBID(const UINT32& u) { m_msr_db_map.msr_id = u; }
	
	inline UINT32 GetClusterDBID() { return m_msr_db_map.cluster_id; }
	inline UINT32 GetMeasurementDBID() { return m_msr_db_map.msr_id; }
	//virtual inline UINT32 GetClusterDBID() const { return 0; }
	//virtual inline UINT32 GetMeasurementDBID() const { return 0; }

	void SetDatabaseMap(const msr_database_id_map& dbidmap, bool dbidSet);
	//virtual inline void SetDatabaseMap_bmsIndex(const UINT32& bmsIndex) { m_msr_db_map.bms_index = bmsIndex; }
	
	virtual void SerialiseDatabaseMap(std::ofstream* os);

protected:
	void coutMeasurement(ostream &os) const;

public:
	string	m_strFirst;
	MEASUREMENT_STATIONS m_MSmeasurementStations;
	
protected:
	string	m_strType;
	bool	m_bIgnore;

	UINT32 	m_lmeasurementIndex;
	UINT32	m_lstn1Index;        			// stations 1, 2 and 3 are indices to
	UINT32	m_lstn2Index;       			// record numbers in the station
	UINT32	m_lstn3Index;					// record file.

	double	m_measAdj;
	double	m_measCorr;
	double	m_measAdjPrec;
	double	m_residualPrec;
	double	m_preAdjCorr;

	string	m_epsgCode;
	string	m_sourceFile;
	
	msr_database_id_map		m_msr_db_map;
	bool					m_databaseIdSet;
};

// In the event a new measurement type is added, ensure SUPPORTED_MSR_COUNT is
// updated accordingly
class MsrTally
{
public:
	MsrTally();

	inline UINT32 SupportedMsrTypes() {
		return SUPPORTED_MSR_COUNT;
	}

	static void FillMsrList(vchar& msr_list);
	static string GetMsrName(const char& c);
	
	void initialise();

	MsrTally& operator+=(const MsrTally& rhs);
	MsrTally& operator-=(const MsrTally& rhs);
	MsrTally operator+(const MsrTally& rhs) const;
	MsrTally operator-(const MsrTally& rhs) const;
	UINT32 TotalCount();
	void coutSummary(ostream &os, const string& title);
	UINT32 MeasurementCount(const char& msrType);
	void CreateTally(const vdnaMsrPtr& vMeasurements);
	void CreateTally(const vmsr_t& vMeasurements, const vUINT32& CML);
	void IncrementMsrType(const char& msrType, const UINT32& count=1);

	void coutSummaryMsrToStn(ostream &os, const string& station);
	void coutSummaryMsrToStn_Compressed(ostream &os, const string& station);

	bool GPSOnly();	
	inline bool ContainsNonGPS() { return containsNonGPS; }

	static _MEASUREMENT_STATIONS_ Stations(const char& measType);
	
	bool containsNonGPS;

	UINT32 A, B, C, D, E, G, H, I, J, K, L, M, P, Q, R, S, V, X, Y, Z;
	UINT32 ignored;
	UINT32 SUPPORTED_MSR_COUNT;
	UINT32 totalCount;
};

typedef vector<MsrTally> vmsrtally;
typedef vmsrtally::iterator _it_vmsrtally;

template <typename T>
void MsrToStnSummaryHeaderLine(
	T& stream)
{
	UINT32 i, j(STATION + (NUMERIC_WIDTH * 20) + STAT);
	for (i=0; i<j; ++i)
		stream << "-";

	stream << endl;
}

template <typename T>
void MsrToStnSummaryHeader(
	T& stream, string& header)
{
	stream << endl << header << endl;
	stream << "------------------------------------------" << endl << endl;

	stream << setw(STATION) << left << "Station" <<
		setw(NUMERIC_WIDTH) << right <<	"A" <<
		setw(NUMERIC_WIDTH) << right <<	"B" <<
		setw(NUMERIC_WIDTH) << right <<	"C" <<
		setw(NUMERIC_WIDTH) << right <<	"D" <<
		setw(NUMERIC_WIDTH) << right <<	"E" <<
		setw(NUMERIC_WIDTH) << right << "G" <<
		setw(NUMERIC_WIDTH) << right <<	"H" <<
		setw(NUMERIC_WIDTH) << right <<	"I" <<
		setw(NUMERIC_WIDTH) << right <<	"J" <<
		setw(NUMERIC_WIDTH) << right <<	"K" <<
		setw(NUMERIC_WIDTH) << right <<	"L" <<
		setw(NUMERIC_WIDTH) << right <<	"M" <<
		setw(NUMERIC_WIDTH) << right <<	"P" <<
		setw(NUMERIC_WIDTH) << right <<	"Q" <<
		setw(NUMERIC_WIDTH) << right <<	"R" <<
		setw(NUMERIC_WIDTH) << right <<	"S" <<
		setw(NUMERIC_WIDTH) << right <<	"V" <<
		setw(NUMERIC_WIDTH) << right <<	"X" <<
		setw(NUMERIC_WIDTH) << right <<	"Y" <<
		setw(NUMERIC_WIDTH) << right <<	"Z" <<
		// Total
		setw(STAT) << right <<	"Total" <<
		endl;

	MsrToStnSummaryHeaderLine(stream);

}




}	// namespace measurements
}	// namespace dynadjust


#endif // ifndef DNAMEASUREMENT_H
