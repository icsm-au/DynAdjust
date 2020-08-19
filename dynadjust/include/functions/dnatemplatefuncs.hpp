//============================================================================
// Name         : dnatemplatefuncs.hpp
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
// Description  : Basic template functions using standard data types
//============================================================================

#ifndef DNATEMPLATEFUNCS_H_
#define DNATEMPLATEFUNCS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <include/config/dnatypes.hpp>
#include <include/measurement_types/dnameasurement.hpp>

using namespace std;
using namespace boost;

using namespace dynadjust::datum_parameters;
using namespace dynadjust::measurements;

template <typename T>
bool isOdd(T integer)
{
	return (integer %2 == 0);
}

#if defined(_WIN32) || defined(__WIN32__)
#if (_MSC_VER < 1600)
// copy_if is not in the C++ standaard!!!
// define a correct implementation of copy_if here
template <typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin, InputIterator end, OutputIterator dest_begin, Predicate pred)
{
	while (begin != end) {
		if (pred(*begin)) 
			*dest_begin++ = *begin;
		++begin;
	}
	return dest_begin;
}
#endif
#endif

// copy_if continues to the next element when the first return from 
// Predicate Pred is true.  This function iterates through all elements 
template <typename InputIterator, typename Predicate>
void copy_if_all_occurrences(InputIterator begin, InputIterator end, Predicate pred)
{
	InputIterator next;
	while (begin != end) {
		next = begin+1;
		while (next != end)
		{
			if (pred(*begin, *next)) 
				next++;
			else
				next = end;
		}
		++begin;
	}
}


template <typename T, typename InputIterator, typename Predicate> 
void erase_if_impl(T* t, InputIterator begin, InputIterator end, Predicate pred)
{
	t->erase(remove_if(begin, end, pred), end);
}

template <typename T, typename Predicate> 
void erase_if(T& t, Predicate pred)
{
	erase_if_impl(&t, t.begin(), t.end(), pred);
}

template <typename T, typename Predicate> 
void erase_if(T* t, Predicate pred)
{
	erase_if_impl(t, t->begin(), t->end(), pred);
}

template <typename T> 
void strip_duplicates(T& t)
{
	sort(t.begin(), t.end());	
	typename T::iterator it_newend(unique(t.begin(), t.end()));
	if (it_newend != t.end())
		t.resize(it_newend - t.begin());
}

template <typename T> 
void strip_duplicates(T* t)
{
	strip_duplicates(*t);
}

// template function for output to file or cout
template <class T>
void outputObject(T t, ostream &os) { os << t; }

// template functor for equality of (type)value
template <class C, typename T, typename OP>
class operator_on_mem_t : public binary_function<C, T, bool>
{
public:
	explicit operator_on_mem_t(T C:: *m, OP Op)
		: m_Value(m), m_Operator(Op) {}
	bool operator() (const C& cObject, const T& tValue) const {
		return m_Operator(cObject.*m_Value, tValue); }
private:
	T C::*m_Value;
	OP m_Operator;
};

// template functor helper
template <class C, typename T, typename OP>
operator_on_mem_t<C, T, OP> operator_on_mem(T C::*m_Value, OP op)
{
	return operator_on_mem_t<C, T, OP>(m_Value, op);
}

/* function object to check the value of a map element */
template <class K, typename V>
class value_equals {
private:
	V value;
public:
	// constructor (initialize value to compare with)
	value_equals (const V& v) : value(v) {
	}
	// comparison
	bool operator() (pair<const K, V> elem) {
		return elem.second == value;
	}
};

//delimiter should be a space, commar or similar
// Expected input is -38 43 28.24255
// Use of NSEW to indicate negative/positive hemispheres 
// is not supported yet.
template <class T>
T ParseDmsString(const string& dmsString, const string& delimiter)
{
	vector<string> tokenList;
	SplitDelimitedString<string>(dmsString, delimiter, &tokenList);

	// "-38 43 28.24255"
	// "-38"       -> 38.000000000
	T dms = DoubleFromString<T>(tokenList.at(0));
	
	if (tokenList.size() < 2)
		return dms;

	// "43"        ->  0.430000000
	//             -> 38.430000000
	T min = DoubleFromString<T>(tokenList.at(1));
	min /= 100.;
	if (dms < 0.0)
		dms -= min;
	else
		dms += min;

	if (tokenList.size() < 3)
		return dms;

	// "28.24255"  ->  0.002824255
	//             -> 38.432824255
	T sec = DoubleFromString<T>(tokenList.at(2));
	sec /= 10000.;
	if (dms < 0.0)
		dms -= sec;
	else
		dms += sec;
	
	return dms;

}
	
