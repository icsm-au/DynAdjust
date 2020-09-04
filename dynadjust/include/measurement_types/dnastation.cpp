//============================================================================
// Name         : dnastation.cpp
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
// Description  : DynAdjust CDnaStation implementation file
//============================================================================

#include <include/measurement_types/dnastation.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/exception/dnaexception.hpp>

using namespace std;
using namespace dynadjust::epsg;
using namespace dynadjust::datum_parameters;
using namespace dynadjust::exception;

namespace dynadjust {
namespace measurements {

// Comparison functions
// m_strName
bool operator<(const CDnaStation& left, const CDnaStation& right)
{
	return left.m_strName < right.m_strName;
}

bool operator<(const boost::shared_ptr<CDnaStation>& left, const boost::shared_ptr<CDnaStation>& right)
{
	return left.get()->m_strName < right.get()->m_strName;
}

CAStationList::CAStationList()
	: availMsrCount_ (0)
	, assocMsrCount_ (0)
	, amlStnIndex_ (0)
	, validStation_(VALID_STATION)
{
}

CAStationList::CAStationList(bool validStation)
	: availMsrCount_ (0)
	, assocMsrCount_ (0)
	, amlStnIndex_ (0)
	, validStation_(validStation ? VALID_STATION : INVALID_STATION)
{
}

CAStationList::CAStationList(const CAStationList& newAStnList)
{
	availMsrCount_ = newAStnList.availMsrCount_;
	assocMsrCount_ = newAStnList.assocMsrCount_;
	amlStnIndex_ = newAStnList.amlStnIndex_;
	validStation_ = newAStnList.validStation_;
}

CAStationList::CAStationList(const UINT32& assocMsrs, const UINT32& availMsrs, const UINT32& firstStnIndex, bool validStation)
{
	availMsrCount_ = availMsrs;
	assocMsrCount_ = assocMsrs;
	amlStnIndex_ = firstStnIndex;
	validStation_ = (validStation ? VALID_STATION : INVALID_STATION);

}
CAStationList::~CAStationList()
{

}

void CAStationList::coutStationListInfo()
{
	cout << setw(4) << assocMsrCount_ << setw(4) << amlStnIndex_ << endl;
}

CAStationList& CAStationList::operator =(const CAStationList& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	availMsrCount_ = rhs.availMsrCount_;
	assocMsrCount_ = rhs.assocMsrCount_;
	amlStnIndex_ = rhs.amlStnIndex_;
	validStation_ = rhs.validStation_;

	return *this;
}













//<CDnaStation>//
CDnaStation::CDnaStation(const string& referenceframe, const string& epoch)
	: m_strName("")
	, m_dXAxis(0.), m_dYAxis(0.), m_dZAxis(0.), m_dHeight(0.)
	, m_dStdDevX(0.), m_dStdDevY(0.), m_dStdDevZ(0.), m_dStdDevHt(0.)
	, m_strConstraints("FFF"), m_strType(""), m_strHemisphereZone(""), m_strDescription(""), m_strComment("")
	, m_cLatConstraint('F'), m_cLonConstraint('F'), m_cHtConstraint('F')
	, m_ctType(LLH_type_i), m_ctTypeSupplied(LLH_type_i), m_htType(ORTHOMETRIC_type_i)
	, m_dcurrentLatitude(0.), m_dcurrentLongitude(0.), m_dcurrentHeight(0.)
	, m_fgeoidSep(0.), m_dmeridianDef(0.), m_dverticalDef(0.)
	, m_lfileOrder(0), m_lnameOrder(0)
	, m_zone(0), m_unusedStation(INVALID_STATION)
	, m_referenceFrame(referenceframe), m_epoch(epoch)
{
	m_epsgCode = epsgStringFromName<string>(referenceframe);
}

CDnaStation::~CDnaStation()
{
}

// copy constructors
CDnaStation::CDnaStation(const CDnaStation& newStation)
{
	m_strName = newStation.m_strName;
	m_strConstraints = newStation.m_strConstraints;
	m_strType = newStation.m_strType;
	m_dXAxis = newStation.m_dXAxis;
	m_dYAxis = newStation.m_dYAxis;
	m_dZAxis = newStation.m_dZAxis;
	m_dHeight = newStation.m_dHeight;
	m_dStdDevX = newStation.m_dStdDevX;
	m_dStdDevY = newStation.m_dStdDevY;
	m_dStdDevZ = newStation.m_dStdDevZ;
	m_dStdDevHt = newStation.m_dStdDevHt;
	m_strHemisphereZone = newStation.m_strHemisphereZone;
	m_strDescription = newStation.m_strDescription;
	m_cLatConstraint = newStation.m_cLatConstraint;
	m_cLonConstraint = newStation.m_cLonConstraint;
	m_cHtConstraint = newStation.m_cHtConstraint;
	m_strComment = newStation.m_strComment;

	m_ctTypeSupplied = newStation.m_ctTypeSupplied;
	m_ctType = newStation.m_ctType;
	m_htType = newStation.m_htType;

	m_dcurrentLatitude = newStation.m_dcurrentLatitude;
	m_dcurrentLongitude = newStation.m_dcurrentLongitude;
	m_dcurrentHeight = newStation.m_dcurrentHeight;
	m_fgeoidSep = newStation.m_fgeoidSep;
	m_dmeridianDef = newStation.m_dmeridianDef;
	m_dverticalDef = newStation.m_dverticalDef;
	m_lfileOrder = newStation.m_lfileOrder;
	m_lnameOrder = newStation.m_lnameOrder;
	m_zone = newStation.m_zone;
	m_unusedStation = newStation.m_unusedStation;

	m_referenceFrame = newStation.m_referenceFrame;
	m_epsgCode = newStation.m_epsgCode;
	m_epoch = newStation.m_epoch;
}

CDnaStation::CDnaStation(const string& strName, const string& strConstraints, 
	const string& strType, const double& dXAxis, const double& dYAxis, const double& dZAxis,
	const double& dHeight, const string& strHemisphereZone, const string& strDescription, 
	const string& strComment)
{
	m_strName = strName;
	
	SetConstraints(strConstraints);
	SetCoordType(strType);

	m_dXAxis = dXAxis;
	m_dYAxis = dYAxis;
	m_dZAxis = dZAxis;
	m_dHeight = dHeight;

	m_dStdDevX = PRECISION_1E5;
	m_dStdDevY = PRECISION_1E5;
	m_dStdDevZ = PRECISION_1E5;
	m_dStdDevHt = PRECISION_1E4;

	if (strHemisphereZone.empty())
		m_zone = 0;
	else
		SetHemisphereZone(strHemisphereZone);

	m_strDescription = strDescription;
	m_strComment = strComment;
	m_unusedStation = INVALID_STATION;

	m_dcurrentLatitude = 0.;
	m_dcurrentLongitude = 0.;
	m_dcurrentHeight = 0.;
	m_fgeoidSep = 0.;
	m_dmeridianDef = 0.;
	m_dverticalDef = 0.;

	m_referenceFrame = DEFAULT_DATUM;
	m_epsgCode = DEFAULT_EPSG_S;
	m_epoch = "";
}

CDnaStation& CDnaStation::operator =(const CDnaStation& rhs)
{
	// check for assignment to self!
	if (this == &rhs)
		return *this;

	m_strName = rhs.m_strName;
	m_strConstraints = rhs.m_strConstraints;
	m_strType = rhs.m_strType;
	m_dXAxis = rhs.m_dXAxis;
	m_dYAxis = rhs.m_dYAxis;
	m_dZAxis = rhs.m_dZAxis;
	m_dHeight = rhs.m_dHeight;
	m_strHemisphereZone = rhs.m_strHemisphereZone;
	m_strDescription = rhs.m_strDescription;
	m_cLatConstraint = rhs.m_cLatConstraint;
	m_cLonConstraint = rhs.m_cLonConstraint;
	m_cHtConstraint = rhs.m_cHtConstraint;
	m_strComment = rhs.m_strComment;

	m_ctTypeSupplied = rhs.m_ctTypeSupplied;
	m_ctType = rhs.m_ctType;
	m_htType = rhs.m_htType;

	// measurement file extras
	m_dStdDevX = rhs.m_dStdDevX;
	m_dStdDevY = rhs.m_dStdDevY;
	m_dStdDevHt = rhs.m_dStdDevHt;

	m_dcurrentLatitude = rhs.m_dcurrentLatitude;
	m_dcurrentLongitude = rhs.m_dcurrentLongitude;
	m_dcurrentHeight = rhs.m_dcurrentHeight;
	m_fgeoidSep = rhs.m_fgeoidSep;
	m_dmeridianDef = rhs.m_dmeridianDef;
	m_dverticalDef = rhs.m_dverticalDef;
	m_lfileOrder = rhs.m_lfileOrder;
	m_lnameOrder = rhs.m_lnameOrder;
	m_zone = rhs.m_zone;

	m_unusedStation = rhs.m_unusedStation;

	m_referenceFrame = rhs.m_referenceFrame;
	m_epsgCode = rhs.m_epsgCode;
	m_epoch = rhs.m_epoch;
	
	return *this;
}


void CDnaStation::SetConstraints(const string& sConstraints)
{
	m_strConstraints = trimstr(sConstraints);
	m_cLatConstraint = (char)(*sConstraints.substr(0, 1).c_str());
	m_cLonConstraint = (char)(*sConstraints.substr(1, 1).c_str());
	m_cHtConstraint = (char)(*sConstraints.substr(2, 1).c_str());

}

void CDnaStation::SetConstraints(const char& cLatConstraint, const char& cLonConstraint, const char& cHtConstraint)
{
	m_cLatConstraint = cLatConstraint;
	m_cLonConstraint = cLonConstraint;
	m_cHtConstraint = cHtConstraint;
	UpdateConstraintsString();
}

void CDnaStation::SetLatConstraint(const char& cLatConstraint)
{
	m_cLatConstraint = cLatConstraint;
	UpdateConstraintsString();
}

void CDnaStation::SetLonConstraint(const char& cLonConstraint)
{
	m_cLonConstraint = cLonConstraint;
	UpdateConstraintsString();
}

void CDnaStation::SetHtConstraint(const char& cHtConstraint)
{
	m_cHtConstraint = cHtConstraint;
	UpdateConstraintsString();
}

void CDnaStation::SetConstraints(const double& dLatConstraint, const double& dLonConstraint, const double& dHtConstraint)
{
	if (dLatConstraint <= 0.1)
		m_cLatConstraint = 'C';
	else
		m_cLatConstraint = 'F';

	if (dLonConstraint <= 0.1)
		m_cLonConstraint = 'C';
	else
		m_cLonConstraint = 'F';

	if (dHtConstraint <= 0.1)
		m_cHtConstraint = 'C';
	else
		m_cHtConstraint = 'F';


	UpdateConstraintsString();
}

void CDnaStation::SetConstraints(const double& dLatConstraint, const double& dLonConstraint)
{
	if (dLatConstraint <= 0.1)
		m_cLatConstraint = 'C';
	else
		m_cLatConstraint = 'F';

	if (dLonConstraint <= 0.1)
		m_cLonConstraint = 'C';
	else
		m_cLonConstraint = 'F';

	UpdateConstraintsString();
}

void CDnaStation::SetConstraints(const double& dHtConstraint)
{
	if (dHtConstraint <= 0.1)
		m_cHtConstraint = 'C';
	else
		m_cHtConstraint = 'F';

	UpdateConstraintsString();
}



void CDnaStation::UpdateConstraintsString()
{
	ostringstream stream;
	stream << m_cLatConstraint << m_cLonConstraint << m_cHtConstraint;
	m_strConstraints = stream.str();
	//m_strConstraints.Format("%c%c%c", m_cLatConstraint, m_cLonConstraint, m_cHtConstraint);
}
	

void CDnaStation::SetXAxis(const char& cHemi, string strLat, string strMin, string strSec)
{
	string strString;
	strLat = trimstrleft(strLat);
	strMin = trimstrleft(strMin);
	strSec = trimstrleft(strSec);

	ostringstream s;

	bool bNegative = false;
	int nDeg = LongFromString<UINT32>(strLat);
	int nMin = LongFromString<UINT32>(strMin);
	double dSec = DoubleFromString<double>(strSec);

	// Hemisphere
	if (cHemi == ' ' || cHemi == 's' || cHemi == 'S')
		bNegative = true;

	// Degrees
	if (nDeg == 0)
	{
		if (nMin < 10)
			s << "0.0" << nMin;		// strString.Format("0.0%1d", nMin);
		else
			s << "0." << nMin;		// strString.Format("0.%2d", nMin);
		strString = s.str();
	}
	else
	{
		if (nMin < 10)
			s << nDeg << ".0" << nMin;		// strString.Format("%2d.0%1d", nDeg, nMin);
		else
			s << nDeg << "." << nMin;		// strString.Format("%2d.%2d", nDeg, nMin);
		strString = s.str();
	}

	strString = trimstr(strString);

	if (bNegative)
		strString.insert(0, "-");

	// Format seconds without decimal place
	// First, compute number of decimal places
	size_t nDecPlaces;
	size_t dp;
	s.precision(0);

	if ((dp = strSec.find(".")) == string::npos)
	{
		if (dSec < 10.0)
			s << "0" << dSec;		// strSec.Format("0%1.0f�", dSec);
		else
			s << dSec;			// strSec.Format("%2.0f�", dSec);
	}
	else
	{
		nDecPlaces = dp;
		nDecPlaces = strSec.length() - nDecPlaces - 1;

		// extract the decimal place
		double dFactor = pow(10., static_cast<int> (nDecPlaces));
		double dTemp = dFactor * dSec;
		double d1onFactor = 1.0 / dFactor;

		if ((dSec + d1onFactor) < 1.0)
			s << "00" << dTemp;		// strSec.Format("00%.0f", dTemp);
		else if ((dSec + d1onFactor) < 10.0)
			s << "0" << dTemp;		// strSec.Format("0%.0f", dTemp);
		else
			s << dTemp;				// strSec.Format("%.0f", dTemp);
		strSec = s.str();

		size_t nLength = strSec.length();
		if (nLength < nDecPlaces + 2)
		{
			string sPadding(2 + nDecPlaces - nLength, '0');
			strSec = sPadding + strSec;
		}
	}

	strString += strSec;
	SetXAxis(strString);
}


void CDnaStation::SetYAxis(const char& cHemi, string strLon, string strMin, string strSec)
{
	string strString;
	strLon = trimstrleft(strLon);
	strMin = trimstrleft(strMin);
	strSec = trimstrleft(strSec);

	ostringstream s;

	bool bNegative = false;
	int nDeg = LongFromString<UINT32>(strLon);
	int nMin = LongFromString<UINT32>(strMin);
	double dSec = DoubleFromString<double>(strSec);

	// Hemisphere
	if (cHemi == 'w' || cHemi == 'W')
		bNegative = true;

	// Degrees
	if (nDeg == 0)
	{
		if (nMin < 10)
			s << "0.0" << nMin;		// strString.Format("0.0%1d", nMin);
		else
			s << "0." << nMin;		// strString.Format("0.%2d", nMin);
		strString = s.str();
	}
	else
	{
		if (nMin < 10)
			s << nDeg << ".0" << nMin;		// strString.Format("%3d.0%1d", nDeg, nMin);
		else
			s << nDeg << "." << nMin;		// strString.Format("%3d.%2d", nDeg, nMin);
		strString = s.str();
	}

	strString = trimstr(strString);

	if (bNegative)
		strString.insert(0, "-");

	// Format seconds without decimal place
	// First, compute number of decimal places
	size_t nDecPlaces;
	size_t dp;
	s.precision(0);

	if ((dp = strSec.find(".")) == string::npos)
	{
		if (dSec < 10.0)
			s << "0" << dSec;		// strSec.Format("0%1.0f�", dSec);
		else
			s << dSec;			// strSec.Format("%2.0f�", dSec);
	}
	else
	{
		nDecPlaces = dp;
		nDecPlaces = strSec.length() - nDecPlaces - 1;

		// extract the decimal place
		double dFactor = pow(10., static_cast<int> (nDecPlaces));
		double dTemp = dFactor * dSec;
		double d1onFactor = 1.0 / dFactor;

		if ((dSec + d1onFactor) < 1.0)
			s << "00" << dTemp;		// strSec.Format("00%.0f", dTemp);
		else if ((dSec + d1onFactor) < 10.0)
			s << "0" << dTemp;		// strSec.Format("0%.0f", dTemp);
		else
			s << dTemp;				// strSec.Format("%.0f", dTemp);
		strSec = s.str();

		size_t nLength = strSec.length();
		if (nLength < nDecPlaces + 2)
		{
			string sPadding(2 + nDecPlaces - nLength, '0');
			strSec = sPadding + strSec;
		}
	}

	strString += strSec;
	SetYAxis(strString);
}


void CDnaStation::SetCoordType(const string& sType) {
	m_strType = trimstr(sType);
	m_ctTypeSupplied = m_ctType = GetMyCoordTypeC();	
}

// X, Easting, Latitude
void CDnaStation::SetXAxis(const string& str)
{
	switch (m_ctType)
	{
	case LLH_type_i:
	case LLh_type_i:
		FromDmsString(&m_dXAxis, trimstr(str));
		m_dXAxis = Radians(m_dXAxis);
		break;
	default:
		// All other types will be converted by dna_import::ReduceStations_LLH()
		DoubleFromString(m_dXAxis, trimstr(str));
	}
}


// Y, Northing, Longitude
void CDnaStation::SetYAxis(const string& str)
{
	switch (m_ctType)
	{
	case LLH_type_i:
	case LLh_type_i:
		FromDmsString(&m_dYAxis, trimstr(str));
		m_dYAxis = Radians(m_dYAxis);
		break;
	default:
		DoubleFromString(m_dYAxis, trimstr(str));
	}
}


// Z
void CDnaStation::SetZAxis(const string& str)
{
	DoubleFromString(m_dZAxis, trimstr(str));
}


// Height
void CDnaStation::SetHeight(const string& str)
{
	if (GetMyCoordTypeC() == XYZ_type_i)
		SetZAxis(str);
	else
		DoubleFromString(m_dHeight, trimstr(str));
}

// Hemisphere zone
void CDnaStation::SetHemisphereZone(const string& sHemisphereZone)
{
	m_strHemisphereZone = trimstr(sHemisphereZone);
	if (m_strHemisphereZone.empty())
		return;
	if (isdigit(m_strHemisphereZone.substr(0).c_str()[0]))					// is the first character a decimal digit?
		m_zone = LongFromString<UINT32>(m_strHemisphereZone.substr(0));		// extract zone from first position
	else
		m_zone = LongFromString<UINT32>(m_strHemisphereZone.substr(1));		// extract zone after hemisphere
}

// Reduce from Cartesian or Projection to Geographic
// m_dXAxis always contains X, Easting and Latitude
// m_dYAxis always contains Y, Northing and Longitude
void CDnaStation::ReduceStations_LLH(const CDnaEllipsoid* m_eEllipsoid, const CDnaProjection* m_pProjection)
{
	switch (m_ctType)
	{
	case XYZ_type_i:
		// Convert from cartesian to geographic (radians)
		CartToGeo<double>(m_dXAxis, m_dYAxis, m_dZAxis, &m_dXAxis, &m_dYAxis, &m_dHeight, m_eEllipsoid);
		m_dZAxis= 0.;
		m_htType = ELLIPSOIDAL_type_i;

		// Force the current type to be geographic (ellipsoid height).  
		// The user-supplied type is retained in m_ctTypeSupplied
		m_ctType = LLh_type_i;
		m_strType = LLh_type;
		break;
	case UTM_type_i:
		// Convert from projection to geographic (radians)
		GridToGeo<double, UINT32>(m_dXAxis, m_dYAxis, m_zone, &m_dXAxis, &m_dYAxis, 
			m_eEllipsoid->GetSemiMajor(), m_eEllipsoid->GetInverseFlattening(),		// ellipsoid parameters
			m_pProjection->GetFalseEasting(), m_pProjection->GetFalseNorthing(),	// projection parameters
			m_pProjection->GetCentralScaleFactor(), m_pProjection->GetLongCentralMeridianZone1(), 
			m_pProjection->GetZoneWidth());

		// Force the current type to be geographic (orthometric height).
		// The user-supplied type is retained in m_ctTypeSupplied
		m_ctType = LLH_type_i;
		m_strType = LLH_type;
		break;
	default:
		break;
	}
}

// Reduce from Geographic or Projection to Cartesian
// m_dXAxis always contains X, Easting and Latitude
// m_dYAxis always contains Y, Northing and Longitude
void CDnaStation::ReduceStations_XYZ(const CDnaEllipsoid* m_eEllipsoid, const CDnaProjection* m_pProjection)
{
	switch (m_ctType)
	{
	case UTM_type_i:
		// Convert from projection to geographic (radians)
		GridToGeo<double, UINT32>(m_dXAxis, m_dYAxis, m_zone, &m_dXAxis, &m_dYAxis, 
			m_eEllipsoid->GetSemiMajor(), m_eEllipsoid->GetInverseFlattening(),	// ellipsoid parameters
			m_pProjection->GetFalseEasting(), m_pProjection->GetFalseNorthing(),	// projection parameters
			m_pProjection->GetCentralScaleFactor(), m_pProjection->GetLongCentralMeridianZone1(), 
			m_pProjection->GetZoneWidth());
	case LLH_type_i:
	case LLh_type_i:
		m_dcurrentLatitude = m_dXAxis;
		m_dcurrentLongitude = m_dYAxis;
		m_dcurrentHeight = m_dHeight;
		if (m_ctType == LLH_type_i)
			m_dcurrentHeight += m_fgeoidSep;
		// Convert from geographic (radians) to cartesian
		// Assumes height is ellipsoidal
		GeoToCart<double>(m_dcurrentLatitude, m_dcurrentLongitude, m_dcurrentHeight, &m_dXAxis, &m_dYAxis, &m_dZAxis, m_eEllipsoid);
		break;
	default:
		break;
	}

	// Force the current type to be geographic.  The user-supplied type is retained in m_ctTypeSupplied
	m_ctType = XYZ_type_i;
	m_strType = XYZ_type;
}

bool CDnaStation::IsValidConstraint(const string& sConst)
{
	if (iequals(sConst, "CCC"))
		return true;
	if (iequals(sConst, "CCF"))
		return true;
	if (iequals(sConst, "CFF"))
		return true;
	if (iequals(sConst, "FFF"))
		return true;
	if (iequals(sConst, "FFC"))
		return true;
	if (iequals(sConst, "FCC"))
		return true;
	if (iequals(sConst, "FCF"))
		return true;
	if (iequals(sConst, "CFC"))
		return true;
	return false;
}
	

_COORD_TYPE_ CDnaStation::GetCoordTypeC(const string& sType)
{
	// case insensitive
	if (iequals(sType, XYZ_type))
		return XYZ_type_i;
	else if (iequals(sType, UTM_type))
		return UTM_type_i;				// height is assumed to be orthometric
	else if (iequals(sType, ENU_type))
		return ENU_type_i;

	// case sensitive
	else if (boost::equals(sType, LLh_type))
		return LLh_type_i;				// ellipsoid height
	
	// default
	else if (boost::equals(sType, LLH_type))
		return LLH_type_i;					// orthometric height (default)

	// If this point is reached, sType is an unknown coordinate type, so throw!
	stringstream ss;
	ss << "  '" << sType << "' is not a recognised coordinate type." << endl;
	throw boost::enable_current_exception(runtime_error(ss.str()));

	return LLH_type_i;
}


_COORD_TYPE_ CDnaStation::GetMyCoordTypeC() const
{
	return GetCoordTypeC(m_strType);
}


_HEIGHT_SYSTEM_ CDnaStation::GetHeightSystemC(const string& sType) const
{
	if (sType.compare(ELLIPSOIDAL_type) == 0)
		return ELLIPSOIDAL_type_i;
	return ORTHOMETRIC_type_i;		// default
}


_HEIGHT_SYSTEM_ CDnaStation::GetMyHeightSystemC() const
{
	if (m_strType.compare(LLh_type) == 0)
		return ELLIPSOIDAL_type_i;
	return ORTHOMETRIC_type_i;	// default
}

// SetHeightSystem called by void Height_pimpl::system(const ::std::string& system)
// where system is an attribute of the element Height (either "ellipsoidal" or "orthometric")
void CDnaStation::SetHeightSystem(const string& sType)
{
	SetHeightSystem(GetHeightSystemC(sType));
}
	
void CDnaStation::SetHeightSystem(const HEIGHT_SYSTEM& type_i)
{
	m_htType = type_i;

	switch (m_htType)
	{
	case ELLIPSOIDAL_type_i:
		// Convert to orthometric
		if (m_ctType == LLH_type_i)
			m_ctType = LLh_type_i;
		if (m_strType.compare(LLH_type))
			m_strType = LLh_type;
		break;
	case ORTHOMETRIC_type_i:
		// Convert to ellipsoidal
		if (m_ctType == LLh_type_i)
			m_ctType = LLH_type_i;
		if (m_strType.compare(LLh_type))
			m_strType = LLH_type;
		break;
	}
}

void CDnaStation::coutStationData(ostream &os, ostream &os2, const UINT16& uType) const
{
	UINT32 precision = 3;
	if (m_ctType == LLH_type_i)
		precision = 10;
	stringstream ss;
	string str;
	size_t dot;

	switch (uType)
	{
	case DNA_COUT:
	case GEOLAB_COUT:
		
		os << "+ " << setw(16) << m_strName;
		os << setw(4) << m_strConstraints;
		os << setw(4) << m_strType;
		os << setw(20) << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? RadtoDms(m_dXAxis): m_dXAxis);
		os << setw(20) << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? RadtoDmsL(m_dYAxis): m_dYAxis);
		//os << setw(16) << vStations[s].GetZAxis();
		os << setw(13) << setprecision(4) << fixed << m_dHeight;
		//os << setw(10) << vStations[s].GetRedHeight();
		os << setw(5) << m_strHemisphereZone;
		os << setw(6) << m_lnameOrder;
		os << setw(19) << m_strDescription;
		//os << setw(10) << vStations[s].GetComment();
		os << endl;
		break;
	case NEWGAN_COUT:
		os << setw(3) << "4";
		os << right << setw(12) << m_strName;
		os << " ";
		os << left << setw(23) << m_strDescription.substr(0, 23);
		ss.str("");
		ss << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? RadtoDms(m_dXAxis): m_dXAxis);
		str = trimstr(ss.str());
		dot = str.find(".");
		str.replace(dot, 1, " ");
		str.insert(dot+5, ".");
		if (m_dXAxis < 0)
		{
			str.replace(0, 1, " ");		// replace negative sign
			str = trimstr(str);
			os << left << "S" << setw(precision + 4) << right << str;
		}
		else
			os << left << "N" << setw(precision + 4) << right << str;
		
