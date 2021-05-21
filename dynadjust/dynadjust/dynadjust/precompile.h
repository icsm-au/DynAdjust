// precompile.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// See https://github.com/boostorg/process/issues/161
#include <boost/process.hpp>

#if defined(_WIN32) || defined(__WIN32__)
#define BOOST_USE_WINDOWS_H
#endif

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaoptions.hpp>
#include <include/config/dnaoptions-interface.hpp>

#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnaprocessfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnatemplatedatetimefuncs.hpp>

#include <include/config/dnaprojectfile.hpp>
