//============================================================================
// Name         : dnatemplatestnmsrfuncs.hpp
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
// Description  : Common calculation functions using predefined station and
//				  measurement types
//============================================================================

#ifndef DNATEMPLATESTNMSRFUNCS_H_
#define DNATEMPLATESTNMSRFUNCS_H_

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
#include <include/functions/dnatemplatefuncs.hpp>
#include <include/config/dnatypes.hpp>
#include <include/measurement_types/dnameasurement_types.hpp>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace dynadjust::measurements;

template <typename T>
// Get all the statons asociated with a measurement
void GetGXMsrStations(vector<CDnaGpsBaseline>* vgpsBsls, vector<T>& msrStations)
{
	msrStations.clear();
			
	// Step 1. Set ignore flag for all baselines to true
	for_each(vgpsBsls->begin(), vgpsBsls->end(), 
		[&msrStations](CDnaGpsBaseline& bsl) {
			msrStations.push_back(bsl.GetFirst());
			msrStations.push_back(bsl.GetTarget());
	});

	// Strip duplicate entries
	strip_duplicates(msrStations);
}


template <typename T>
// Get all the statons asociated with a measurement
void GetMsrStations(const vmsr_t& binaryMsrs, const T& bmsIndex, vector<T>& msrStations)
{
	msrStations.clear();
	T clusterID(binaryMsrs.at(bmsIndex).clusterID);

	it_vmsr_t_const _it_msr_start = binaryMsrs.begin() + bmsIndex;
	it_vmsr_t_const _it_msr_prev = _it_msr_start;

	bool bCluster(false);

	// I don't think this is necessary for X measruements, since
	// bmsIndex points to the first element in a cluster.  Perhaps keep
	// this here to safe guard against cases where bmsIndex is not the
	// first???
	switch (binaryMsrs.at(bmsIndex).measType)
	{
	case 'D':
	case 'X':
	case 'Y':
		// move to the beginning of the cluster
		while (_it_msr_start != binaryMsrs.begin())
		{
			if ((--_it_msr_prev)->clusterID != clusterID)
				break;
			_it_msr_start--;
		}
		bCluster = true;
		break;
	}

	while (_it_msr_start->clusterID == clusterID)		// Will be true for non-cluster measurements
	{	
		if (_it_msr_start->measType != 'D')
		{
			if (_it_msr_start->measStart != xMeas)
			{
				if (++_it_msr_start == binaryMsrs.end())
					break;
				continue;
			}
		}		
		
		// add the relevant stations
		// Station 1 is in all measurement types
		msrStations.push_back(_it_msr_start->station1);
		// Station 2 is in two- and three-station measurements
		if (MsrTally::Stations(_it_msr_start->measType) >= TWO_STATION)
			msrStations.push_back(_it_msr_start->station2);
		// Station 3 is only in three-station measurements
		if (MsrTally::Stations(_it_msr_start->measType) == THREE_STATION)
			msrStations.push_back(_it_msr_start->station3);
	
		if (!bCluster)
			break;

		if (++_it_msr_start == binaryMsrs.end())
			break;
	}

	// Strip duplicate entries
	strip_duplicates(msrStations);
}

template <typename T>
// Get all the binary measurement indices involved in a measurement
// On return, msrIndices will contain only one index.  For clusters
// and directions, msrIndices will be more than one.
void GetMsrIndices(const vmsr_t& binaryMsrs, const T& bmsIndex, vector<T>& msrIndices)
{
	msrIndices.clear();

	// First, sift out conventional measurement types and return immediately.
	switch (binaryMsrs.at(bmsIndex).measType)
	{
	case 'D':
	case 'X':
	case 'Y':
		// Clusters dealt with after this switch statement
		break;
	default:
		// basic measurement type for which there is only one index, so
		// add the index and return
		msrIndices.push_back(bmsIndex);
		return;
	}

	// Ok, now deal with cluster measurements
	UINT32 clusterID(binaryMsrs.at(bmsIndex).clusterID);

	it_vmsr_t_const _it_msr_start(binaryMsrs.begin() + bmsIndex);
	it_vmsr_t_const _it_msr_prev(_it_msr_start);
	
	// move to the beginning of the cluster
	while (_it_msr_start != binaryMsrs.begin())
	{
		if ((--_it_msr_prev)->clusterID != clusterID)
			break;
		--_it_msr_start;
	}


	while (_it_msr_start->clusterID == clusterID && _it_msr_start != binaryMsrs.end())
	{
		if (_it_msr_start->ignore)
		{
			if (++_it_msr_start == binaryMsrs.end())
				break;
			continue;
		}

		if (_it_msr_start->measType != 'D')
		{
			if (_it_msr_start->measStart > xMeas)
			{
				if (++_it_msr_start == binaryMsrs.end())
					break;
				continue;
			}
		}		

		msrIndices.push_back(static_cast<T>(std::distance(binaryMsrs.begin(), _it_msr_start)));
	
		if (++_it_msr_start == binaryMsrs.end())
			break;
	}

	// Sort and strip duplicate entries
	strip_duplicates(msrIndices);
}