//symbols is only optional if withSpaces is true
template <class T>
string FormatDmsString(const T& dDegMinSec, const int precision, bool withSpaces, bool withSymbols)
{
	stringstream ss;
	ss << fixed << setprecision(precision) << dDegMinSec;

	// No symbols or spaces? return string
	if (!withSpaces && !withSymbols)
		return ss.str();

	string strNumber(ss.str()), strBuf(ss.str());
	
	size_t decimal(0);
	int precision_fmt(precision);
	int minute_symbol_loc(withSymbols?4:3), second_symbol_loc(withSymbols?8:6);
		
	// Add symbols for degrees minutes and seconds
	if ((decimal = strNumber.find('.')) != string::npos)
	{
		// found a decimal point!
		if (decimal == 0)					// decimal point at start, ie >>   .0123
		{
			strBuf.insert(decimal, "0");
			decimal++;
		}

		// add spaces
		if (withSpaces)
			strBuf.replace(decimal, 1, " "); 
		
		// add symbols
		if (withSymbols)
			strBuf.insert(decimal, "\260");		// 272 is the degree symbol
								
		// add zero after "tens" minute or "tens" second value
		if (precision == 1 || precision == 3)
		{
			strBuf += "0";
			precision_fmt++;
		}
		
		if (precision > 2)
		{
			// add spaces
			if (withSpaces)
				strBuf.insert((decimal+minute_symbol_loc), " "); 
		
			// add symbols
			if (withSymbols)
				strBuf.insert((decimal+minute_symbol_loc), "\222");		//minutes symbol
		}

		if (precision == 2 && withSymbols)
			strBuf += "\222";			
		
		if (precision > 4)
		{
			strBuf.insert((decimal+second_symbol_loc), ".");
			if (withSymbols)
				strBuf += "\224";
		}
		
		if (precision == 4 && withSymbols)
			strBuf += "\224";	
	}	
	else if (withSymbols)
		strBuf += "\260";		// couldn't find a decimal point, so append the degree symbol
	
	
	//// Show North and South notation
	//if (strNumber[0] == '-' || strNumber[0] == 's' || strNumber[0] == 'S' || 
	//	strNumber[0] == 'w' || strNumber[0] == 'W')
	//{
	//	if (iFlag == 1 || iFlag == 3)	// input/output latitude
	//		strBuf.replace(0, 1, "S");
	//	else							// input/output longitude
	//		strBuf.replace(0, 1, "W");
	//}
	//else
	//{
	//	if (iFlag == 1 || iFlag == 3)
	//		strBuf = "N" + strBuf;
	//	else
	//		strBuf = "E" + strBuf;
	//}
	//
	//strBuf = strBuf.Mid(0, 1) + " " + strBuf.Mid(1);


	return strBuf;
}

//symbols is only optional if withSpaces is true
template <class T>
string FormatDnaDmsString(const T& dDegMinSec, int precision)
{
	stringstream ss;
	if (precision < 4)
		precision = 4;
	ss << fixed << setprecision(precision) << dDegMinSec;
	string strBuf(ss.str());
	
	size_t decimal(0);
	string d, m, s;
		
	// Format degrees minutes and seconds
	if ((decimal = strBuf.find('.')) != string::npos)
	{
		// found a decimal point!
		if (decimal == 0)					// decimal point at start, ie >>   .0123
		{
			strBuf.insert(decimal, "0");
			decimal++;
		}

		// degrees
		d = strBuf.substr(0, decimal);

		// minutes
		m = strBuf.substr(decimal+1, 2);

		// seconds
		s = strBuf.substr(decimal+3);
		if (s.length() > 2)
			s.insert(2, ".");
		
		// cater for rounding
		if (s.substr(0, 2) == "60")
		{
			s.replace(0, 2, "00");
			int mm = atoi(m.c_str()) + 1;
			ss.str("");
			ss << mm;
			m = ss.str();
			if (mm < 10)
				m.insert(0, "0");
		}

		if (m.substr(0, 2) == "60")
		{
			m.replace(0, 2, "00");
			int dd = atoi(d.c_str()) + 1;
			ss.str("");
			ss << dd;
			d = ss.str();
		}
		
		ss.str("");
		ss << setw(3) << d << setw(3) << right << m << " " << setw(6) << s;
		strBuf = ss.str();
	}

	return strBuf;
}

	
// used to sort nearby stations
template <typename T = stringstring_doubledouble_pair>
class CompareStationPairs {
public:
	bool operator()(const T& left, const T& right) {
		if (left.first.first == right.first.first)
			return left.first.second < right.first.second;
		return left.first.first < right.first.first;
	}
};

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareClusterID
{
public:
	CompareClusterID()
		: _m(0), _id(0) {}
	CompareClusterID(const vector<M>* m, const U& id=0)
		: _m(m), _id(id) {}
	// used for lower_bound, upper_bound, etc...
	bool operator()(const boost::shared_ptr<U> lhs, const U& rhs) {
		return (*(lhs.get()) < _m->at(rhs).clusterID);
	}
	bool operator()(const U& lhs, const boost::shared_ptr<U> rhs) {
		return (_m->at(lhs).clusterID < *(rhs.get()));
	}
	// Sort by clusterID then by file order.
	// The additional, secondary sort by fileOrder is necessary
	// if the same order is expected from gcc sort and vc9 sort.
	// Strangely, vc9's sort implementation ptoduces different results to
	// gcc if sorted only by clusterID.  The difference only occurs when two
	// measurements have the same clusterID. 
	// This has an impact is on the selection of the next station from the
	// free list. It is not that one segmentation result is incorrect,
	// rather, it is just difficult to compare results directly if
	// segmentation blocks are different. 20.07.2009.
	bool operator()(const U& lhs, const U& rhs) {
		if (_m->at(lhs).clusterID == _m->at(rhs).clusterID)
			return _m->at(lhs).fileOrder < _m->at(rhs).fileOrder;
		return _m->at(lhs).clusterID < _m->at(rhs).clusterID;
	}
	bool operator()(const U& id) {
		return _m->at(id).clusterID == _id;
	}
	inline void SetAMLPointer(const vector<M>* m) { _m = m; }
	inline void SetClusterID(const U& id) { _id = id; }
	inline bool IsAMLPointerNull() { return _m == NULL; }
private:
	const vector<M>*	_m;
	U					_id;
};


// A = CAStationList, U = UINT32
// Example use:
// CompareMeasCount<CAStationList, UINT32> msrcountCompareFunc(&vAssocStnList_, vAssocStnList_.at(_it_stnmap->second).GetAssocMsrCount());
// boost::shared_ptr<UINT32> stnID(new UINT32(_it_stnmap->second));

