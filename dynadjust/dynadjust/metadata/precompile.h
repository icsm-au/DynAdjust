// precompile.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>

#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/detail/absolute_path.hpp>

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaconsts-interface.hpp>
#include <include/config/dnaoptions.hpp>
#include <include/config/dnaoptions-interface.hpp>

#include <include/exception/dnaexception.hpp>

#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnatemplatecalcfuncs.hpp>

#include <include/parameters/dnadatum.hpp>
#include <include/functions/dnatemplatedatetimefuncs.hpp>
#include <include/parameters/dnadatumprojectionparam.hpp>
#include <include/parameters/dnaellipsoid.hpp>