template <typename T>
// Get the first binary measurement index for a measurement
T GetFirstMsrIndex(const vmsr_t& binaryMsrs, const T& bmsIndex)
{
	// First, sift out conventional measurement types and return immediately.
	switch (binaryMsrs.at(bmsIndex).measType)
	{
	case 'D':
	case 'X':
	case 'Y':
		// Clusters dealt with after this switch statement
		break;
	default:
		// basic measurement type for which there is only one index, so
		// return bmsIndex
		return bmsIndex;
	}

	// Ok, now deal with cluster measurements
	UINT32 clusterID(binaryMsrs.at(bmsIndex).clusterID);

	it_vmsr_t_const _it_msr_start(binaryMsrs.begin() + bmsIndex);
	it_vmsr_t_const _it_msr_prev(_it_msr_start);
	
	// move to the beginning of the cluster
	while (_it_msr_start != binaryMsrs.begin())
	{
		if ((--_it_msr_prev)->clusterID != clusterID)
			break;
		--_it_msr_start;
	}

	// return the index of the measurement's first record in the binary measurement file 
	return static_cast<UINT32>(std::distance(binaryMsrs.begin(), _it_msr_start));
}

//template <typename T, typename Iter>
//void RenameStationsMsr(T* msr, Iter begin, Iter end)
//{
//	for (_it_string_vstring_pair it = begin;
//		it != end;
//		++it)
//	{
//
//	it_string_pair it;
//	it = binary_search_index_pair(begin, end, msr->GetFirst());
//	
//	if (it != end)
//		msr->SetFirst(it->second);
//		
//	// Does this measurement have a second station?
//	if (msr->m_MSmeasurementStations == ONE_STATION)
//		return;
//
//	// Okay, at least two stations
//	it = binary_search_index_pair(begin, end, msr->GetTarget());
//	if (it != end)
//		msr->SetTarget(it->second);
//			
//	// Does this measurement have a third station?
//	if (msr->m_MSmeasurementStations == TWO_STATION)
//		return;
//			
//	// Okay, three stations
//	it = binary_search_index_pair(begin, end, msr->GetTarget2());
//	if (it != end)
//		msr->SetTarget2(it->second);
//}
	

template <typename T, typename Iter>
void RenameStationsMsr(T* msr, Iter begin, Iter end)
{
	_it_string_vstring_pair it;
	Iter original = begin;
	
	// Search the aliases for the first station name
	// Emulate for_each behaviour
	while (begin != end) 
	{		
		if (binary_search(begin->second.begin(), begin->second.end(), msr->GetFirst()))
		{
			msr->SetFirst(begin->first);
			break;
		}
		++begin;
	}

	// Is this measurement a one-station measurement?
	if (msr->m_MSmeasurementStations == ONE_STATION)
		return;

	// Search the aliases for the second station name
	// Emulate for_each behaviour
	begin = original;
	while (begin != end) 
	{		
		if (binary_search(begin->second.begin(), begin->second.end(), msr->GetTarget()))
		{
			msr->SetTarget(begin->first);
			break;
		}
		++begin;
	}

	// Is this measurement a two-station measurement?
	if (msr->m_MSmeasurementStations == TWO_STATION)
		return;

	// Search the aliases for the third station name
	// Emulate for_each behaviour
	begin = original;
	while (begin != end) 
	{		
		if (binary_search(begin->second.begin(), begin->second.end(), msr->GetTarget2()))
		{
			msr->SetTarget2(begin->first);
			break;
		}
		++begin;
	}
}

