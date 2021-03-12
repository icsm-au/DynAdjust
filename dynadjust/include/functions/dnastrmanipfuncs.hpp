//============================================================================
// Name         : dnastrmanipfuncs.hpp
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
// Description  : String Manipulation Functions
//============================================================================

#ifndef DNASTRMANIPFUNCS_H_
#define DNASTRMANIPFUNCS_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

#include <algorithm>
#include <functional>
#include <sstream>
#include <vector>
#include <math.h>
#include <iostream>
#include <boost/tokenizer.hpp>

#include <include/functions/dnatemplatecalcfuncs.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

using namespace std;
using boost::spirit::qi::double_;
using boost::spirit::qi::float_;
using boost::spirit::qi::parse;

template <class T, class U>
T val_uint(const U& value)
{
	return lexical_cast<T, U>(value);
}

template <class T, class U>
T valorno_uint(const U& value, T& var)
{
	if (iequals(value, "no"))
		return var = 0;

	var = 1;
	return lexical_cast<T, U>(value);
}

template <class T>
T valorno_string(const T& value)
{
	if (iequals(value, "no"))
		return "";
	return value;
}

template <class T, class U>
T yesno_uint(const U& value)
{
	if (iequals(value, "yes"))
		return 1;
	//if (iequals(value, "no"))
		return 0;

}

template <class T>
string yesno_string(const T value)
{
	if (value == 1)
		return string("yes");
	else
		return string("no");
}

template <class T>
string bool_string(const T value)
{
	if (value)
		return string("true");
	else
		return string("false");
}

template <class T>
T findandreplace(const T& source, const T& find, const T& replace)
{
	size_t index;
	T str = source;
	while ((index = str.find(find)) != T::npos)
		str.replace(index, find.length(), replace);
	return str;
}

// convert to upper case
template <class T>
void str_toupper(string& strtext)
{
	transform(strtext.begin(), strtext.end(), strtext.begin(), (T(*)(T))toupper);
}

// convert to upper case
template <class S, class T>
S str_upper(const T* const strtext)
{
	S strtext_(strtext);
	str_toupper<int>(strtext_);
	return strtext_;
}

// convert to upper case
template <class S>
S str_upper(S& strtext)
{
	str_toupper<int>(strtext);
	return strtext;
}

template <class T>
T trimstr_(const T& Src, const T& c)
{
	size_t p2 = Src.find_last_not_of(c);
	if (p2 == string::npos)
		return std::string();
	size_t p1 = Src.find_first_not_of(c);
	if (p1 == string::npos)
		p1 = 0;
	return Src.substr(p1, (p2-p1)+1);
}

template <class T>
T trimstrleft_(const T& Src, const T& c)
{
	size_t p2 = Src.length();
	size_t p1 = Src.find_first_not_of(c);
	if (p1 == string::npos)
		p1 = 0;
	return Src.substr(p1, (p2-p1)+1);
}

template <class T>
T trimstrright_(const T& Src, const T& c)
{
	size_t p2 = Src.find_last_not_of(c);
	if (p2 == string::npos)
		return std::string();
	return Src.substr(0, p2+1);
}

template <class T>
T trimstr(T& Src) {
	return trimstr_(Src, static_cast<T>(" \r\n"));
}
template <class T>
T trimstr(const T& Src) {
	return trimstr_(Src, static_cast<T>(" \r\n"));
}
template <class T>
T trimstrleft(const T& Src) {
	return trimstrleft_(Src, static_cast<T>(" \r\n"));
}
template <class T>
T trimstrright(const T& Src) {
	return trimstrright_(Src, static_cast<T>(" \r\n"));
}

template <class T, class U>
string StringFromTW(const T& t, const U& width, const U& precision=0)
{
	stringstream ss;

	// Assume number at precision prints within width
	ss << setw(width) << fixed << right << setprecision(precision) << t;
	int trim = (int)trimstr(ss.str()).length() - width;

	// Formatted string length is less than or equal to the fixed width
	if (trim <= 0)
		return ss.str();

	ss.str("");

	// Formatted string length exceeds fixed width by trim.
	// Use scientific notation if the overflow is 
	// greater than the precision
	if (trim > 0)
	{
		// scientific notation requires 6 or 5
		// characters (e.g. -1e+00 or 1e+00), not
		// including the fractional part
		if (width < (t < 0. ? 6 : 5))
		{
			// insufficient space for scientific notation!!!
			ss << setw(width) << string(width, '#');
			return ss.str();
		}
		
		int prec1 = width - (t < 0. ? 6 : 5);
		// if precision is required, an extra space is needed for
		// the decimal point, so reduce by one.
		if (prec1 > 0)
			prec1--;
		
		int prec = min<int>(precision, prec1);
		
		ss << setw(width) << scientific << right << setprecision(prec) << t;
	}
	else
	{
		int prec = precision - trim;
		if (prec < 0)
			prec = 0;
		ss << setw(width) << fixed << right << setprecision(prec) << t;
	}

	return ss.str();
}