		ss.str("");
		ss << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? RadtoDmsL(m_dYAxis): m_dYAxis);
		str = trimstr(ss.str());
		dot = str.find(".");
		str.replace(dot, 1, " ");
		str.insert(dot+5, ".");
		if (m_dYAxis < 0)
		{
			str.replace(0, 1, " ");		// replace negative sign
			str = trimstr(str);
			os << left << "W" << setw(precision + 5) << right << str;
		}
		else
			os << left << "E" << setw(precision + 5) << right << str;
		
		
		os << right << setw(9) << setprecision(3) << fixed << m_dHeight;
		os << endl;
		break;
	case GMT_OUT:
		os << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? DegreesL(m_dYAxis): m_dYAxis);
		os << "  ";
		os << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? Degrees(m_dXAxis): m_dXAxis);
		os << endl;

		os2 << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? DegreesL(m_dYAxis): m_dYAxis);
		os2 << "  ";
		os2 << setprecision(precision) << fixed << (m_ctType == LLH_type_i ? Degrees(m_dXAxis): m_dXAxis);
		os2 << "  6 0 0 LM " << m_strName << endl;		
		break;
	}
	

}


void CDnaStation::WriteBinaryStn(std::ofstream* binary_stream, const UINT16 bUnused)
{
	station_t stationRecord;
	strcpy(stationRecord.stationName, m_strName.substr(0, STN_NAME_WIDTH).c_str());
	strcpy(stationRecord.stationConst, m_strConstraints.substr(0, STN_CONST_WIDTH).c_str());
	strcpy(stationRecord.stationType, m_strType.substr(0, STN_TYPE_WIDTH).c_str());

	stationRecord.suppliedStationType = m_ctTypeSupplied;

	switch (m_ctTypeSupplied)
	{
	case LLH_type_i:
	case UTM_type_i:
		stationRecord.suppliedHeightRefFrame = ORTHOMETRIC_type_i;
		break;
	default:
		break;
	}

	stationRecord.initialLatitude = m_dXAxis;
	if (fabs(m_dcurrentLatitude - 0.) < PRECISION_1E15)
		stationRecord.currentLatitude = m_dXAxis;
	else
		stationRecord.currentLatitude = m_dcurrentLatitude;
	
	stationRecord.initialLongitude = m_dYAxis;
	if (fabs(m_dcurrentLongitude - 0.) < PRECISION_1E15)
		stationRecord.currentLongitude = m_dYAxis;
	else
		stationRecord.currentLongitude = m_dcurrentLongitude;

	stationRecord.initialHeight = m_dHeight;	
	if (fabs(m_dcurrentHeight - 0.) < PRECISION_1E15)
		stationRecord.currentHeight = m_dHeight;
	else
		stationRecord.currentHeight = m_dcurrentHeight;
		
	stationRecord.geoidSep = m_fgeoidSep;
	stationRecord.meridianDef = m_dmeridianDef;
	stationRecord.verticalDef = m_dverticalDef;
	stationRecord.zone = m_zone;
	strcpy(stationRecord.description, m_strDescription.substr(0, STN_DESC_WIDTH).c_str());
	stationRecord.fileOrder = m_lfileOrder;
	stationRecord.nameOrder = m_lnameOrder;
	stationRecord.unusedStation = bUnused;

	strcpy(stationRecord.epoch, m_epoch.substr(0, STN_EPOCH_WIDTH).c_str());
	strcpy(stationRecord.epsgCode, m_epsgCode.substr(0, STN_EPSG_WIDTH).c_str());

	binary_stream->write(reinterpret_cast<char *>(&stationRecord), sizeof(station_t));
}


