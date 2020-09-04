//============================================================================
// Name         : dnaadjustprogress.cpp
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
// Description  : DynAdjust Adjustment executable interface and progress
//============================================================================

#include <dynadjust/dnaadjustwrapper/dnaadjustprogress.hpp>

bool running;
boost::mutex cout_mutex;

dna_adjust_thread::dna_adjust_thread(dna_adjust* dnaAdj, project_settings* p,
		_ADJUST_STATUS_* adjustStatus)
		: _dnaAdj(dnaAdj), _p(p), _adjustStatus(adjustStatus) 
{
}

void dna_adjust_thread::operator()()
{
	running = true;
		
	if (!prepareAdjustment())
	{
		running = false;
		return;
	}
	if (!processAdjustment())
	{
		running = false;
		return;
	}

	// Do addition things here...

	running = false;
}

bool dna_adjust_thread::prepareAdjustment()
{
	try {
		*_adjustStatus = ADJUST_EXCEPTION_RAISED;
		_dnaAdj->PrepareAdjustment(*_p);
		*_adjustStatus = ADJUST_SUCCESS;
		return true;
	}
	catch (const NetAdjustException& e) {
		handlePrepareAdjustError(e.what());
	}
	catch (const NetMemoryException& e) {
		handlePrepareAdjustError(e.what());
	}
	catch (const runtime_error& e) {
		handlePrepareAdjustError(e.what());
	}
	catch (...) {
		string err("Undefined error.\n  This could be the result of insufficient memory or poorly configured data.");
		handlePrepareAdjustError(err);
	}
	*_adjustStatus = ADJUST_EXCEPTION_RAISED;
	return false;
}

bool dna_adjust_thread::processAdjustment()
{		
	try {
		*_adjustStatus = _dnaAdj->AdjustNetwork();
		*_adjustStatus = ADJUST_SUCCESS;
		return true;
	} 
	catch (const NetAdjustException& e) {
		handleProcessAdjustError(e.what());
	}
	catch (const NetMemoryException& e) {
		handleProcessAdjustError(e.what());
	}
	catch (const runtime_error& e) {
		handleProcessAdjustError(e.what());
	}
	catch (...) {
		string err("- Error: Undefined error.\n  This could be the result of insufficient memory or poorly configured data.");
		handleProcessAdjustError(err);
	}
	*_adjustStatus = ADJUST_EXCEPTION_RAISED;
	return false;
}

void dna_adjust_thread::handlePrepareAdjustError(const string& error_msg)
{
	running = false;
	stringstream ss;
	ss << /*PROGRESS_BACKSPACE_12 << */setw(PROGRESS_ADJ_BLOCK_12) << " " << endl
		<< "- Error: " << endl << "  " << error_msg << endl;
	coutMessage(ss.str());
}

void dna_adjust_thread::handleProcessAdjustError(const string& error_msg)
{
	running = false;
	boost::this_thread::sleep(milliseconds(50));
	printErrorMsg(error_msg);
}

void dna_adjust_thread::printErrorMsg(const string& error_msg)
{
	stringstream ss, sst;
	
	switch (_p->a.adjust_mode)
	{
	case SimultaneousMode:
		// "  Iteration " = 12
		// setw(2)        =  2
		// -------------------
		//                  14
		sst << "  Iteration " << right << setw(2) << fixed << setprecision(0) << _dnaAdj->CurrentIteration();
		ss << PROGRESS_BACKSPACE_14 << setw(PROGRESS_ADJ_BLOCK_14) << right << sst.str();
		break;
	case Phased_Block_1Mode:
	case PhasedMode:
		// "  Iteration " = 12
		// setw(2)        =  2
		// ", block"      =  7
		// setw(6)        =  6
		// "(forward) "   = 10
		// -------------------
		//                  37
		sst << "  Iteration " << 
			right << setw(2) << fixed << setprecision(0) << _dnaAdj->CurrentIteration() << ", block" << right << setw(6) << fixed << setprecision(0) << 
			_dnaAdj->CurrentBlock() + 1 << (_dnaAdj->processingForward() ? " (forward)" : " (reverse)");
		ss << PROGRESS_BACKSPACE_37 << setw(37) << right << sst.str();
	}

	ss << endl << "- Error: Cannot compute the least squares estimates.  Reason:\n  " << error_msg << endl;
	coutMessage(ss);
}

void dna_adjust_thread::coutMessage(stringstream& message)
{
	coutMessage(message.str());
}

void dna_adjust_thread::coutMessage(const string& message)
{
	cout_mutex.lock();
	cout.flush();
	cout << message;
	cout.flush();
	cout_mutex.unlock();
}








dna_adjust_progress_thread::dna_adjust_progress_thread(dna_adjust* dnaAdj, project_settings* p)
		: _dnaAdj(dnaAdj), _p(p) {}

