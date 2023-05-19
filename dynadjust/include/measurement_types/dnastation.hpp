//============================================================================
// Name         : dnastation.hpp
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
// Description  : Interface for the CDnaStation class
//============================================================================

#ifndef DNASTATION_H
#define DNASTATION_H

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <cctype>
#include <fstream>
#include <include/config/dnaexports.hpp>
#include <include/config/dnatypes.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaconsts-iostream.hpp>
#include <include/config/dnaoptions.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnatemplatecalcfuncs.hpp>
#include <include/functions/dnatemplategeodesyfuncs.hpp>
#include <include/parameters/dnaellipsoid.hpp>
#include <include/parameters/dnaprojection.hpp>
#include <include/io/dnaiodnatypes.hpp>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace dynadjust::datum_parameters;

#define XML_INDENT_1 "    "
#define XML_INDENT_2 "        "
#define XML_INDENT_3 "            "

namespace dynadjust { namespace measurements {

class CAStationList;
class CDnaStation;

// station types
typedef boost::shared_ptr<CDnaStation> dnaStnPtr;
typedef vector<dnaStnPtr> vdnaStnPtr;		// vector of dnaStnPtr
typedef vdnaStnPtr::iterator _it_vdnastnptr;
typedef vdnaStnPtr::const_iterator _it_vdnastnptr_const;
typedef pair<_it_vdnastnptr, _it_vdnastnptr> it_pair_dnastnptr;

typedef vector<CAStationList> vASL, *pvASL;
typedef vASL::iterator _it_vasl;
typedef boost::shared_ptr<CAStationList> ASLPtr;
typedef vector<ASLPtr> vASLPtr, *pvASLPtr;


#ifdef _MSC_VER
class DNATYPE_API CAStationList
#else
class CAStationList
#endif
{
public:
	CAStationList();
	CAStationList(bool validStation);
	virtual ~CAStationList();

	// move constructor and assignment operator
	CAStationList(const CAStationList&& s);
	CAStationList& operator=(CAStationList&& rhs);

private:
	// disallow copying
	CAStationList(const CAStationList&);
	CAStationList& operator=(const CAStationList& rhs);

public:

	inline CAStationList* handle() { return this; }

	inline void SetInvalid() { validStation_ = FALSE; }
	inline void SetValid() { validStation_ = TRUE; }

	inline bool IsInvalid() { return validStation_ == INVALID_STATION; }
	inline bool IsValid() { return validStation_ == VALID_STATION; }
	
	inline UINT32 GetAvailMsrCount() const { return availMsrCount_; }
	inline UINT32 GetAssocMsrCount() const { return assocMsrCount_; }
	inline UINT32 GetAMLStnIndex() const { return amlStnIndex_; }
	inline UINT16 Validity() const { return validStation_; }
	inline UINT32* GetAvailMsrCountPtr() { return &availMsrCount_; }
	inline UINT32* GetAssocMsrCountPtr() { return &assocMsrCount_; }
	inline UINT32* GetAMLStnIndexPtr() { return &amlStnIndex_; }
	inline UINT16* ValidityPtr() { return &validStation_; }
	
	inline void DecrementAvailMsrCount() { 
		if (availMsrCount_ > 0) 
			--availMsrCount_; 
	}
	inline void DecrementMsrCount() { 
		if (assocMsrCount_ > 0) 
			--assocMsrCount_; 
		if (availMsrCount_ > 0) 
			--availMsrCount_; 
	}
	
	inline void IncrementMsrCount() { ++assocMsrCount_; }
	inline void IncrementMsrCount(const UINT32& increment) { assocMsrCount_ += increment; }
	
	inline void InitAssocMsrCount() { assocMsrCount_ = 0; }
	inline void SetAvailMsrCount(const UINT32& availMsrCount) { availMsrCount_ = availMsrCount; }
	inline void SetAssocMsrCount(const UINT32& assocMsrCount) { assocMsrCount_ = assocMsrCount; }
	inline void SetAMLStnIndex(const UINT32& amlStnIndex) { amlStnIndex_ = amlStnIndex; }

protected:
	UINT32 availMsrCount_;			// all associated measurements, not including ignored measurements
	UINT32 assocMsrCount_;			// all associated measurements, including ignored measurements
	UINT32 amlStnIndex_;
	UINT16 validStation_;
};
	

#ifdef _MSC_VER
class DNATYPE_API CDnaStation
#else
class CDnaStation
#endif	
{
public:
	CDnaStation(const string& referenceframe, const string& epoch);
	CDnaStation(const CDnaStation&);
	CDnaStation(const string& strName, const string& strConstraints, const string& strType, const double& dXAxis, const double& dYAxis, const double& dZAxis, const double& dHeight, const string& strHemisphereZone, const string& strDescription, const string& strComment);
	virtual ~CDnaStation();

	CDnaStation& operator=(const CDnaStation& rhs);
	inline CDnaStation* clone() const { return new CDnaStation(*this); }

	//inline CDnaStation& operator[](int iIndex) { return this[iIndex]; }

	friend bool operator<(const CDnaStation& left, const CDnaStation& right);
	friend bool operator<(const boost::shared_ptr<CDnaStation>& left, const boost::shared_ptr<CDnaStation>& right);
	
	inline int CompareStationName(const string& s) { return m_strName.compare(s); }
	inline string GetName() const { return m_strName; }
	inline string GetOriginalName() const { return m_strOriginalName; }
	inline string GetConstraints() const { return m_strConstraints; }
	inline CONSTRAINT_TYPE GetConstraintType() const { return m_constraintType; }
	inline string GetCoordType() const { return m_strType; }

	inline double GetXAxis() const { return m_dXAxis; }	// X, Easting, Latitude
	inline double GetYAxis() const { return m_dYAxis; }	// Y, Northing, Longitude
	inline double GetZAxis() const { return m_dZAxis; }	// Z
	inline double GetLatitude() const { return m_dXAxis; }
	inline double GetLongitude() const { return m_dYAxis; }
	inline double GetEasting() const { return m_dXAxis; }
	inline double GetNorthing() const { return m_dYAxis; }
	inline double GetHeight() const { return m_dHeight; }
	inline string GetHemisphereZone() const { return m_strHemisphereZone; }
	inline string GetDescription() const { return m_strDescription; }
	inline string GetComment() const { return m_strComment; }

	inline double GetcurrentLatitude() const { return m_dcurrentLatitude; }
	inline double GetcurrentLongitude() const { return m_dcurrentLongitude; }
	inline double GetcurrentHeight() const { return m_dcurrentHeight; }
	inline float GetgeoidSep() const { return m_fgeoidSep; }
	inline double GetmeridianDef() const { return m_dmeridianDef; }
	inline double GetverticalDef() const { return m_dverticalDef; }
	inline UINT32 GetnameOrder() const { return m_lnameOrder; }
	inline UINT32 GetZone() const { return m_zone; }

	inline UINT32 GetfileOrder() const { return m_lfileOrder; }
	

	inline bool IsUnused() const { return m_unusedStation; }
	inline bool IsNotUnused() const { return m_unusedStation == false; }

	static bool IsValidConstraint(const string& sConst);
	static _COORD_TYPE_ GetCoordTypeC(const string& sType);
	_COORD_TYPE_ GetMyCoordTypeC() const;
	_HEIGHT_SYSTEM_ GetHeightSystemC(const string& sType) const;
	_HEIGHT_SYSTEM_ GetMyHeightSystemC() const;
	inline _HEIGHT_SYSTEM_ GetMyHeightSystem() const { return m_htType;	}
	
	void SetHeightSystem(const string& sType);
	void SetHeightSystem(const _HEIGHT_SYSTEM_& type_i);

	void SetStationRec(const station_t& stationRecord);

	void SetXAxis(const string& str);
	void SetYAxis(const string& str);
	void SetZAxis(const string& str);
	void SetHeight(const string& str);
	void SetCoordType(const string& sType);

	void ReduceStations_LLH(const CDnaEllipsoid* m_eEllipsoid, const CDnaProjection* m_pProjection);
	void ReduceStations_XYZ(const CDnaEllipsoid* m_eEllipsoid, const CDnaProjection* m_pProjection);
		
	inline void SetXAxis_d(const double& dXAxis) { m_dXAxis = dXAxis; }
	inline void SetYAxis_d(const double& dYAxis) { m_dYAxis = dYAxis; }
	inline void SetZAxis_d(const double& dZAxis) { m_dZAxis = dZAxis; }
	inline void SetXAxisStdDev_d(const double& dXAxisStdDev) { m_dStdDevX = dXAxisStdDev; }
	inline void SetYAxisStdDev_d(const double& dYAxisStdDev) { m_dStdDevY = dYAxisStdDev; }
	inline void SetZAxisStdDev_d(const double& dZAxisStdDev) { m_dStdDevZ = dZAxisStdDev; }
	inline void SetHeight_d(const double& dHeight) { m_dHeight = dHeight; }
	inline void SetcurrentHeight_d(const double& dHeight) { m_dcurrentHeight = dHeight; }
	
	inline void SetName(const string& sName) { m_strName = trimstr(sName); }
	inline void SetOriginalName(const string& sName) { m_strOriginalName = trimstr(sName); }
	inline void SetOriginalName() { m_strOriginalName = m_strName; }
	void SetHemisphereZone(const string& sHemisphereZone);
	inline void SetDescription(const string& sDescription) { m_strDescription = trimstr(sDescription); }
	inline void SetComment(const string& sComment) { m_strComment = trimstr(sComment); }

	inline void SetfileOrder(const UINT32& order) { m_lfileOrder = order; }
	inline void SetnameOrder(const UINT32& order) { m_lnameOrder = order; }

	inline void SetgeoidSep(const float& fgeoidSep) { m_fgeoidSep = fgeoidSep; }
	inline void SetmeridianDef(const double& dmeridianDef) { m_dmeridianDef = dmeridianDef; }
	inline void SetverticalDef(const double& dverticalDef) { m_dverticalDef = dverticalDef; }	

	inline void SetStationUnused() { m_unusedStation = true; }
	inline void SetStationUse(bool use) { m_unusedStation = use; }

	void SetConstraints(const string& str);

	void SetXAxisStdDev(const string& str);
	void SetYAxisStdDev(const string& str);
	void SetHeightStdDev(const string& str);

	void PrepareStnData(double& lat_east_x,	double& lon_north_y, double& ht_zone_z,
		string& hemisphereZone, string& coordinateType,
		UINT32& LEX_precision, UINT32& LNY_precision, UINT32& HZ_precision,
		const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection, bool writeCurrentEstimates);
	
	//void coutStationData(ostream &os, ostream &os2, const UINT16& uType = 0) const;
	void WriteBinaryStn(std::ofstream* binary_stream, const UINT16 bUnused=0);
		
	void WriteDNAXMLStnCurrentEstimates(std::ofstream* dna_ofstream, 
		const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
		INPUT_FILE_TYPE t, const dna_stn_fields* dsw=0);
	
	void WriteDNAXMLStnInitialEstimates(std::ofstream* dna_ofstream, 
		const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
		INPUT_FILE_TYPE t, const dna_stn_fields* dsw=0);
	
	void WriteDNAXMLStn(std::ofstream* dna_ofstream,
		const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
		const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
		INPUT_FILE_TYPE t, const dna_stn_fields* dsw=0);

	void WriteDNAStn(std::ofstream* dna_ofstream, const string& coordinateType, 
		const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
		string& hemisphereZone, const dna_stn_fields& dsw);

	void WriteDynaMLStn(std::ofstream* xml_ofstream, const string& coordinateType, 
		const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
		string& hemisphereZone);

	void WriteGeoidfile(std::ofstream* binary_stream);

	static string CoordinateName(const char& c);

	inline string GetReferenceFrame() const { return m_referenceFrame; }
	inline string GetEpoch() const { return m_epoch; }
	
	inline void SetReferenceFrame(const string& r) { m_referenceFrame = trimstr(r); }
	inline void SetEpoch(const string& e) { m_epoch = trimstr(e); }
	inline void SetEpsg(const string& e) { m_epsgCode = trimstr(e); }

	string m_strName;

protected:

	string m_strOriginalName;

	double m_dXAxis;
	double m_dYAxis;
	double m_dZAxis;
	double m_dHeight;
	
	double m_dStdDevX;
	double m_dStdDevY;
	double m_dStdDevZ;
	double m_dStdDevHt;

	string m_strConstraints;
	string m_strType;
	string m_strHemisphereZone;
	string m_strDescription;
	string m_strComment;

	char m_cLatConstraint;
	char m_cLonConstraint;
	char m_cHtConstraint;

	COORD_TYPE m_ctType;
	COORD_TYPE m_ctTypeSupplied;
	HEIGHT_SYSTEM m_htType;

	double m_dcurrentLatitude;
	double m_dcurrentLongitude;
	double m_dcurrentHeight;
	float m_fgeoidSep;
	double m_dmeridianDef;
	double m_dverticalDef;
	UINT32 m_lfileOrder;
	UINT32 m_lnameOrder;
	UINT32 m_zone;
	bool m_unusedStation;

	string	m_referenceFrame;
	string	m_epsgCode;
	string	m_epoch;

	CONSTRAINT_TYPE m_constraintType;
};
	
}	// namespace measurements
}	// namespace dynadjust

#endif // ifndef DNASTATION_H