void CDnaStation::WriteDNAXMLStnCurrentEstimates(std::ofstream* dna_ofstream, 
	const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
	INPUT_FILE_TYPE t, const dna_stn_fields* dsw)
{
	// Get the latest estimates
	double lat_east_x(m_dcurrentLatitude);
	double lon_north_y(m_dcurrentLongitude);
	double ht_zone_z(m_dcurrentHeight);

	// Write latest station estimates
	WriteDNAXMLStn(dna_ofstream,
		ellipsoid, projection,
		lat_east_x, lon_north_y, ht_zone_z,
		t, dsw);
}
	

void CDnaStation::WriteDNAXMLStnInitialEstimates(std::ofstream* dna_ofstream, 
	const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
	INPUT_FILE_TYPE t, const dna_stn_fields* dsw)
{
	// Write initial station estimates
	WriteDNAXMLStn(dna_ofstream,
		ellipsoid, projection,
		m_dXAxis, m_dYAxis, m_dHeight,
		t, dsw);
}


void CDnaStation::WriteDNAXMLStn(std::ofstream* dna_ofstream,
	const CDnaEllipsoid* ellipsoid, const CDnaProjection* projection,
	const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
	INPUT_FILE_TYPE t, const dna_stn_fields* dsw)
{
	//m_ctTypeSupplied = LLh_type_i;

	string hemisphereZone(m_strHemisphereZone);
	string coordinateType(LLH_type);

	switch (m_ctTypeSupplied)
	{
	case XYZ_type_i:
		coordinateType = XYZ_type;
		break;
	case LLh_type_i:
		coordinateType = LLh_type;
		break;
	case UTM_type_i:
		coordinateType = UTM_type;
		break;
	case ENU_type_i:	// Not supported yet, so revert to Lat, Long, Height
		coordinateType = LLH_type;
		m_ctTypeSupplied = LLH_type_i;
		break;
	case LLH_type_i:
		coordinateType = LLH_type;
		break;
	default:
		break;
	}

	// Convert height to orthometric?
	// Decisions for printing (estimated) station height for various input coordinate types
	// and geoid model.  Estimated stations are in LLh.
	// h = H + Na
	//
	// Input Coord type		Geoid loaded?          Geoid not loaded?          Result
	// ---------------------------------------------------------------------------------
	// LLH				    ht_z - m_fgeoidSep     ht_z (m_fgeoidSep = 0)     H (orthometric)
	// UTM					ht_z - m_fgeoidSep     ht_z (m_fgeoidSep = 0)     H (orthometric)
	// ENU					ht_z - m_fgeoidSep     ht_z (m_fgeoidSep = 0)     H (orthometric)
	// LLh          	    ht_z			       ht_z					      h (ellipsoid)
	// XYZ          	    LLh->XYZ		       LLh->XYZ					  X, Y, Z
	double lat_east_x_mod(lat_east_x);
	double lon_north_y_mod(lon_north_y);
	double ht_zone_z_mod(ht_zone_z);

	switch (m_ctTypeSupplied)
	{
	case LLH_type_i:
	case UTM_type_i:
		// reduce to orthometric.  
		// If geoid hasn't been loaded, m_fgeoidSep will be 0.0
		ht_zone_z_mod -= m_fgeoidSep;
		break;
	default:
		break;
	}

	// Convert coordinates to cartesian or utm?
	// Convert radians values to degrees, minutes and seconds?
	stringstream ss;
	double zone;
	switch (m_ctTypeSupplied)
	{
	case XYZ_type_i:
		GeoToCart<double>(lat_east_x, lon_north_y, ht_zone_z, 
			&lat_east_x_mod, &lon_north_y_mod, &ht_zone_z_mod, ellipsoid);
		break;
	
	case UTM_type_i:
		GeoToGrid<double>(lat_east_x, lon_north_y, 
			&lat_east_x_mod, &lon_north_y_mod, &zone, ellipsoid, projection, true);
		
		ss << fixed << setprecision(0) << zone;
		hemisphereZone = ss.str();	
		break;

	case LLh_type_i:
	case LLH_type_i:
		lat_east_x_mod = RadtoDms(lat_east_x);
		lon_north_y_mod = RadtoDmsL(lon_north_y);
		break;
	default:
		break;
	}

	switch (t)
	{
	case dna:
	
		// Write DNA station file
		WriteDNAStn(dna_ofstream, coordinateType,
			lat_east_x_mod, lon_north_y_mod, ht_zone_z_mod, 
			hemisphereZone, *dsw);
		break;

	case dynaml:

		// Write DynAdjust XML station file
		WriteDynaMLStn(dna_ofstream, coordinateType,
			lat_east_x_mod, lon_north_y_mod, ht_zone_z_mod,
			hemisphereZone);
		break;
	default:
		break;
	}
}