template<typename A, typename U>
class CompareMeasCount
{
public:
	CompareMeasCount(const vector<A>* a) : _a(a) {}

	// used for lower_bound, upper_bound, etc...
	bool operator()(const boost::shared_ptr<U> lhs, const U& rhs) {
		return (*(lhs.get()) < _a->at(rhs).GetAssocMsrCount());
	}
	bool operator()(const U& lhs, const boost::shared_ptr<U> rhs) {
		return (_a->at(lhs).GetAssocMsrCount() < *(rhs.get()));
	}
	bool operator()(const U& lhs, const U& rhs)
	{
		if (_a->at(lhs).GetAssocMsrCount() == _a->at(rhs).GetAssocMsrCount())
			return _a->at(lhs).GetAMLStnIndex() < _a->at(rhs).GetAMLStnIndex();
		return _a->at(lhs).GetAssocMsrCount() < _a->at(rhs).GetAssocMsrCount();
	}
	inline void SetASLPointer(const vector<A>* a) { _a = a; }
	inline bool IsASLPointerNull() { return _a == NULL; }
private:
	const vector<A>*	_a;
};


template<typename A, typename U>
class CompareMeasCount2
{
public:
	CompareMeasCount2(const vector<A>* a) : _a(a) {}

	bool operator()(const U& lhs, const U& rhs)
	{
		if (_a->at(lhs).get()->GetAssocMsrCount() == _a->at(rhs).get()->GetAssocMsrCount())
			return _a->at(lhs).get()->GetAMLStnIndex() < _a->at(rhs).get()->GetAMLStnIndex();
		return _a->at(lhs).get()->GetAssocMsrCount() < _a->at(rhs).get()->GetAssocMsrCount();
	}
private:
	const vector<A>*	_a;
};


template<typename A, typename U>
class CompareValidity
{
public:
	CompareValidity(const vector<A>* a, const UINT16& v) : _a(a), _v(v) {}