template <typename T, typename Iter>
void IgnoreGXMeasurements(T* msr, Iter begin, Iter end)
{
	
	// Search the aliases for the first station name
	// Emulate for_each behaviour	
	if (binary_search(begin, end, msr->GetClusterID()))
	{
		msr->SetIgnore(true);	
		for_each(msr->GetBaselines_ptr()->begin(),
			msr->GetBaselines_ptr()->end(),
			[] (CDnaGpsBaseline& bsl) {
				bsl.SetIgnore(true);
		});
	}
}


template <typename T, typename msriterator>
// Copy the cluster measurement 
void CopyClusterMsr(T& cluster, const msriterator _it_msr, T& clusterCopy)
{
	clusterCopy.clear();

	// First, sift out conventional measurement types and return immediately.
	switch (_it_msr->measType)
	{
	case 'D':
	case 'X':
	case 'Y':
		// Clusters dealt with after this switch statement
		break;
	default:
		// basic measurement type for which there is only one element
		clusterCopy.push_back(*_it_msr);
		return;
	}

	// Ok, now deal with cluster measurements
	UINT32 clusterID(_it_msr->clusterID);

	msriterator _it_msr_start(_it_msr);
	msriterator _it_msr_temp(_it_msr_start);
	
	while (_it_msr_start != cluster.begin())
	{
		if ((--_it_msr_temp)->clusterID != clusterID)
			break;
		--_it_msr_start;
	}

	msriterator _it_msr_last(_it_msr);

	while (_it_msr_last != cluster.end())
	{
		if ((++_it_msr_temp)->measType != 'Y')
		{
			++_it_msr_last;
			break;
		}
		++_it_msr_last;
	}

	clusterCopy.insert(clusterCopy.begin(), _it_msr_start, _it_msr_last);
}


// Comparison functions
template <typename T = dnaStnPtr, typename S = string>
class CompareStationName {
public:
	bool operator()(const T& left, const T& right) {
		if (left->GetName() == right->GetName())
			return (left->GetfileOrder() < right->GetfileOrder());
		return left->GetName() < right->GetName();
	}
	bool operator()(const T& left, const S& right) {
		return left->GetName() < right;
	}
	bool operator()(const S& left, const T& right) {
		return left < right->GetName();
	}
};

template <typename T = dnaStnPtr, typename S = string>
class EqualStationNameSaveDuplicates {
public:
	EqualStationNameSaveDuplicates(vector<S>* stns) : _stns(stns) {}

	bool operator()(const T& left, const T& right) {
		if (equals(left->GetName(), right->GetName()))
			_stns->push_back(right->GetName());

		return (left->GetName() == right->GetName());
	}

	vector<S>* _stns;
};


template <typename T = dnaStnPtr>
class EqualStationName {
public:
	bool operator()(const T& left, const T& right) {
		return (left->GetName() == right->GetName());
	}
};


template <typename T = dnaStnPtr, typename S = string>
class EqualStationName_CaseInsensitive {
public:
	EqualStationName_CaseInsensitive(vector<S>* stns) : _stns(stns) {}

	bool operator()(const T& left, const T& right) {
		if (iequals(left->GetName(), right->GetName()))
		{
			_stns->push_back(right->GetName());
			return true;
		}
		return false;
	}
	vector<S>* _stns;
};


template <typename T = dnaStnPtr, typename S = string>
class TestEqualStationName {
public:
	TestEqualStationName(vector<S>* usedStns, vector<S>* unusedStns) 
		: _usedStns(usedStns)
		, _unusedStns(unusedStns) {}

	bool operator()(const T& stn) {
		// Is stn in the list of stations to keep?  If so, return false
		if (binary_search(_usedStns->begin(), _usedStns->end(), stn->GetName()))
			return true;
		
		// Station is not in the list, so add it to the unused stations
		_unusedStns->push_back(stn->GetName());
		return false;		
	}
	vector<S>* _usedStns;
	vector<S>* _unusedStns;
};


template <typename T = dnaStnPtr, typename S = string>
class TestNotEqualStationName {
public:
	TestNotEqualStationName(vector<S>* usedStns, vector<S>* unusedStns) 
		: _usedStns(usedStns)
		, _unusedStns(unusedStns) {}