template <class T>
string StringFromT(const T& t, const int& precision=-1)
{
	stringstream ss;
	if (precision < 0)
		ss << t;
	else
		ss << setprecision(precision) << fixed << t;
	return ss.str();
}

template <class T, class U, class S>
S double_string_width(const T value, const U& width)
{
	S s;
	char* cvalue = new char[width+1];
#if defined(_WIN32) || defined(__WIN32__)
	_snprintf(cvalue, width, "%.2f", value);
#else
	snprintf(cvalue, width, "%.2f", value);
#endif
	s = cvalue;
	delete[] cvalue;
	return trimstr(s);
}




// See here for boost::lexical_cast vs sscanf performance 
// http://www.boost.org/doc/libs/1_48_0/doc/html/boost_lexical_cast/performance.html
//
// An interesting article on conversion speeds
// http://tinodidriksen.com/2011/05/28/cpp-convert-string-to-double-speed/

template <class T>
T DoubleFromString(const string& str)
{
	T t(0.);
	parse(str.begin(), str.end(), double_, t);
	return t;
}

template <class T>
T FloatFromString(const string& str)
{
	T t(0.);
	parse(str.begin(), str.end(), float_, t);
	return t;
}

template <class T>
void DoubleFromString(T& t, const string& str)
{
	parse(str.begin(), str.end(), double_, t);
}

template <class T>
void FloatFromString(T& t, const string& str)
{
	parse(str.begin(), str.end(), float_, t);
}

template <class T>
bool DoubleFromString_ZeroCheck(T& t, const string& str)
{
	parse(str.begin(), str.end(), double_, t);
	if (fabs(t) < PRECISION_1E35)
		return true;
	return false;
}

template <class T>
// signed
typename std::enable_if<std::is_signed<T>::value, T>::type LongFromString(const string& str)
{
	char* end(NULL);
	T t(strtol(str.c_str(), &end,  10));
	
	if (*end)
	{
		stringstream ss;
		ss << "String to long conversion error: non-convertible part: " << end;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	return t;
}

template <class T>
// unsigned
typename std::enable_if<std::is_unsigned<T>::value, T>::type LongFromString(const string& str)
{
	char* end(NULL);
	T t(strtoul(str.c_str(), &end,  10));
	
	if (*end)
	{
		stringstream ss;
		ss << "String to long conversion error: non-convertible part: " << end;
		throw boost::enable_current_exception(runtime_error(ss.str()));
	}

	return t;
}

//template <class T>
//T LongFromString(const string& str)
//{
//	char* end(NULL);
//	T t;
//	string s(typeid(t).name());
//
//	if (iequals(s, "unsigned int") || iequals(s, "unsigned long") ||	// msvc
//	    iequals(s, "j") || iequals(s, "m"))								// gcc
//		t = strtoul(str.c_str(), &end,  10);
//	else if (iequals(s, "int") || iequals(s, "long") ||					// msvc
//	    iequals(s, "i") || iequals(s, "l"))								// gcc
//		t = strtol(str.c_str(), &end,  10);
//	else
//	{
//		stringstream ss;
//		ss << "String to long conversion error: " << s << " is not an integer." << end;
//		throw boost::enable_current_exception(runtime_error(ss.str()));
//	}
//
//	if (*end)
//	{
//		stringstream ss;
//		ss << "String to long conversion error: non-convertible part: " << end;
//		throw boost::enable_current_exception(runtime_error(ss.str()));
//	}
//
//	return t;
//}

//template <class T>
//void LongFromString(T *l, const string& str)
//{
//	//*l = atol(str.c_str());
//
//	*l = LongFromString<T>(str);
//}
//
template <class T>
void FromDmsString(T *d, const string& str)
{
	DmstoDeg(atof(str.c_str()), d);
}

template <class T>
T FromDmsString(const string& str)
{
	return DmstoDeg(atof(str.c_str()));
}

template <class T>
void RadFromDmsString(T *d, const string& str)
{
	DmstoDeg(atof(str.c_str()), d);
	Radians(d);
}

template <class T>
void RadFromSecondsString(T *d, const string& str)
{
	*d = SecondstoRadians(atof(str.c_str()));
}

template <class T>
void SplitDelimitedString(const T& str, const T& separator, vector<T>* tokenList)
{
	// use boost to extract stations from comma delimited string
	typedef tokenizer<char_separator<char> > tokenizer;
	char_separator<char> sepa(separator.c_str());
	tokenizer tokens(str, sepa);
	tokenList->clear();
	try {
		for_each(tokens.begin(), tokens.end(), 
			[&tokenList](const string& s) {
				if (!trimstr(s).empty())
					tokenList->push_back(trimstr(s));
		});
	}
	catch (...) {
		return;
	}	
}

#endif //DNASTRMANIPFUNCS_H_