void dna_adjust_progress_thread::prepareAdjustment()
{
	if (_p->g.quiet)
		// Nothing to be displayed
		return;
	
	if (_p->a.report_mode)
		// Nothing to be displayed
		return;

	stringstream ss;
	ss << "+ Preparing for ";

	switch (_p->a.adjust_mode)
	{
	case Phased_Block_1Mode:
	case PhasedMode:
		ss << "a " << _dnaAdj->blockCount() << " block ";
		break;
	}
			
	ss << "adjustment... ";		
	coutMessage(ss.str());
	ss.str("");
	
	UINT32 block, currentBlock(0);
	stringstream sst;
	bool first_time(true);
	
	switch (_p->a.adjust_mode)
	{
	case SimultaneousMode:
		while (running && _dnaAdj->IsPreparing())
		{
			boost::this_thread::sleep(milliseconds(40));
		}
		
		if (_dnaAdj->ExceptionRaised())
			return;

		ss << " done." << endl;
		coutMessage(ss.str());
		
		break;
	case Phased_Block_1Mode:
	case PhasedMode:
		while (running && _dnaAdj->IsPreparing())
		{
			block = _dnaAdj->CurrentBlock();

			if (block != currentBlock)
			{
				ss.str("");
				ss << " block " << left << setw(5) << fixed << setprecision(0) << _dnaAdj->CurrentBlock() + 1;

				sst.str("");
				if (first_time)
				{
					sst << setw(PROGRESS_ADJ_BLOCK_12) << left << " ";
					first_time = false;
				}
				sst << PROGRESS_BACKSPACE_12 << setw(PROGRESS_ADJ_BLOCK_12) << left << ss.str();
				coutMessage(sst.str());
				currentBlock = block;
			}
			boost::this_thread::sleep(milliseconds(40));
		}

		if (_dnaAdj->ExceptionRaised())
			return;
		
		ss.str("");
		ss << " done.";
		sst.str("");
		if (!first_time)
			sst << PROGRESS_BACKSPACE_12;
		sst << setw(PROGRESS_ADJ_BLOCK_12) << left << ss.str() << endl;
		coutMessage(sst.str());
		
	}
}

void dna_adjust_progress_thread::processAdjustment()
{
	if (_p->g.quiet)
		// Nothing to be displayed
		return;
	
	if (_p->a.max_iterations > 0)
		coutMessage(string("+ Adjusting network...\n"));

	UINT32 block, currentBlock(0);
	UINT32 currentIteration(0);
	stringstream ss, sst;
	bool first_time(true);

	switch (_p->a.adjust_mode)
	{
	case SimultaneousMode:
			
		while (_dnaAdj->IsAdjusting())
		{
			// use message bank
			while (_dnaAdj->NewMessagesAvailable())
			{
				if (!_dnaAdj->GetMessageIteration(currentIteration))
					break;
				ss.str("");
				ss << "  Iteration " << right << setw(2) << fixed << setprecision(0) << currentIteration;
				ss << ", max station corr: " << right << setw(PROGRESS_ADJ_BLOCK_12) <<
					_dnaAdj->GetMaxCorrection(currentIteration) << endl;
					
				coutMessage(ss.str());
			}
			boost::this_thread::sleep(milliseconds(80));
		}
		
		break;
	case Phased_Block_1Mode:
	case PhasedMode:
		
		while (_dnaAdj->IsAdjusting())
		{
			block = _dnaAdj->CurrentBlock();
			
			// use message bank
			while (_dnaAdj->NewMessagesAvailable())
			{
				if (!_dnaAdj->GetMessageIteration(currentIteration))
					break;
						
				ss.str("");
				ss << "  Iteration " << right << setw(2) << fixed << setprecision(0) << currentIteration;
				ss << ", max station corr: " << right << setw(PROGRESS_ADJ_BLOCK_12) << _dnaAdj->GetMaxCorrection(currentIteration) << endl;
				
				sst.str("");
				if (first_time)
					sst << setw(PROGRESS_ADJ_BLOCK_28) << left << " ";
				sst << PROGRESS_BACKSPACE_28 << setw(PROGRESS_ADJ_BLOCK_28) << left << ss.str();
				coutMessage(sst.str());
				first_time = true;
			}

			boost::this_thread::sleep(milliseconds(40));
					
			// print new block to screen when adjusting only
			if (block != currentBlock && _dnaAdj->IsAdjusting())
			{						
				ss.str("");
				ss << "  Iteration " << right << setw(2) << fixed << setprecision(0) << _dnaAdj->CurrentIteration();

	#ifdef MULTI_THREAD_ADJUST
				if (_p->a.multi_thread && !_dnaAdj->processingCombine())
					ss << left << setw(13) << ", adjusting...";
				else
	#endif
				ss << ", block " << left << setw(6) << fixed << setprecision(0) << _dnaAdj->CurrentBlock() + 1;
						
				sst.str("");
				if (first_time)
				{
					sst << setw(PROGRESS_ADJ_BLOCK_28) << left << " ";
					first_time = false;
				}
				
				sst << PROGRESS_BACKSPACE_28 << setw(PROGRESS_ADJ_BLOCK_28) << left << ss.str();
				coutMessage(sst.str());

				currentBlock = block;
			}
		}
	}
}

void dna_adjust_progress_thread::operator()()
{
	if (_p->g.quiet)
		// Nothing to be displayed
		return;

	// cout messages related to preparing adjustment
	// prepareAdjustment() will continue while the following is true:
	//	running
	//	_dnaAdj->IsPreparing()
	//	!_dnaAdj->ExceptionRaised()
	prepareAdjustment();
	
	if (_dnaAdj->ExceptionRaised())
		return;	
	
	processAdjustment();	
}

void dna_adjust_progress_thread::coutMessage(const string& message)
{
	cout_mutex.lock();
	cout.flush();
	cout << message;
	cout.flush();
	cout_mutex.unlock();
}