	bool operator()(const T& stn) {
		// Is stn in the list of stations to keep?  If so, return false
		if (binary_search(_usedStns->begin(), _usedStns->end(), stn->GetName()))
			return false;
		
		// Station is not in the list, so add it to the unused stations
		_unusedStns->push_back(stn->GetName());
		return true;		
	}
	vector<S>* _usedStns;
	vector<S>* _unusedStns;
};


// uses low accuracy spherical formula
template <typename T = dnaStnPtr, typename S = stringstring_doubledouble_pair, typename U = double>
class NearbyStation_LowAcc {
public:
	NearbyStation_LowAcc (const U& tolerance, vector<S>* stns) 
		: _tolerance(tolerance), _stns(stns) {}

	bool operator()(const T& left, const T& right) {
		if ((_dist = GreatCircleDistance(
			left->GetLatitude(), 
			left->GetLongitude(), 
			right->GetLatitude(), 
			right->GetLongitude())) < _tolerance)
		{
			_stns->push_back(stringstring_doubledouble_pair(
				string_string_pair(left->GetName(), right->GetName()),
				doubledouble_pair(_dist, right->GetHeight()-left->GetHeight())));
			return true;
		}
		return false;
	}

	U			_tolerance;
	vector<S>*	_stns;
	double		_dist;
};


// uses high accuracy spherical formula
template <typename T = dnaStnPtr, typename U = double, typename S = stringstring_doubledouble_pair, typename E = CDnaEllipsoid>
class NearbyStation_HighAcc {
public:
	NearbyStation_HighAcc(const U& tolerance, vector<S>* stns, const E& ellipsoid) 
		: _tolerance(tolerance), _stns(stns), _ellipsoid(ellipsoid) {}

	bool operator()(const T& left, const T& right) {
		if ((_dist = RobbinsReverse(
			left->GetLatitude(), 
			left->GetLongitude(), 
			right->GetLatitude(), 
			right->GetLongitude(), 
			&_dAzimuth, &_ellipsoid)) < _tolerance)
		{
			_stns->push_back(stringstring_doubledouble_pair(
				string_string_pair(left->GetName(), right->GetName()),
				doubledouble_pair(_dist, right->GetHeight()-left->GetHeight())));
			return true;
		}
		return false;
	}

	U			_tolerance;
	vector<S>*	_stns;
	U			_dAzimuth;
	double		_dist;
	E			_ellipsoid;		// GDA by default
};


// T = double/float
template<typename T = double>
class FindStnsWithinBoundingBox{
public:
	FindStnsWithinBoundingBox(const T& upperLat, const T& upperLon, const T& lowerLat, const T& lowerLon, pvstring stns)
		: _upperLat(upperLat), _upperLon(upperLon), _lowerLat(lowerLat), _lowerLon(lowerLon)
		, _stns(stns) {}

	bool operator()(dnaStnPtr s) {
		if (s->GetXAxis() > _upperLat)
		{
			_stns->push_back(s->GetName());
			return true;
		}
		if (s->GetXAxis() < _lowerLat)
		{
			_stns->push_back(s->GetName());
			return true;
		}
		if (s->GetYAxis() > _upperLon)
		{
			_stns->push_back(s->GetName());
			return true;
		}
		if (s->GetYAxis() < _lowerLon)
		{
			_stns->push_back(s->GetName());
			return true;		
		}
		return false;
	}

private:
	T _upperLat;
	T _upperLon;
	T _lowerLat;
	T _lowerLon;
	pvstring _stns;
};

// requires stns to be sorted
template<typename U = string>
class FindMsrsConnectedToStns_GX{
public:
	FindMsrsConnectedToStns_GX(const pvstring stns)
		: _stns(stns) {}

	bool operator()(const CDnaGpsBaseline& m) {
		if (binary_search(_stns->begin(), _stns->end(), m.GetFirst()))
			return true;
		if (binary_search(_stns->begin(), _stns->end(), m.GetTarget()))
			return true;
		return false;
	}
private:
	pvstring _stns;
};

// requires stns to be sorted
template<typename U = string>
class FindMsrsConnectedToStns_Y{
public:
	FindMsrsConnectedToStns_Y(const pvstring stns)
		: _stns(stns) {}

	bool operator()(const CDnaGpsPoint& m) {
		if (binary_search(_stns->begin(), _stns->end(), m.GetFirst()))
			return true;
		return false;
	}	
private:
	pvstring _stns;
};