	// used for lower_bound, upper_bound, etc...
	bool operator()(const U& asl_index) {
		return (_a->at(asl_index).Validity() == _v);
	}	
	bool operator()(const U& lhs, const U& rhs)
	{
		return _a->at(lhs).Validity() < _a->at(rhs).Validity();
	}
private:
	const vector<A>*	_a;
	const UINT16		_v;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasStart
{
public:
	CompareMeasStart(vector<M>* m, MEASUREMENT_START id=xMeas) 
		:  _m(m), _id(id) {}
	bool operator()(const U& freemsr_index) {
		return _m->at(freemsr_index).measStart == _id;
	}
	bool operator()(const U& lhs, const U& rhs) {
		return _m->at(lhs).measStart < _m->at(rhs).measStart;
	}
private:
	vector<M>*			_m;
	MEASUREMENT_START	_id;
};

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareNonMeasStart
{
public:
	CompareNonMeasStart(vector<M>* m, MEASUREMENT_START id=xMeas) 
		:  _m(m), _id(id) {}
	bool operator()(const U& freemsr_index) {
		return _m->at(freemsr_index).measStart != _id;
	}
	bool operator()(const U& lhs, const U& rhs) {
		return _m->at(lhs).measStart < _m->at(rhs).measStart;
	}
private:
	vector<M>*			_m;
	MEASUREMENT_START	_id;
};

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMsrFileOrder
{
public:
	CompareMsrFileOrder(vector<M>* m)
		:  _m(m) {}
	bool operator()(const U& lhs, const U& rhs) {
		return _m->at(lhs).fileOrder < _m->at(rhs).fileOrder;
	}
private:
	vector<M>*	_m;
};


// U = u32u32_uint32_pair
template<typename U=u32u32_uint32_pair>
class CompareBlockStationMapUnique_byBlock
{
public:
	bool operator()(const U& lhs, const U& rhs) {
		if (lhs.second == rhs.second)
			return lhs.first.first < rhs.first.first;
		return lhs.second < rhs.second;
	}
};


// S = station_t, U = u32u32_double_pair
template <typename S, typename U = u32u32_double_pair>
class CompareBlockStationMapUnique_byFileOrder {
public:
	CompareBlockStationMapUnique_byFileOrder(vector<S>* s)
		:  _s(s) {}
	bool operator()(const U& left, const U& right) {
		return _s->at(left.first.first).fileOrder < _s->at(right.first.first).fileOrder;
	}
private:
	vector<S>*	_s;
};

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasType_PairFirst
{
public:
	CompareMeasType_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (_m->at(lhs.first).measType == _m->at(rhs.first).measType)
		{
			if (_m->at(lhs.first).station1 == _m->at(rhs.first).station1)
			{
				if (_m->at(lhs.first).station2 == _m->at(rhs.first).station2)
					return _m->at(lhs.first).term1 < _m->at(rhs.first).term1;
				else
					return _m->at(lhs.first).station2 < _m->at(rhs.first).station2;	
			}
			return _m->at(lhs.first).station1 < _m->at(rhs.first).station1;
		}
		return _m->at(lhs.first).measType < _m->at(rhs.first).measType;
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasFromStn_PairFirst
{
public:
	CompareMeasFromStn_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (_m->at(lhs.first).station1 == _m->at(rhs.first).station1)
		{
			if (_m->at(lhs.first).measType == _m->at(rhs.first).measType)
			{
				if (_m->at(lhs.first).station2 == _m->at(rhs.first).station2)
					return _m->at(lhs.first).term1 < _m->at(rhs.first).term1;
				else
					return _m->at(lhs.first).station2 < _m->at(rhs.first).station2;
			}
			return _m->at(lhs.first).measType < _m->at(rhs.first).measType;
		}
		return _m->at(lhs.first).station1 < _m->at(rhs.first).station1;
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasToStn_PairFirst
{
public:
	CompareMeasToStn_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (_m->at(lhs.first).station2 == _m->at(rhs.first).station2)
		{
			if (_m->at(lhs.first).measType == _m->at(rhs.first).measType)
			{
				if (_m->at(lhs.first).station1 == _m->at(rhs.first).station1)
					return _m->at(lhs.first).term1 < _m->at(rhs.first).term1;
				else
					return _m->at(lhs.first).station1 < _m->at(rhs.first).station1;
			}
			return _m->at(lhs.first).measType < _m->at(rhs.first).measType;
		}
		return _m->at(lhs.first).station2 < _m->at(rhs.first).station2;
	}
private:
	vector<M>*	_m;
};


// m = measurement_t
template<typename m>
bool isCompoundMeas(const m& msrType)
{
	switch (msrType)
	{
	case 'G':
	case 'X':
	case 'Y':
		return true;
	}
	return false;
}

template<typename m>
bool notCompoundMeas(const m& msrType)
{
	switch (msrType)
	{
	case 'G':
	case 'X':
	case 'Y':
		return false;
	}
	return true;
}

// m = measurement_t
template<typename m>
bool isCompoundMeasAll(const m& msrType)
{
	switch (msrType)
	{
	case 'D':
	case 'G':
	case 'X':
	case 'Y':
		return true;
	}
	return false;
}

template<typename m>
bool notCompoundMeasAll(const m& msrType)
{
	switch (msrType)
	{
	case 'D':
	case 'G':
	case 'X':
	case 'Y':
		return false;
	}
	return true;
}

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasValue_PairFirst
{
public:
	CompareMeasValue_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (isCompoundMeas(_m->at(lhs.first).measType) && notCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).term1, _m->at(lhs.first+1).term1, _m->at(lhs.first+2).term1));
			return lhsValue > _m->at(rhs.first).term1;
		}
		else if (notCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double rhsValue(magnitude(_m->at(rhs.first).term1, _m->at(rhs.first+1).term1, _m->at(rhs.first+2).term1));
			return _m->at(lhs.first).term1 > rhsValue;
		}
		else if (isCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).term1, _m->at(lhs.first+1).term1, _m->at(lhs.first+2).term1));
			double rhsValue(magnitude(_m->at(rhs.first).term1, _m->at(rhs.first+1).term1, _m->at(rhs.first+2).term1));
			return lhsValue > rhsValue;
		}
		else
			return _m->at(lhs.first).term1 > _m->at(rhs.first).term1;
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasResidual_PairFirst
{
public:
	CompareMeasResidual_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (isCompoundMeas(_m->at(lhs.first).measType) && notCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).measCorr, _m->at(lhs.first+1).measCorr, _m->at(lhs.first+2).measCorr));
			return lhsValue > _m->at(rhs.first).measCorr;
		}
		else if (notCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double rhsValue(magnitude(_m->at(rhs.first).measCorr, _m->at(rhs.first+1).measCorr, _m->at(rhs.first+2).measCorr));
			return _m->at(lhs.first).measCorr > rhsValue;
		}
		else if (isCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).measCorr, _m->at(lhs.first+1).measCorr, _m->at(lhs.first+2).measCorr));
			double rhsValue(magnitude(_m->at(rhs.first).measCorr, _m->at(rhs.first+1).measCorr, _m->at(rhs.first+2).measCorr));
			return lhsValue > rhsValue;
		}
		else
			return _m->at(lhs.first).measCorr > _m->at(rhs.first).measCorr;
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasAdjSD_PairFirst
{
public:
	CompareMeasAdjSD_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (isCompoundMeas(_m->at(lhs.first).measType) && notCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).measAdjPrec, _m->at(lhs.first+1).measAdjPrec, _m->at(lhs.first+2).measAdjPrec));
			return lhsValue > _m->at(rhs.first).measAdjPrec;
		}
		else if (notCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double rhsValue(magnitude(_m->at(rhs.first).measAdjPrec, _m->at(rhs.first+1).measAdjPrec, _m->at(rhs.first+2).measAdjPrec));
			return _m->at(lhs.first).measAdjPrec > rhsValue;
		}
		else if (isCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))
		{
			double lhsValue(magnitude(_m->at(lhs.first).measAdjPrec, _m->at(lhs.first+1).measAdjPrec, _m->at(lhs.first+2).measAdjPrec));
			double rhsValue(magnitude(_m->at(rhs.first).measAdjPrec, _m->at(rhs.first+1).measAdjPrec, _m->at(rhs.first+2).measAdjPrec));
			return lhsValue > rhsValue;
		}
		else
			return _m->at(lhs.first).measAdjPrec > _m->at(rhs.first).measAdjPrec;
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareMeasNstat_PairFirst
{
public:
	CompareMeasNstat_PairFirst(vector<M>* m)
		:  _m(m) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (isCompoundMeasAll(_m->at(lhs.first).measType) && notCompoundMeasAll(_m->at(rhs.first).measType))
		{
			double lhsValue;
			switch (_m->at(lhs.first).measType)
			{
			case 'G':
			case 'X':
			case 'Y':
				//lhsValue = magnitude(_m->at(lhs.first).NStat, _m->at(lhs.first+1).NStat, _m->at(lhs.first+2).NStat);
				lhsValue = max(fabs(_m->at(lhs.first).NStat), fabs(_m->at(lhs.first + 1).NStat));
				lhsValue = max(lhsValue, fabs(_m->at(lhs.first + 2).NStat));
				break;
			case 'D':
				//TRACE("%.9f\n", radians_to_degrees_(_m->at(lhs.first).term1));
				lhsValue = fabs(_m->at(lhs.first).NStat);
				for (UINT32 d(1); d<_m->at(lhs.first).vectorCount1; ++d)
				{
					//TRACE("%.9f\n", _m->at(lhs.first+d).term1);
					if (fabs(_m->at(lhs.first+d).NStat) > lhsValue)
						lhsValue = fabs( _m->at(lhs.first+d).NStat);
				}
				break;
			}
			
			return fabs(lhsValue) > fabs(_m->at(rhs.first).NStat);
		}
		else if (notCompoundMeasAll(_m->at(lhs.first).measType) && isCompoundMeasAll(_m->at(rhs.first).measType))
		{			
			double rhsValue;
			switch (_m->at(rhs.first).measType)
			{
			case 'G':
			case 'X':
			case 'Y':
				//rhsValue = magnitude(_m->at(rhs.first).NStat, _m->at(rhs.first+1).NStat, _m->at(rhs.first+2).NStat);
				rhsValue = max(fabs(_m->at(rhs.first).NStat), fabs(_m->at(rhs.first + 1).NStat));
				rhsValue = max(rhsValue, fabs(_m->at(rhs.first + 2).NStat));
				break;
			case 'D':
				//TRACE("%.9f\n", radians_to_degrees_(_m->at(rhs.first).term1));
				rhsValue = fabs(_m->at(rhs.first).NStat);
				for (UINT32 d(1); d<_m->at(rhs.first).vectorCount1; ++d)
				{
					//TRACE("%.9f\n", _m->at(lhs.first+d).term1);
					if (fabs(_m->at(rhs.first+d).NStat) > rhsValue)
						rhsValue =  fabs(_m->at(rhs.first+d).NStat);
				}
				break;
			}

			return fabs(_m->at(lhs.first).NStat) > fabs(rhsValue);
		}
		else if (isCompoundMeasAll(_m->at(lhs.first).measType) && isCompoundMeasAll(_m->at(rhs.first).measType))
		{
			double lhsValue;
			switch (_m->at(lhs.first).measType)
			{
			case 'G':
			case 'X':
			case 'Y':
				//lhsValue = magnitude(_m->at(lhs.first).NStat, _m->at(lhs.first+1).NStat, _m->at(lhs.first+2).NStat);
				lhsValue = max(fabs(_m->at(lhs.first).NStat), fabs(_m->at(lhs.first + 1).NStat));
				lhsValue = max(lhsValue, fabs(_m->at(lhs.first + 2).NStat));
				break;
			case 'D':
				//TRACE("%.9f\n", radians_to_degrees_(_m->at(lhs.first).term1));
				lhsValue = fabs(_m->at(lhs.first).NStat);
				for (UINT32 d(1); d<_m->at(lhs.first).vectorCount1; ++d)
				{
					//TRACE("%.9f\n", _m->at(lhs.first+d).term1);
					if (fabs(_m->at(lhs.first+d).NStat) > lhsValue)
						lhsValue =  fabs(_m->at(lhs.first+d).NStat);
				}
				break;
			}

			double rhsValue;
			switch (_m->at(rhs.first).measType)
			{
			case 'G':
			case 'X':
			case 'Y':
				//rhsValue = magnitude(_m->at(rhs.first).NStat, _m->at(rhs.first+1).NStat, _m->at(rhs.first+2).NStat);
				rhsValue = max(fabs(_m->at(rhs.first).NStat), fabs(_m->at(rhs.first + 1).NStat));
				rhsValue = max(rhsValue, fabs(_m->at(rhs.first + 2).NStat));
				break;
			case 'D':
				//TRACE("%.9f\n", radians_to_degrees_(_m->at(rhs.first).term1));
				rhsValue = fabs(_m->at(rhs.first).NStat);
				for (UINT32 d(1); d<_m->at(rhs.first).vectorCount1; ++d)
				{
					//TRACE("%.9f\n", _m->at(lhs.first+d).term1);
					if (fabs(_m->at(rhs.first+d).NStat) > rhsValue)
						rhsValue =  fabs(_m->at(rhs.first+d).NStat);
				}
			}

			return fabs(lhsValue) > fabs(rhsValue);
		}
		else
			return fabs(_m->at(lhs.first).NStat) > fabs(_m->at(rhs.first).NStat);
	}
