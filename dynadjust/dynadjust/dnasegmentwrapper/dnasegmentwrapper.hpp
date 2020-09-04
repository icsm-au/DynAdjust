//============================================================================
// Name         : dnasegmentwrapper.hpp
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
// Description  : DynAdjust Network Segmentation Executable
//============================================================================

#ifndef DNASEGMENTWRAPPER_H_
#define DNASEGMENTWRAPPER_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>
#include <string.h>
#include <set>

#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::posix_time;
using namespace boost::program_options;
namespace po = boost::program_options;

#include <include/config/dnaversion.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaoptions-interface.hpp>
#include <include/config/dnaconsts-interface.hpp>
#include <include/config/dnaprojectfile.hpp>

#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnafilepathfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>

#include <dynadjust/dnasegment/dnasegment.hpp>

using namespace dynadjust::networksegment;
using namespace dynadjust::exception;

extern bool running;
extern boost::mutex cout_mutex;

class dna_segment_thread
{
public:
	dna_segment_thread(dna_segment* dnaSeg, project_settings* p, _SEGMENT_STATUS_* segmentStatus, milliseconds* s, string* status_msg)
		: _dnaSeg(dnaSeg)
		, _p(p)
		, _segmentStatus(segmentStatus)
		, _s(s)
		, _status_msg(status_msg) {};
	void operator()()
	{
		running = true;
		
		if (!_p->g.quiet)
		{
			cout_mutex.lock();
			cout << "+ Loading binary files...";
			cout_mutex.unlock();
		}
		
		try {
			// prepare the segmentation
			_dnaSeg->PrepareSegmentation(_p);
		} 
		catch (const NetSegmentException& e) {
			cout_mutex.lock();
			cout << endl << "- Error: " << e.what() << endl;
			cout_mutex.unlock();
			running = false;
			return;
		}

		if (!_p->g.quiet)
		{
			cout_mutex.lock();
			cout << " done." << endl;

			if (_dnaSeg->StartingStations().empty())
			{
				string startStn(_dnaSeg->DefaultStartingStation());
				if (startStn == "")
					startStn = "the first station";
				cout << "+ Adopting " << _dnaSeg->DefaultStartingStation() << " as the initial station in the first block." << endl;
			}
			cout << "+ Creating block " << setw(PROGRESS_PAD_39) << " ";
			cout_mutex.unlock();
		}
		cpu_timer time;
		try {
			*_segmentStatus = SEGMENT_EXCEPTION_RAISED;
			*_segmentStatus = _dnaSeg->SegmentNetwork(_p, _status_msg);
		} 
		catch (const NetSegmentException& e) {
			running = false;
			cout_mutex.lock();
			cout << endl << "- Error: " << e.what() << endl;
			cout_mutex.unlock();
			return;
		}
		catch (const runtime_error& e) {
			running = false;
			boost::this_thread::sleep(milliseconds(250));
			cout_mutex.lock();
			cout << endl << "- Error: " << e.what() << endl;
			cout_mutex.unlock();
			return;
		}

		*_s = milliseconds(time.elapsed().wall/MILLI_TO_NANO);
		running = false;
		
		if (!_p->g.quiet)
		{
			cout_mutex.lock();
			cout << PROGRESS_BACKSPACE_39 << "\b" << setw(PROGRESS_PAD_39+3) << left << "s... done.   " << endl;
			cout_mutex.unlock();
		}		
	}
private:
	dna_segment*		_dnaSeg;
	project_settings*	_p;
	_SEGMENT_STATUS_*	_segmentStatus;
	milliseconds*		_s;
	string*				_status_msg;
};

class dna_segment_progress_thread
{
public:
	dna_segment_progress_thread(dna_segment* dnaSeg, project_settings* p)
		: _dnaSeg(dnaSeg), _p(p) {};
	void operator()()
	{
		UINT32 block, currentBlock(0);
		ostringstream ss;
		
		while (running)
		{
			block = _dnaSeg->currentBlock();
			if (!_p->g.quiet)
			{
				if (block != currentBlock)
				{
					ss.str("");
					ss << "(" << fixed << setw(2) << right << setprecision(0) << _dnaSeg->GetProgress() << "% stations used)";
					cout_mutex.lock();
					cout << PROGRESS_BACKSPACE_39 << setw(PROGRESS_BLOCK) << left << block << setw(PROGRESS_PERCENT_29) << right << ss.str();
					cout.flush();
					cout_mutex.unlock();
					currentBlock = block;
				}
			}
			boost::this_thread::sleep(milliseconds(75));
		}	
	}

private:
	dna_segment*		_dnaSeg;
	project_settings*	_p;
};

#endif
