// precompile.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

typedef vector<string> vstring;

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