private:
	vector<M>*	_m;
};


// M = measurement_t, U = UINT32, T = double
template<typename M, typename U, typename T>
class CompareMeasOutlier_PairFirst
{
public:
	CompareMeasOutlier_PairFirst(vector<M>* m, const T t)
		:  _m(m), _t(t) {}
	bool operator()(const pair<U, pair<U, U> >& lhs, const pair<U, pair<U, U> >& rhs) {
		if (isCompoundMeas(_m->at(lhs.first).measType) && notCompoundMeas(_m->at(rhs.first).measType))
		{
			bool lhs_pass((fabs(_m->at(lhs.first).NStat) < _t) || (fabs(_m->at(lhs.first+1).NStat) < _t) || (fabs(_m->at(lhs.first+2).NStat) < _t));
			return (lhs_pass < (fabs(_m->at(rhs.first).NStat) < _t));
		}
		else if (notCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))	
		{
			bool rhs_pass((fabs(_m->at(rhs.first).NStat) < _t) || (fabs(_m->at(rhs.first+1).NStat) < _t) || (fabs(_m->at(rhs.first+2).NStat) < _t));
			return ((fabs(_m->at(lhs.first).NStat) < _t) < rhs_pass);
		}
		else if (isCompoundMeas(_m->at(lhs.first).measType) && isCompoundMeas(_m->at(rhs.first).measType))	
		{
			bool lhs_pass((fabs(_m->at(lhs.first).NStat) < _t) || (fabs(_m->at(lhs.first+1).NStat) < _t) || (fabs(_m->at(lhs.first+2).NStat) < _t));
			bool rhs_pass((fabs(_m->at(rhs.first).NStat) < _t) || (fabs(_m->at(rhs.first+1).NStat) < _t) || (fabs(_m->at(rhs.first+2).NStat) < _t));
			return lhs_pass < rhs_pass;
		}
		else
			return ((fabs(_m->at(lhs.first).NStat) < _t) < (fabs(_m->at(rhs.first).NStat) < _t));
	}