void CDnaStation::WriteDNAStn(std::ofstream* dna_ofstream, const string& coordinateType, 
	const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
	string& hemisphereZone, const dna_stn_fields& dsw)
{	
	UINT32 LEX_precision(4), LNY_precision(4), HZ_precision(4);	
	
	switch (m_ctTypeSupplied)
	{
	case LLh_type_i:
	case LLH_type_i:
		LEX_precision = LNY_precision = 10;
		break;
	default:
		break;
	}

	(*dna_ofstream) << left << setw(dsw.stn_name) << m_strName;
	(*dna_ofstream) << left << setw(dsw.stn_const) << m_strConstraints;
	(*dna_ofstream) << " ";
	(*dna_ofstream) << left << setw(dsw.stn_type) << coordinateType;
	(*dna_ofstream) << right << setw(dsw.stn_e_phi_x) << setprecision(LEX_precision) << fixed << lat_east_x;
	(*dna_ofstream) << right << setw(dsw.stn_n_lam_y) << setprecision(LNY_precision) << fixed << lon_north_y;
	(*dna_ofstream) << right << setw(dsw.stn_ht_z) << setprecision(HZ_precision) << fixed << ht_zone_z;	
	if (m_ctTypeSupplied == UTM_type_i)
		(*dna_ofstream) << right << setw(dsw.stn_hemi_zo) << hemisphereZone;
	else
		(*dna_ofstream) << right << setw(dsw.stn_hemi_zo) << " ";
	(*dna_ofstream) << " ";
	(*dna_ofstream) << left << m_strDescription << endl;
}
	