// requires stns to be sorted
template<typename U = string>
class FindMsrsConnectedToStns_D{
public:
	FindMsrsConnectedToStns_D(const pvstring stns)
		: _stns(stns) {}

	bool operator()(const CDnaDirection& m) {
		if (binary_search(_stns->begin(), _stns->end(), m.GetTarget()))
			return true;
		return false;
	}	
private:
	pvstring _stns;
};

// T = double/float
// requires stns to be sorted
template<typename T = pvstring>
class FindMsrsConnectedToStns{
public:
	FindMsrsConnectedToStns(const T stns)
		: _stns(stns) {}

	bool operator()(dnaMsrPtr m) {

		FindMsrsConnectedToStns_GX<string> gpsbslFunc(_stns);
		FindMsrsConnectedToStns_Y<string> gpspntFunc(_stns);
		FindMsrsConnectedToStns_D<string> dirnFunc(_stns);
		switch (m->GetTypeC())
		{
		
		// vector-type measurements
		case 'G':
		case 'X':			
			return find_if(m->GetBaselines_ptr()->begin(), m->GetBaselines_ptr()->end(), gpsbslFunc) != m->GetBaselines_ptr()->end();
		case 'Y':			
			return find_if(m->GetPoints_ptr()->begin(), m->GetPoints_ptr()->end(), gpspntFunc) != m->GetPoints_ptr()->end();
		}

		if (binary_search(_stns->begin(), _stns->end(), m->GetFirst()))
			return true;

		//
		// single station measurements
		//
		switch (m->GetTypeC())
		{
		case 'H':	// Orthometric height
		case 'I':	// Astronomic latitude
		case 'J':	// Astronomic longitude
		case 'P':	// Geodetic latitude
		case 'Q':	// Geodetic longitude
		case 'R':	// Ellipsoidal height
			return false;
		}

		//
		// dual (or more) station measurements
		//
		if (binary_search(_stns->begin(), _stns->end(), m->GetTarget()))
			return true;

		switch (m->GetTypeC())
		{
		case 'D':	// Direction set
			return find_if(
				m->GetDirections_ptr()->begin(), 
				m->GetDirections_ptr()->end(), 
				dirnFunc) != m->GetDirections_ptr()->end();
		case 'C':	// Chord dist
		case 'E':	// Ellipsoid arc
		case 'M':	// MSL arc
		case 'S':	// Slope distance
		case 'L':	// Level difference
		case 'B':	// Geodetic azimuth
		case 'K':	// Astronomic azimuth
		case 'V':	// Zenith angle
		case 'Z':	// Vertical angle
			return false;
		}
		
		//
		// triple station measurements
		//
		if (binary_search(_stns->begin(), _stns->end(), m->GetTarget2()))
			return true;

		//switch (m->GetTypeC())
		//{
		//case 'A':	// Horizontal angles
		//default:
		//	return false;
		//}
		
		return false;
	}

private:
	T _stns;
};
	

template<typename T>
void ResetMeasurementPtr(dnaMsrPtr* msrPtr, const T& cType)
{
	switch (cType)
	{
	case 'A': // Horizontal angle
		msrPtr->reset(new CDnaAngle);
		break;
	case 'B': // Geodetic azimuth
	case 'K': // Astronomic azimuth
		msrPtr->reset(new CDnaAzimuth);
		break;
	case 'C': // Chord dist
	case 'E': // Ellipsoid arc
	case 'M': // MSL arc
	case 'S': // Slope distance
		msrPtr->reset(new CDnaDistance);
		break;
	case 'D': // Direction set
		msrPtr->reset(new CDnaDirectionSet);
		break;
	case 'G': // GPS Baseline (treat as single-baseline cluster)
	case 'X': // GPS Baseline cluster
		msrPtr->reset(new CDnaGpsBaselineCluster);
		break;
	case 'H': // Orthometric height
		msrPtr->reset(new CDnaHeight);
		break;
	case 'I': // Astronomic latitude
	case 'J': // Astronomic longitude
	case 'P': // Geodetic latitude
	case 'Q': // Geodetic longitude
		msrPtr->reset(new CDnaCoordinate);
		break;
	case 'L': // Level difference
		msrPtr->reset(new CDnaHeightDifference);
		break;
	case 'R': // Ellipsoidal height
		msrPtr->reset(new CDnaHeight);
		break;
	case 'V': // Zenith angle
	case 'Z': // Vertical angle
		msrPtr->reset(new CDnaDirection);
		break;
	case 'Y': // GPS point cluster
		msrPtr->reset(new CDnaGpsPointCluster);
		break;
	}
}
	