private:
	vector<M>*	_m;
	T			_t;
};


// S = station_t, U = UINT32
template<typename S, typename U>
class CompareStnFileOrder
{
public:
	CompareStnFileOrder(vector<S>* s)
		:  _s(s) {}
	bool operator()(const U& lhs, const U& rhs) {
		return _s->at(lhs).fileOrder < _s->at(rhs).fileOrder;
	}
private:
	vector<S>*	_s;
};


// S = CDnaStation
template<typename S>
class CompareStnName_CDnaStn
{
public:
	bool operator()(const boost::shared_ptr<S> lhs, const boost::shared_ptr<S> rhs) {
		if (lhs.get()->GetName() == rhs.get()->GetName())
			return (lhs.get()->GetfileOrder() < rhs.get()->GetfileOrder());
		return (lhs.get()->GetName() < rhs.get()->GetName());
		
	}
};

// S = CDnaStation
template<typename S>
class CompareStnFileOrder_CDnaStn
{
public:
	bool operator()(const boost::shared_ptr<S> lhs, const boost::shared_ptr<S> rhs) {
		if (lhs.get()->GetfileOrder() == rhs.get()->GetfileOrder())
			return (lhs.get()->GetName() < rhs.get()->GetName());
		return (lhs.get()->GetfileOrder() < rhs.get()->GetfileOrder());
	}
};

// S = station_t, U = stn_block_map_t<UINT32, double>
template<typename S, typename U>
class CompareStnFileOrder_StnBlockMap
{
public:
	CompareStnFileOrder_StnBlockMap(vector<S>* s)
		:  _s(s) {}
	bool operator()(const U& lhs, const U& rhs) {
		return _s->at(lhs.station_id).fileOrder < _s->at(rhs.station_id).fileOrder;
	}
private:
	vector<S>*	_s;
};


// S = station_t, U = stn_block_map_t<UINT32, double>
template<typename U>
class CompareStnOrder_StnBlockMap
{
public:
	bool operator()(const U& lhs, const U& rhs) {
		if (lhs.block_no == rhs.block_no)
			return lhs.station_id < rhs.station_id;
		return lhs.block_no < rhs.block_no;
	}
};


// S = station_t, U = UINT32
template<typename S, typename U>
class CompareStnLongitude
{
public:
	CompareStnLongitude(vector<S>* s, bool leftToRight=true)
		:  _s(s)
		, _leftToRight(leftToRight) {}
	bool operator()(const U& lhs, const U& rhs) {
		if (_leftToRight)
			return _s->at(lhs).initialLongitude < _s->at(rhs).initialLongitude;
		else
			return _s->at(lhs).initialLongitude > _s->at(rhs).initialLongitude;
	}
private:
	vector<S>*	_s;
	bool		_leftToRight;
};


// M = measurement_t
template<typename M, typename C = char>
class CompareMeasTypeT
{
public:
	CompareMeasTypeT(const C& t) :  _type(t) {}
	inline void SetComparand(const char& t) { _type = t; }
	bool operator()(const M& msr) {
		return msr.measType == _type;
	}

private:
	C _type;
};
	

// M = measurement_t
template<typename M>
class CompareValidMeasTypeT
{
public:
	CompareValidMeasTypeT(const char& t) :  _type(t) {}
	inline void SetComparand(const char& t) { _type = t; }
	bool operator()(const M& msr) {
		return msr.ignore == false &&
			msr.measType == _type;
	}

private:
	char _type;
};


template<typename T = scalar_t, typename S = string>
class CompareScalarStation1
{
public:
	// used for lower_bound, upper_bound, etc...
	bool operator()(const T& scalar, const S& station) {
		return (scalar.station1 == station);
	}
};
	
template<typename T = scalar_t, typename S = string>
class CompareScalarStation2
{
public:
	// used for lower_bound, upper_bound, etc...
	bool operator()(const T& scalar, const S& station) {
		return (scalar.station2 == station);
	}
};

// Baseline scalar functions
//template <typename T = scalar_t>
//class CompareBaselineScalar {
//public:
//
//	CompareBaselineScalar(vector<T>* scalars)
//		: _scalars(scalars)
//	{
//	}
//
//	bool operator()(const T& left, const T& right) {
//		if (equals(left.station1, right->station1))
//			return left.station2 < right->station2;
//		else
//			return left.station1 < right->station1;
//	}
//
//	bool operator()(const string& station1, const string& station2) {
//		// not right
//		if (!lower_bound(_scalars.begin(), _scalars.end(),
//			boost::bind(CompareScalarStation1<scalar_t, string>(), station1)))
//			return false;
//		if (!binary_search(_scalars.begin(), _scalars.end(),
//			boost::bind(CompareScalarStation2<scalar_t, string>(), station2)))
//			return false;
//		return true;
//	}
//private:
//	vector<T>*	_scalars;
//};


