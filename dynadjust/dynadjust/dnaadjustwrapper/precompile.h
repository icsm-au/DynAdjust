// precompile.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <include/config/dnaversion.hpp>
#include <include/config/dnaversion-stream.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnatypes.hpp>
#include <include/config/dnatypes-gui.hpp>
#include <include/config/dnaexports.hpp>
#include <include/config/dnaoptions-interface.hpp>
#include <include/config/dnaconsts-interface.hpp>
#include <include/config/dnaprojectfile.hpp>

#include <include/exception/dnaexception.hpp>

#include <include/io/dnaiobst.hpp>
#include <include/io/dnaiobms.hpp>
#include <include/io/dnaiomap.hpp>
#include <include/io/dnaioaml.hpp>
#include <include/io/dnaioasl.hpp>
#include <include/io/dnaioseg.hpp>
#include <include/io/dnaioadj.hpp>
#include <include/io/dnaiosnx.hpp>

#include <include/functions/dnatemplatematrixfuncs.hpp>
#include <include/functions/dnatemplatestnmsrfuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnaintegermanipfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>

#include <include/thread/dnathreading.hpp>

#include <include/parameters/dnaepsg.hpp>
#include <include/parameters/dnadatum.hpp>

#include <include/math/dnamatrix_contiguous.hpp>

#include <include/memory/dnafile_mapping.hpp>

#include <dynadjust/dnaadjust/dnaadjust.hpp>

#include <dynadjust/dnaadjustwrapper/dnaadjustwrapper.hpp>
#include <dynadjust/dnaadjustwrapper/dnaadjustprogress.hpp>