// T = double/float
// requires stns to be sorted
template<typename T = CDnaDirection>
class SortDirectionsObsClockwise{
public:
	SortDirectionsObsClockwise() {};

	bool operator()(const T& left, const T& right) {
		if (left.GetValue() == right.GetValue())
			return left.m_strTarget < right.m_strTarget;
		return left.GetValue() < right.GetValue();
	}
};
	
template <typename T = dnaStnPtr>
class CompareLatitude {
public:
	bool operator()(const T& left, const T& right) {
		return left->GetXAxis() < right->GetXAxis();
	}
};


template <typename T = dnaStnPtr>
class CompareLongitude {
public:
	bool operator()(const T& left, const T& right)	{
		return left->GetYAxis() < right->GetYAxis();
	}
};

// used to sort measurements
template <typename M = CDnaMeasurement>
class CompareMsr {
public:
	bool operator()(const boost::shared_ptr<M> left, const boost::shared_ptr<M> right) {
		if (left->GetIgnore() == right->GetIgnore()) {
			if (iequals(left->GetType(), right->GetType()))
			{
				switch (left->GetTypeC())
				{
				case 'A':	// Horizontal angles
					return *(dynamic_cast<const CDnaAngle*>(&(*left))) < 
						*(dynamic_cast<const CDnaAngle*>(&(*right)));
				
				case 'C':	// Chord dist
				case 'E':	// Ellipsoid arc
				case 'M':	// MSL arc
				case 'S':	// Slope distance
					return *(dynamic_cast<const CDnaDistance*>(&(*left))) < 
						*(dynamic_cast<const CDnaDistance*>(&(*right)));
				
				case 'D':	// Direction set
					return *(dynamic_cast<const CDnaDirectionSet*>(&(*left))) < 
						*(dynamic_cast<const CDnaDirectionSet*>(&(*right)));
				
				case 'B':	// Geodetic azimuth
				case 'K':	// Astronomic azimuth
					return *(dynamic_cast<const CDnaAzimuth*>(&(*left))) < 
						*(dynamic_cast<const CDnaAzimuth*>(&(*right)));
				
				case 'I':	// Astronomic latitude
				case 'J':	// Astronomic longitude
				case 'P':	// Geodetic latitude
				case 'Q':	// Geodetic longitude
					return *(dynamic_cast<const CDnaCoordinate*>(&(*left))) < 
						*(dynamic_cast<const CDnaCoordinate*>(&(*right)));
				
				case 'H':	// Orthometric height
				case 'R':	// Ellipsoidal height
					return *(dynamic_cast<const CDnaHeight*>(&(*left))) < 
						*(dynamic_cast<const CDnaHeight*>(&(*right)));
				
				case 'L':	// Level difference
					return *(dynamic_cast<const CDnaHeightDifference*>(&(*left))) < 
						*(dynamic_cast<const CDnaHeightDifference*>(&(*right)));
				
				case 'V':	// Zenith angle
				case 'Z':	// Vertical angle
					return *(dynamic_cast<const CDnaDirection*>(&(*left))) < 
						*(dynamic_cast<const CDnaDirection*>(&(*right)));
				
				case 'G':	// GPS Baseline (treat as single-baseline cluster)
				case 'X':	// GPS Baseline cluster
					return *(dynamic_cast<const CDnaGpsBaselineCluster*>(&(*left))) < 
						*(dynamic_cast<const CDnaGpsBaselineCluster*>(&(*right)));
				
				case 'Y':	// GPS point cluster				
					return *(dynamic_cast<const CDnaGpsPointCluster*>(&(*left))) < 
						*(dynamic_cast<const CDnaGpsPointCluster*>(&(*right)));
				}
				
				return left < right;				
			}
			else
				return left->GetType() < right->GetType(); }		
		return left->GetIgnore() < right->GetIgnore();
	}
};

	