// M = measurement_t
template<typename M, typename U, typename C>
class CompareValidFreeMeasType_vT
{
public:
	CompareValidFreeMeasType_vT(vector<M>* msrs, vector<C>& vtypes) 
		: _msrs(msrs), _vtypes(vtypes) 
	{
		sort(vtypes.begin(), vtypes.end());
	}
	bool operator()(const U& msr_index) {
		return _msrs->at(msr_index).ignore == false &&
			binary_search(_vtypes.begin(), _vtypes.end(), _msrs->at(msr_index).measType);
	}

private:
	vector<M>*	_msrs;
	vector<C> _vtypes;
};
	

// M = measurement_t, U = UINT32
template<typename M, typename U>
class CompareIgnoreedMeas
{
public:
	CompareIgnoreedMeas(vector<M>* m) :  _m(m) {}
	bool operator()(const U& freemsr_index) {
		return _m->at(freemsr_index).ignore;
	}
	bool operator()(const U& lhs, const U& rhs) {
		return _m->at(lhs).ignore < _m->at(rhs).ignore;
	}
private:
	vector<M>*	_m;
};


// M = CDnaMeasurement, U = UINT32
template<typename M>
class CompareIgnoreedClusterMeas
{
public:
	CompareIgnoreedClusterMeas() {}
	bool operator()(const M& msr) {
		return msr.GetIgnore();
	}
	bool operator()(const M& lhs, const M& rhs) {
		return lhs.GetIgnore() < rhs.GetIgnore();
	}
};

// M = CDnaGpsPoint or CDnaGpsBaseline (derived from CDnaMeasurement), U = UINT32
template<typename M>
class CompareIgnoredMsr
{
public:
	CompareIgnoredMsr() {}
	bool operator()(const boost::shared_ptr<M> msr) {
		return msr->GetIgnore();
	}
	bool operator()(const boost::shared_ptr<M> lhs, const boost::shared_ptr<M> rhs) {
		if (lhs->GetIgnore() == rhs->GetIgnore())
			return lhs->GetFirst() < rhs->GetFirst();
		return lhs->GetIgnore() < rhs->GetIgnore();
	}
};

// M = CDnaGpsPoint or CDnaGpsBaseline (derived from CDnaMeasurement), U = UINT32
template<typename M>
class CompareEmptyClusterMeas
{
public:
	CompareEmptyClusterMeas() {}
	bool operator()(const boost::shared_ptr<M> msr) {
		switch (msr->GetTypeC())
		{
		case 'X':
			return msr->GetBaselines_ptr()->empty();
		case 'Y':
			return msr->GetPoints_ptr()->empty();
		default:
			return false;
		}
		//return msr->GetTotal() == 0;
	}
	//bool operator()(const boost::shared_ptr<M> lhs, const boost::shared_ptr<M> rhs) {
	//	return lhs->GetTotal() < rhs->GetTotal();
	//}
};

// M = CDnaGpsPoint or CDnaGpsBaseline (derived from CDnaMeasurement), U = UINT32
template<typename M>
class CompareClusterMsrFunc
{
public:
	CompareClusterMsrFunc() {}
	bool operator()(const boost::shared_ptr<M> lhs, const boost::shared_ptr<M> rhs) {
		if (lhs->GetTypeC() != rhs->GetTypeC())
			return lhs->GetTypeC() < rhs->GetTypeC();
		else
			return lhs->GetFirst() < rhs->GetFirst();
	}
};


// M = CDnaMeasurement
template<typename M>
class CompareMeasType
{
public:
	CompareMeasType(const string& s) :  _s(s) {}
	inline void SetComparand(const char& s) { _s = s; }

	bool operator()(boost::shared_ptr<M> m) {
		for (_it_s=_s.begin(); _it_s!=_s.end(); ++_it_s)  {
			if (m->GetTypeC() == *_it_s)
				return true;
		}
		return false;
	}

private:
	string				_s;
	_it_str				_it_s;
};

// M = CDnaMeasurement
template<typename M>
class CompareNonMeasType
{
public:
	CompareNonMeasType(const string& s) :  _s(s) {}
	inline void SetComparand(const string& s) { _s = s; }

	bool operator()(boost::shared_ptr<M> m) {
		_bFd = true;
		for (_it_s=_s.begin(); _it_s!=_s.end(); ++_it_s)
			_bFd = _bFd && m->GetTypeC() != *_it_s;
		return _bFd;
	}


private:
	string				_s;
	_it_str				_it_s;
	bool				_bFd;
};


template<typename M, typename U>
class CompareCovarianceStart
{
public:
	CompareCovarianceStart(vector<M>* m) :  _m(m) {}
	bool operator()(const U& freemsr_index) {
		return _m->at(freemsr_index).measStart > zMeas;
	}
private:
	vector<M>*	_m;
};


template<typename T>
class ComparePairSecond
{
public:
	bool operator()(const pair<T, T>& lhs, const pair<T, T>& rhs) const {
		return pair_secondless(lhs.second, rhs.second);
	}
	bool operator()(const pair<T, T>& lhs, const T& rhs) {
		return pair_secondless(lhs.second, rhs);
	}
	bool operator()(const T& lhs, const pair<T, T>& rhs) {
		return pair_secondless(lhs, rhs.second);
	}
private:
	bool pair_secondless(const T& s1, const T& s2) const {
		return s1 < s2;
	}
};