void CDnaStation::WriteDynaMLStn(std::ofstream* xml_ofstream, const string& coordinateType, 
	const double& lat_east_x, const double& lon_north_y, const double& ht_zone_z,
	string& hemisphereZone)
{
	UINT32 LEX_precision(4), LNY_precision(4), HZ_precision(4);
	
	// Convert radians values to degrees, minutes and seconds
	switch (m_ctTypeSupplied)
	{
	case LLh_type_i:
	case LLH_type_i:
		LEX_precision = LNY_precision = 10;
		break;
	default:
		break;
	}
	
	(*xml_ofstream) << "  <DnaStation>" << endl;
	(*xml_ofstream) << "    <Name>" << m_strName << "</Name>" << endl;
	(*xml_ofstream) << "    <Constraints>" << m_strConstraints << "</Constraints>" << endl;
	(*xml_ofstream) << "    <Type>" << coordinateType << "</Type>" << endl;
	(*xml_ofstream) << "    <StationCoord>" << endl;
	(*xml_ofstream) << "      <Name>" << m_strName << "</Name>" << endl;
	(*xml_ofstream) << "      <XAxis>" << setprecision(LEX_precision) << fixed << lat_east_x << "</XAxis>" << endl;
	(*xml_ofstream) << "      <YAxis>" << setprecision(LNY_precision) << fixed << lon_north_y << "</YAxis>" << endl;
	(*xml_ofstream) << "      <Height>" << setprecision(HZ_precision) << fixed << ht_zone_z << "</Height>" << endl;
	
	// Convert radians values to degrees, minutes and seconds
	if (m_ctTypeSupplied == UTM_type_i)
		(*xml_ofstream) << "      <HemisphereZone>" << hemisphereZone << "</HemisphereZone>" << endl;
	
	(*xml_ofstream) << "    </StationCoord>" << endl;
	(*xml_ofstream) << "    <Description>"  << m_strDescription << "</Description>" << endl;
	(*xml_ofstream) << "  </DnaStation>" << endl;
}
	