template <class CharT, class Traits>
basic_istream<CharT, Traits>& operator>>(basic_istream<CharT, Traits>& is, CAStationList t)
{
	is.read(reinterpret_cast<char *>(t.GetAssocMsrCountPtr()), sizeof(UINT32));
	is.read(reinterpret_cast<char *>(t.GetAMLStnIndexPtr()), sizeof(UINT32));
	is.read(reinterpret_cast<char *>(t.ValidityPtr()), sizeof(UINT16));
	return is;
}

template <class CharT, class Traits>
basic_istream<CharT, Traits>& operator>>(basic_istream<CharT, Traits>& is, CAStationList* t)
{
	is.read(reinterpret_cast<char *>(t->GetAssocMsrCountPtr()), sizeof(UINT32));
	is.read(reinterpret_cast<char *>(t->GetAMLStnIndexPtr()), sizeof(UINT32));
	is.read(reinterpret_cast<char *>(t->ValidityPtr()), sizeof(UINT16));
	return is;
}

template <class CharT, class TraitsT>
basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, CAStationList t)
{
	os.write(reinterpret_cast<char *>(t.GetAssocMsrCountPtr()), sizeof(UINT32));
	os.write(reinterpret_cast<char *>(t.GetAMLStnIndexPtr()), sizeof(UINT32));
	os.write(reinterpret_cast<char *>(t.ValidityPtr()), sizeof(UINT16));
	return os;
}

template <class CharT, class TraitsT>
basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& os, CAStationList* t)
{
	os.write(reinterpret_cast<char *>(t->GetAssocMsrCountPtr()), sizeof(UINT32));
	os.write(reinterpret_cast<char *>(t->GetAMLStnIndexPtr()), sizeof(UINT32));
	os.write(reinterpret_cast<char *>(t->ValidityPtr()), sizeof(UINT16));
	return os;
}


class StationNameIDCompareName {						// class for station name and ID comparison
public: //functions
	// comparison func for sorting
	bool operator()(const string_uint32_pair& lhs, const string_uint32_pair& rhs) const {
		return keyLess(lhs.first, rhs.first);
	}
	// comparison func for lookups
	bool operator()(const string_uint32_pair& lhs, const string_uint32_pair::first_type& k) const {
		return keyLess(lhs.first, k);
	}
	// comparison func for lookups
	bool operator()(const string_uint32_pair::first_type& k, const string_uint32_pair& rhs) const {
		return keyLess(k, rhs.first);
	}
private:
	// the "real" comparison function
	bool keyLess(const string_uint32_pair::first_type& k1, const string_uint32_pair::first_type& k2) const {
		return k1 < k2;
	}
};

class StationNameIDCompareID {						// class for station name and ID comparison
public: //functions
	// comparison func for sorting
	bool operator()(const string_uint32_pair& lhs, const string_uint32_pair& rhs) const {
		return keyLess(lhs.second, rhs.second);
	}
	// comparison func for lookups
	bool operator()(const string_uint32_pair& lhs, const string_uint32_pair::second_type& k) const {
		return keyLess(lhs.second, k);
	}
	// comparison func for lookups
	bool operator()(const string_uint32_pair::second_type& k, const string_uint32_pair& rhs) const {
		return keyLess(k, rhs.second);
	}
private:
	// the "real" comparison function
	bool keyLess(const string_uint32_pair::second_type& k1, const string_uint32_pair::second_type& k2) const {
		return k1 < k2;
	}
};


class ParamStnMsrCompareStn {						// class for station name and ID comparison
public: //functions
	// comparison func for sorting
	bool operator()(const uint32_string_pair& lhs, const uint32_string_pair& rhs) const {
		return keyLess(lhs.first, rhs.first);
	}
	// comparison func for lookups
	bool operator()(const uint32_string_pair& lhs, const uint32_string_pair::first_type& k) const {
		return keyLess(lhs.first, k);
	}
	// comparison func for lookups
	bool operator()(const uint32_string_pair::first_type& k, const uint32_string_pair& rhs) const {
		return keyLess(k, rhs.first);
	}
private:
	// the "real" comparison function
	bool keyLess(const uint32_string_pair::first_type& k1, const uint32_string_pair::first_type& k2) const {
		return k1 < k2;
	}
};



#endif /* DNATEMPLATESTNMSRFUNCS_H_ */