template<typename T>
class ComparePairFirst
{
public:
	bool operator()(const pair<T, T>& lhs, const pair<T, T>& rhs) const {
		return pair_firstless(lhs.first, rhs.first);
	}
	bool operator()(const pair<T, T>& lhs, const T& rhs) {
		return pair_firstless(lhs.first, rhs);
	}
	bool operator()(const T& lhs, const pair<T, T>& rhs) {
		return pair_firstless(lhs, rhs.first);
	}
private:
	bool pair_firstless(const T& s1, const T& s2) const {
		return s1 < s2;
	}
};

	
template<typename T, typename U>
class CompareOddPairFirst
{
public:
	bool operator()(const pair<T, U>& lhs, const pair<T, U>& rhs) const {
		return lhs.first < rhs.first;
	}
};

// S = station_t, T = UINT32, U = string
template<typename S, typename T, typename U>
class CompareOddPairFirst_FileOrder
{
public:
	CompareOddPairFirst_FileOrder(vector<S>* s)
		:  _s(s) {}
	bool operator()(const pair<T, U>& lhs, const pair<T, U>& rhs) {
		return _s->at(lhs.first).fileOrder < _s->at(rhs.first).fileOrder;
	}
private:
	vector<S>*	_s;
};


template<typename T>
class ComparePairSecondf
{
public:
	ComparePairSecondf(T t) :  _t(t) {}
	bool operator()(const pair<T, T>& t) {
		return t.second == _t;
	}
	
	inline void SetComparand(const T& t) { _t = t; }

private:
	T	_t;
};
	
// tweak the binary search so it returns the iterator of the object found
template<typename Iter, typename T>
Iter binary_search_index(Iter begin, Iter end, T value)
{
	Iter i = lower_bound(begin, end, value);
	if (i != end && *i == value)
		return i;
	else 
		return end;
}

// tweak the binary search so it returns the iterator of the object found
template<typename Iter, typename T>
Iter binary_search_index_pair(Iter begin, Iter end, T value)
{
	Iter i = lower_bound(begin, end, value, ComparePairFirst<T>());
	if (i != end && i->first == value)
		return i;
	else 
		return end;
}

// M = measurement_t, U = UINT32
// Compare functor - searches for all stations that appear in the list
template<typename U, typename M, typename C>
class CompareFreeClusterAllStns
{
public:
	CompareFreeClusterAllStns(vector<U>* u, vector<M>* m, const C& c) :  _u(u), _m(m), _c(c) {}
	bool operator()(const U& amlindex) {
		// one-station measurement types
		// is this station on the list?
		// Is station 1 on inner or junction lists?
		switch (_c)
		{
		case 'H':	// Orthometric height
		case 'R':	// Ellipsoidal height
		case 'I':	// Astronomic latitude
		case 'J':	// Astronomic longitude
		case 'P':	// Geodetic latitude
		case 'Q':	// Geodetic longitude
		case 'Y':	// GPS point cluster
			if (binary_search(_u->begin(), _u->end(), _m->at(amlindex).station1))
				return true;
			return false;
		}
		// two-station measurement types
		// is this station on the list?
		switch (_c)
		{
		case 'D':	// Direction set
		case 'B':	// Geodetic azimuth
		case 'K':	// Astronomic azimuth
		case 'C':	// Chord dist
		case 'E':	// Ellipsoid arc
		case 'M':	// MSL arc
		case 'S':	// Slope distance
		case 'L':	// Level difference
		case 'V':	// Zenith angle
		case 'Z':	// Vertical angle
		case 'G':	// GPS Baseline (treat as single-baseline cluster)
		case 'X':	// GPS Baseline cluster
			if (binary_search(_u->begin(), _u->end(), _m->at(amlindex).station1) &&
				binary_search(_u->begin(), _u->end(), _m->at(amlindex).station2))
				return true;
			return false;
		}
		// three-station measurement types
		// is this station on the list?
		if (binary_search(_u->begin(), _u->end(), _m->at(amlindex).station1) &&
			binary_search(_u->begin(), _u->end(), _m->at(amlindex).station2) &&
			binary_search(_u->begin(), _u->end(), _m->at(amlindex).station3))
			return true;
		return false;
	}
private:
	vector<U>*	_u;
	vector<M>*	_m;
	char		_c;
};



template <typename T, typename U>
class PairCompareFirst {
public: //functions
	// comparison func for sorting
	//bool operator()(const pair<T, U>& lhs, const pair<T, U>& rhs) const {
	bool operator()(const string_string_pair& lhs, const string_string_pair& rhs) const {
		return keyLess(lhs.first, rhs.first);
	}
	// comparison func for lookups
	//bool operator()(const pair<T, U>& lhs, const pair<T, U>::first_type& rhs) const {
	bool operator()(const string_string_pair& lhs, const string_string_pair::first_type& rhs) const {
		return keyLess(lhs.first, rhs);
	}
	// comparison func for lookups
	//bool operator()(const pair<T, U>::first_type& lhs, const pair<T, U>& rhs) const {
	bool operator()(const string_string_pair::first_type& lhs, const string_string_pair& rhs) const {
		return keyLess(lhs, rhs.first);
	}
private:
	// the "real" comparison function
	//bool keyLess(const pair<T, U>::first_type& k1, const pair<T, U>::first_type& k2) const {
	bool keyLess(const string_string_pair::first_type& k1, const string_string_pair::first_type& k2) const {
		return k1 < k2;
	}
};


#endif /* DNATEMPLATEFUNCS_H_ */