void CDnaStation::WriteGeoidfile(std::ofstream* geo_ofstream)
{
	(*geo_ofstream) << setw(44) << left << m_strName <<
		setw(15) << setprecision(3) << fixed << left << m_fgeoidSep <<
		setw(10) << setprecision(3) << fixed << right << m_dmeridianDef <<
		setw(10) << setprecision(3) << fixed << right << m_dverticalDef << endl;
}
	

void CDnaStation::SetStationRec(const station_t& stationRecord)
{
	SetName(stationRecord.stationName);
	SetConstraints(stationRecord.stationConst);
	SetCoordType(stationRecord.stationType);
	
	switch (stationRecord.suppliedStationType)
	{
	case UTM_type_i:
		m_ctTypeSupplied = UTM_type_i;
		break;
	case XYZ_type_i:
		m_ctTypeSupplied = XYZ_type_i;
		break;
	case ENU_type_i:
		m_ctTypeSupplied = ENU_type_i;
		break;
	case LLh_type_i:
		m_ctTypeSupplied = LLh_type_i;
		break;
	default:
	case LLH_type_i:
		m_ctTypeSupplied = LLH_type_i;
		break;
	}
	
	m_dXAxis = stationRecord.initialLatitude;
	m_dYAxis = stationRecord.initialLongitude;
	m_dHeight = stationRecord.initialHeight;
	m_dcurrentLatitude = stationRecord.currentLatitude;
	m_dcurrentLongitude = stationRecord.currentLongitude;
	m_dcurrentHeight = stationRecord.currentHeight;
	m_fgeoidSep = stationRecord.geoidSep;
	m_dmeridianDef = stationRecord.meridianDef;
	m_dverticalDef = stationRecord.verticalDef;
	m_zone = stationRecord.zone;
	if (stationRecord.initialLatitude < 0.)
		m_strHemisphereZone = "S";
	else
		m_strHemisphereZone = "N";
	char zone[3];
	sprintf(zone, "%d", m_zone);
	m_strHemisphereZone += zone;
	SetDescription(stationRecord.description);
	m_lfileOrder = stationRecord.fileOrder;
	m_lnameOrder = stationRecord.nameOrder;
	m_unusedStation = (stationRecord.unusedStation == VALID_STATION ? true : false);

	m_epoch = stationRecord.epoch;
	m_epsgCode = stationRecord.epsgCode;
	m_referenceFrame = datumFromEpsgCode<string, UINT32>(LongFromString<UINT32>(m_epsgCode));
}

string CDnaStation::CoordinateName(const char& c)
{
	switch (c)
	{
	case 'P':
		return "Latitude";
	case 'L':
		return "Longitude";
	case 'H':
		return "H(Ortho)";
	case 'h':
		return "h(Ellipse)";
	case 'E':
		return "Easting";
	case 'N':
		return "Northing";
	case 'z':
		return "Zone";
	case 'X':
		return "X";
	case 'Y':
		return "Y";
	case 'Z':
		return "Z";
	}
	return "";
}

}	// namespace measurements
}	// namespace dynadjust
