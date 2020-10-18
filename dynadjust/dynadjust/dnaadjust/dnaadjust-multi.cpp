//============================================================================
// Name         : dnaadjust-multi.cpp
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
// Description  : DynAdjust Network Adjustment (multi-thread) library
//============================================================================

#include <dynadjust/dnaadjust/dnaadjust.hpp>

namespace dynadjust { namespace networkadjust {

concurrent_block_adjustment<UINT32> concurrentAdjustments;

extern concurrent_queue<UINT32> combineAdjustmentQueue;
extern concurrent_queue<UINT32> prepareAdjustmentQueue;
extern boost::mutex dbg_file_mutex;
extern boost::exception_ptr fwd_error;
extern boost::exception_ptr rev_error;
extern boost::exception_ptr cmb_error;
extern boost::exception_ptr prep_error;

bool combineAdjustmentExceptionThrown(const vector<boost::exception_ptr>& cmb_errors_, const UINT32& thread_id)
{
	// got a forward or reverse exception, or 
	// have any of the other combination adjustments failed?
	if (fwd_error || rev_error)
	{
		if (combineAdjustmentQueue.is_empty())
			combineAdjustmentQueue.push_and_notify(99999999);
		return true;
	}

	UINT32 err_size(static_cast<UINT32>(cmb_errors_.size()));
	for (UINT32 err=0; err<err_size; ++err)
	{
		if (cmb_errors_.at(err))
		{
			if (combineAdjustmentQueue.is_empty())
				combineAdjustmentQueue.push_and_notify(99999999);
			return true;
		}
	}
	return false;
}

bool prepareAdjustmentExceptionThrown(const vector<boost::exception_ptr>& prep_errors_, const UINT32& thread_id)
{
	// have any of the other combination adjustments failed?
	UINT32 err_size(static_cast<UINT32>(prep_errors_.size()));
	for (UINT32 err=0; err<err_size; ++err)
	{
		if (prep_errors_.at(err))
		{
			if (prepareAdjustmentQueue.is_empty())
				prepareAdjustmentQueue.push_and_notify(99999999);
			return true;
		}
	}
	return false;
}

// Multi thread phased adjustment
// Notes for general understanding of the multi thread operation:
//	- Thread 1 undertakes a forward adjustment
//	- Thread 2 undertakes a reverse adjustment
//	- Thread 3 undertakes a combine adjustment
//	- Threads 1 and 2 are executed simultaneously without any due regard for each other.  The 
//	  coordinates and uncertainties produced from the forward and reverse runs are managed in
//	  separate matrices, hence the two threads may run without conflict.
//    After each block is adjusted, the corresponding forward and reverse mutexes for each
//	  block are unlocked.
//	- Immediately after block n has been adjusted in both forward and reverse directions, 
//	  Thread 3 undertakes combination adjustment of forward/reverse blocks.  This is handled
//	  by waiting for the two (forward and reverse) mutexes for each block.  Hence, it is
//	  only when both Threads 1 and 2 have triggered 'unlock' on the respective forward/reverse
//	  mutexes that Thread 3 commence.
//
void dna_adjust::AdjustPhasedMultiThread()
{
	currentIteration_ = 0;

	string corr_msg;
	ostringstream ss;
	UINT32 i;
	bool iterate(true);

	milliseconds iteration_time(milliseconds(0));
	cpu_timer it_time, tot_time;

#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
	boost::shared_ptr<thread> f, r, c;
	vector< boost::shared_ptr<thread> > mt_adjust_threads_sp;
#else
	vector<boost::thread> mt_adjust_threads;
#endif	

	concurrentAdjustments.resize_runs(blockCount_);

	// do until convergence criteria is met
	for (i=0; i<projectSettings_.a.max_iterations; ++i)
	{
		if (IsCancelled())
			break;

		SetcurrentBlock(0);
		blockLargeCorr_ = 0;
		largestCorr_ = 0.0;
		maxCorr_ = 0.0;

		///////////////////////////////////
		// Print the iteration # to adj file.
		// protected write to adj file (not needed here since write to
		// adj file at this stage is via single thread
		PrintIteration(++currentIteration_);
		///////////////////////////////////

		concurrentAdjustments.reset_adjustment_runs();		
		concurrentAdjustments.combine_run_stopped();

		combineAdjustmentQueue.reset_blocks_coming();

		// Forward and reverse threads sequentially adjust all blocks in
		// forward and reverse directions.
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
		f.reset(new thread(adjust_forward_thread(this, boost::ref(fwd_error))));
		mt_adjust_threads_sp.push_back(f);
		r.reset(new thread(adjust_reverse_thread(this, boost::ref(rev_error))));
		mt_adjust_threads_sp.push_back(r);
#else
		mt_adjust_threads.push_back(boost::thread(adjust_forward_thread(this, boost::ref(fwd_error))));
		mt_adjust_threads.push_back(boost::thread(adjust_reverse_thread(this, boost::ref(rev_error))));
#endif			
		
		// Combination thread fires up as many threads as there are cores
		// on the current PC to concurrently combine blocks
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
		if (CombinationThreadRequired())
		{
			c.reset(new thread(adjust_combine_thread(this, boost::ref(cmb_error))));
			mt_adjust_threads_sp.push_back(c);
		}
#else
		if (CombinationThreadRequired())
			mt_adjust_threads.push_back(boost::thread(adjust_combine_thread(this, boost::ref(cmb_error))));
#endif	

		// Start the clock
		it_time.start();

		// Start the forward, reverse and combine threads, which commences the
		// network adjustment
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
		for_each(mt_adjust_threads_sp.begin(), mt_adjust_threads_sp.end(), boost::mem_fn(&thread::join));
#else
		for_each(mt_adjust_threads.begin(), mt_adjust_threads.end(), boost::mem_fn(&boost::thread::join));
#endif
		// This point is reached when the threads have finished
		iteration_time = milliseconds(it_time.elapsed().wall/MILLI_TO_NANO);

		//delete mt_adjust_threads;
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
		mt_adjust_threads_sp.clear();
#else
		mt_adjust_threads.clear();
#endif
		
		concurrentAdjustments.combine_run_stopped();
				
		// Were any exceptions thrown?  If so, re-throw and let
		// test stub handle the exception
		if (fwd_error)
			// exception thrown in the forward pass
			boost::rethrow_exception(fwd_error);
		else if (rev_error)
			// exception thrown in the reverse pass
			boost::rethrow_exception(rev_error);
		else if (cmb_error)
			// exception thrown in one of the combination adjustments
			boost::rethrow_exception(cmb_error);

		if (IsCancelled())
			break;

		ss.str("");
		if (iteration_time > seconds(1))
			ss << seconds(static_cast<long>(iteration_time.total_seconds()));
		else
			ss << iteration_time;

		///////////////////////////////////
		// protected write to adj file (not needed here since write to
		// adj file at this stage is via single thread
		adj_file << setw(PRINT_VAR_PAD) << left << "Elapsed time" << ss.str() << endl;
		OutputLargestCorrection(corr_msg);
		///////////////////////////////////

		if (projectSettings_.g.verbose)
			debug_file << concurrentAdjustments.print_adjusted_blocks();

		iterationCorrections_.add_message(corr_msg);
		iterationQueue_.push_and_notify(currentIteration_);				// currentIteration begins at 1, so not zero-indexed

		// continue iterating?
		iterate = fabs(maxCorr_) > projectSettings_.a.iteration_threshold;

		// Update normals and measured-computed matrices for the next iteration.
		// Similar to PrepareAdjustment, UpdateAdjustment prepares every block
		// in the network so that forward and reverse adjustments can commence
		// at the same time.
		UpdateAdjustment(iterate);

		if (!iterate)
			break;
	}
	
	isAdjusting_ = false;

	if (adjustStatus_ > ADJUST_TEST_FAILED)
		return;

	// This is similar to the check in ValidateandFinaliseAdjustment API for
	// phased single-thread mode. 
	if (IsCancelled())
	{
		adjustStatus_ = ADJUST_CANCELLED;
		return;
	}

	if (currentIteration_ == projectSettings_.a.max_iterations)
		adjustStatus_ = ADJUST_MAX_ITERATIONS_EXCEEDED;

	// Print status
	PrintAdjustmentStatus();
	// Compute and print time taken to run adjustment
	PrintAdjustmentTime(tot_time, total_time);
}


bool dna_adjust::CombinationThreadRequired()
{
	for (UINT32 block(0); block<blockCount_; ++block) 
	{
		// Combination adjustment is required if there
		// is at least one intermediate block.
		if (v_blockMeta_.at(block)._blockIntermediate)
			return true;
	}
	return false;
}


void dna_adjust::UpdateEstimatesFinalNoCombine()
{
	for (UINT32 block(0); block<blockCount_; ++block) 
		UpdateEstimatesFinal(block);
}


void dna_adjust::SolveMTTry(bool COMPUTE_INVERSE, const UINT32& block)
{
	// Least Squares Solution
	try {            
		SolveMT(COMPUTE_INVERSE, block);
	}
	catch (const runtime_error& e) {

		// could not invert matrix.  throw error

#ifdef _MS_COMPILER_
#pragma region debug_output
#endif
		if (projectSettings_.g.verbose)
		{
			debug_file << "Block " << block + 1 << " (reverse, multi-thread)" << endl;
			debug_file << "Pre-adjustment Estimates" << fixed << setprecision(16) << v_estimatedStationsR_.at(block);
			debug_file << "Design" << fixed << setprecision(16) << v_designR_.at(block);
			debug_file << "Measurements" << fixed << setprecision(16) << v_measMinusCompR_.at(block);
			debug_file << "At * V-inv" << fixed << setprecision(16) << v_AtVinvR_.at(block);
			debug_file << "Normals " << fixed << setprecision(16) << v_normalsR_.at(block) << endl;
			debug_file.flush();
		}
#ifdef _MS_COMPILER_
#pragma endregion debug_output
#endif

		SignalExceptionAdjustment(e.what(), block);
	}
}

void dna_adjust::SolveMT(bool COMPUTE_INVERSE, const UINT32& block)
{
	if (COMPUTE_INVERSE)
	{
		// Compute inverse of normals (aposteriori variance matrix)
		// (AT * V-1 * A)-1
		FormInverseVarianceMatrix(&(v_normalsR_.at(block)));
	}

	// compute weighted "measured minus computed"
	matrix_2d At_Vinv_m(v_designR_.at(block).columns(), 1);
	At_Vinv_m.multiply_mkl(v_AtVinvR_.at(block), "N", v_measMinusCompR_.at(block), "N");

	// Solve corrections from normal equations
	v_correctionsR_.at(block).redim(v_designR_.at(block).columns(), 1);
	v_correctionsR_.at(block).multiply_mkl(v_normalsR_.at(block), "N", At_Vinv_m, "N");

#ifdef _MS_COMPILER_
#pragma region debug_output
#endif	
	// debug output?
	if (projectSettings_.g.verbose > 3)
	{
		dbg_file_mutex.lock();

		UINT32 i;
		switch (projectSettings_.a.adjust_mode)
		{
		case PhasedMode:
			for (i=0; i<v_parameterStationList_.at(block).size(); ++i)
			{
				debug_file << bstBinaryRecords_.at(v_parameterStationList_.at(block).at(i)).stationName;
				if (find(v_ISL_.at(block).begin(), v_ISL_.at(block).end(), v_parameterStationList_.at(block).at(i)) != v_ISL_.at(block).end())
					debug_file << "i ";
				else if (find(v_JSL_.at(block).begin(), v_JSL_.at(block).end(), v_parameterStationList_.at(block).at(i)) != v_JSL_.at(block).end())
						debug_file << "j ";
				else 
					debug_file << " ";				
			}
			break;
		}

		debug_file << endl;
		
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Pre-adjustment Estimates" << fixed << setprecision(16) << v_estimatedStationsR_.at(block);
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Design" << fixed << setprecision(16) << v_designR_.at(block);
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Measurements" << fixed << setprecision(16) << v_measMinusCompR_.at(block);
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "At * V-inv" << fixed << setprecision(16) << v_AtVinvR_.at(block) << endl;
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Normals " << fixed << setprecision(16) << v_normalsR_.at(block) << endl;
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Precisions" << fixed << setprecision(16) << v_normalsR_.at(block) << endl;
		debug_file << "Block " << block + 1 << " (Reverse)";
		debug_file << endl;
		debug_file << "Corrections" << fixed << setprecision(16) << v_correctionsR_.at(block) << endl;
		debug_file.flush();

		dbg_file_mutex.unlock();

#ifdef _MS_COMPILER_
#pragma endregion debug_output
#endif	

	}
}

		
void adjust_forward_thread::operator()()
{
	stringstream ss;
	UINT32 currentBlock;

	concurrentAdjustments.forward_run_started();
	
	try {
		// Set exception
		error_ = boost::exception_ptr();

		for (currentBlock=0; currentBlock<main_adj_->blockCount_; ++currentBlock)
		{
			if (main_adj_->IsCancelled())
				return;

			//ss.str("");
			//ss << "1> Adjusted block " << currentBlock + 1 << " (forward, in isolation)... " << endl;
			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());

			// Check if an exception was thrown in the reverse or combine threads
			if (rev_error || cmb_error)
				return;
			
			// At this point, whether first iteration or not, if currentBlock is the first block, 
			// the normals will have been initialised.  For all later blocks, the normals will contain
			// the contribution of junction station coordinates and variances from preceding blocks.
			// In either case, the block is ready for adjustment.  Junction station coordinates and 
			// variances are carried forward below

			// Least Squares Solution
			main_adj_->SolveTry(true, currentBlock);

			// Since Solve() can be computationally intensive, re-check
			// if an exception was thrown in the reverse or combine threads
			if (rev_error || cmb_error)
				return;
		
			main_adj_->UpdateEstimatesForward(currentBlock);
		
			/////////////////////////////////////
			//// protected write to adj file
			//ss.str("");
			//ss << "1> Forward adjustment (in isolation) of block " << currentBlock + 1 << " complete." << endl;

			//adj_file_mutex.lock();
			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());
			//adj_file_mutex.unlock();
			/////////////////////////////////////

			// OK, now shrink matrices back to normal size
			main_adj_->ShrinkForwardMatrices(currentBlock);

			// This step is needed to carry coordinates and variance estimates for junctions only
			// for the forward run.  It is not needed for the combination stage
			main_adj_->CarryForwardJunctions(currentBlock, currentBlock+1);
	
			// Does this block need combining?
			if (!main_adj_->CombineRequired(currentBlock))
				continue;
				
			// Ok finished adjustment, set adjusted 
			// state of this block in the forward list
			concurrentAdjustments.set_forward_block_adjusted(currentBlock);

			// Has the reverseThread dealt with this block? 
			if (concurrentAdjustments.reverse_block_adjusted(currentBlock))
				// add this block # to the queue and notify the waiting 
				// combine thread
				combineAdjustmentQueue.push_and_notify(currentBlock);
		
			///////////////////////////////////
			// protected write to adj file
			//ss.str("");
			//ss << "1> Forward adjustment of block " << currentBlock << " complete." << endl;

			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());		
			///////////////////////////////////
		}

		concurrentAdjustments.forward_run_finished();

		if (concurrentAdjustments.finished_all_runs())
			combineAdjustmentQueue.queue_exhausted();

		// notify all threads waiting on combineAdjustmentQueue
		combineAdjustmentQueue.notify_all();

		// Reset exception
		error_ = boost::exception_ptr();

	} 
	catch (...) {
		error_ = boost::current_exception();
		if (combineAdjustmentQueue.is_empty())
			combineAdjustmentQueue.push_and_notify(99999999);
		//combineAdjustmentQueue.notify_all();
		return;
	}

	// notify all threads waiting on combineAdjustmentQueue
	//combineAdjustmentQueue.notify_all();

	//this_thread::sleep(milliseconds(10));

	// notify all threads waiting on combineAdjustmentQueue
	//combineAdjustmentQueue.notify_all();	
}
	

void adjust_reverse_thread::operator()()
{
	stringstream ss;
	UINT32 currentBlock(main_adj_->blockCount_-1), block;

	concurrentAdjustments.reverse_run_started();
	
	try {
		// Set exception
		error_ = boost::exception_ptr();

		for (block=0; block<main_adj_->blockCount_; ++block, --currentBlock)
		{
			if (main_adj_->IsCancelled())
				return;

			// Check if an exception was thrown in the reverse or combine threads
			if (fwd_error || cmb_error)
				return;
		
			// if this is a single block, then there is no need to perform a reverse adjustment
			if (!main_adj_->PrepareAdjustmentReverse(currentBlock, true))
				continue;
		
			/////////////////////////////////////
			//// protected write to adj file
			//ss.str("");
			//ss << "2> Adjusted block " << currentBlock + 1 << " (reverse, in isolation)... " << endl;
			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());
			/////////////////////////////////////

			// Backup normals prior to inversion for re-use in combination
			// adjustment... only if a combination is required
			main_adj_->BackupNormals(currentBlock, true);

			// At this point, whether first iteration or not, if currentBlock is the last block, 
			// the normals will have been initialised.  For all subsequent blocks, the normals will contain
			// the contribution of junction station coordinates and variances from preceding blocks.
			// In either case, the block is ready for adjustment.  Junction station coordinates and 
			// variances are carried in the reverse direction below

			// Least Squares Solution
			main_adj_->SolveMTTry(true, currentBlock);

			// Since SolveMT() can be computationally intensive, re-check
			// if an exception was thrown in the forward or combine threads
			if (fwd_error || cmb_error)
				return;
		
			main_adj_->UpdateEstimatesReverse(currentBlock, true);

			/////////////////////////////////////
			//// protected write to adj file
			//ss.str("");
			//ss << "2> Reverse adjustment (in isolation) of block " << currentBlock + 1 << " complete." << endl;

			//adj_file_mutex.lock();
			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());
			//adj_file_mutex.unlock();
			/////////////////////////////////////

			// Now, carry the estimated junction station coordinates and variances 
			// to the next block (to be used in the next loop), except when currentBlock
			// is the first block and currentBlock-1 is an isolated block.
			//
			// Remember - the junction station estimates and variances of the next block
			// (obtained during the forward pass) were copied during the forward pass
			// in CarryStnEstimatesandVariancesForward(..), so no need to re-copy.
			main_adj_->CarryReverseJunctions(currentBlock, currentBlock-1, true);
	
			// Does this block need combining?
			if (!main_adj_->CombineRequired(currentBlock))
			{
				if (main_adj_->FirstBlock(currentBlock))
					main_adj_->UpdateEstimatesFinal(currentBlock);
			
				// This block does not need combination - continue
				continue;
			}

			// Ok finished adjustment, set adjusted 
			// state of this block in the reverse list
			concurrentAdjustments.set_reverse_block_adjusted(currentBlock);			

			// Has the forwardThread dealt with this block? 
			if (concurrentAdjustments.forward_block_adjusted(currentBlock))
				// add this block # to the queue and notify the waiting 
				// combine thread
				combineAdjustmentQueue.push_and_notify(currentBlock);
		
			
			///////////////////////////////////
			// protected write to adj file
			//ss.str("");
			//ss << "2> Reverse adjustment of block " << currentBlock << " complete." << endl;

			//main_adj_->ThreadSafeWritetoAdjFile(ss.str());
			///////////////////////////////////
		}

		concurrentAdjustments.reverse_run_finished();

		if (concurrentAdjustments.finished_all_runs())
			combineAdjustmentQueue.queue_exhausted();

		// Reset exception
		error_ = boost::exception_ptr();

	} 
	catch (...) {
		error_ = boost::current_exception();
		if (combineAdjustmentQueue.is_empty())
			combineAdjustmentQueue.push_and_notify(99999999);
		return;
	}
}
	

void adjust_process_combine_thread::operator()()
{
	stringstream ss;
	UINT32 pseudomsrJSLCount;
	UINT32 currentBlock;

	try {
		while (true)		
		{
			if (main_adj_->IsCancelled())
				return;

			if (combineAdjustmentQueue.is_queue_exhausted())
				break;
			
			// Have any other adjustments failed?
			if (combineAdjustmentExceptionThrown(cmb_errors_, thread_id_))
				return;

			// Wait here until blocks have been placed on the queue
			if (!combineAdjustmentQueue.front_and_pop(currentBlock))
			{
				boost::this_thread::sleep(milliseconds(2));
				continue;
			}

			// Have any other adjustments failed?
			if (combineAdjustmentExceptionThrown(cmb_errors_, thread_id_))
				return;

			main_adj_->isCombining_ = true;
			main_adj_->SetcurrentBlock(currentBlock);

			if (main_adj_->PrepareAdjustmentCombine(currentBlock, pseudomsrJSLCount, true))
			{
				// Least Squares Solution
				main_adj_->SolveMTTry(true, currentBlock);
				main_adj_->UpdateEstimatesCombine(currentBlock, pseudomsrJSLCount, true);
				main_adj_->UpdateEstimatesFinal(currentBlock);
			}
		}

		error_ = boost::exception_ptr();
	}
	catch (...) {
		error_ = boost::current_exception();
		return;
	}
}
	

void adjust_combine_thread::operator()()
{
	main_adj_->isCombining_ = false;
	
	// Wait here until notification has been received
	combineAdjustmentQueue.wait_if_queue_is_empty();

	// Has either a forward or reverse adjustment failed?
	if (fwd_error || rev_error)
		return;

	// OK, at this point, there is a block # in the queue - let's combine!

	// Get number of cores available
	UINT32 thread_id, cores(boost::thread::hardware_concurrency());

	// Set up exception pointers
	vector<boost::exception_ptr> cmb_errors;
	cmb_errors.resize(cores);

	// Create the thread pool
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
	boost::shared_ptr<thread> c;
	vector< boost::shared_ptr<thread> > mt_combine_threads_sp;
#else
	vector<boost::thread> mt_combine_threads;
#endif	

	for (thread_id=0; thread_id<cores; ++thread_id)
	{

#if defined(__ICC) || defined(__INTEL_COMPILER)			// Intel compiler
		c.reset(new thread(
			adjust_process_combine_thread(
				main_adj_,								// pointer to main adjustment object
				thread_id,								// this new thread's ID (1..n, where n = # cores)
				boost::ref(cmb_errors.at(thread_id)),	// reference to this thread's exception_ptr
				boost::ref(cmb_errors))));				// reference to all other threads' exception_ptrs

		mt_combine_threads_sp.push_back(c);
#else
		mt_combine_threads.push_back(boost::thread(
			adjust_process_combine_thread(
				main_adj_,								// pointer to main adjustment object
				thread_id,								// this new thread's ID (1..n, where n = # cores)
				boost::ref(cmb_errors.at(thread_id)),	// reference to this thread's exception_ptr
				boost::ref(cmb_errors))));				// reference to all other threads' exception_ptrs
#endif			
		
	}

#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
	for_each(mt_combine_threads_sp.begin(), mt_combine_threads_sp.end(), boost::mem_fn(&thread::join));
#else
	for_each(mt_combine_threads.begin(), mt_combine_threads.end(), boost::mem_fn(&boost::thread::join));
#endif	

	// cmb_error is set during combineThreadProcessor
	for (UINT32 err=0; err<cores; ++err)
	{
		if (cmb_errors.at(err))
		{
			error_ = cmb_errors.at(err);
			return;
		}
	}	
}



// Prepare adjustment

void adjust_process_prepare_thread::operator()()
{
	stringstream ss;
	UINT32 currentBlock;

	try {
		while (true)		
		{
			if (prepareAdjustmentQueue.is_queue_exhausted())
				break;
			
			// Have any other adjustments failed?
			if (prepareAdjustmentExceptionThrown(prep_errors_, thread_id_))
				return;

			// Wait here until blocks have been placed on the queue
			if (!prepareAdjustmentQueue.front_and_pop(currentBlock))
			{
				boost::this_thread::sleep(milliseconds(2));
				continue;
			}

			// Have any other adjustments failed?
			if (prepareAdjustmentExceptionThrown(prep_errors_, thread_id_))
				return;

			main_adj_->SetcurrentBlock(currentBlock);
			main_adj_->CreateMeasurementTally(currentBlock);
			main_adj_->PrepareAdjustmentBlock(currentBlock, thread_id_);

			if (prepareAdjustmentQueue.is_empty())
				prepareAdjustmentQueue.queue_exhausted();
		}

		error_ = boost::exception_ptr();
	}
	catch (...) {
		error_ = boost::current_exception();
	}
}
	

void adjust_prepare_thread::operator()()
{	
	// load up the queue with the block ids
	prepareAdjustmentQueue.push_data(createIncrementingIntegerVector<UINT32>(main_adj_->blockCount()));

	// OK, at this point, there is a block # in the queue - let's combine!
	// Get number of cores available
	UINT32 thread_id, cores(boost::thread::hardware_concurrency());

	// Set up exception pointers
	vector<boost::exception_ptr> prep_errors;
	prep_errors.resize(cores);

	// Create the thread pool
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
	boost::shared_ptr<thread> p;
	vector< boost::shared_ptr<thread> > mt_prepare_threads_sp;
#else
	vector<boost::thread> mt_prepare_threads;
#endif	
	

	for (thread_id=0; thread_id<cores; ++thread_id)
	{
#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
		p.reset(new thread(
			adjust_process_prepare_thread(
				main_adj_,				// pointer to main adjustment object
				thread_id,				// this new thread's ID (1..n, where n = # cores)
				boost::ref(prep_errors.at(thread_id)),	// reference to this thread's exception_ptr
				boost::ref(prep_errors))));		// reference to all other threads' exception_ptrs

		mt_prepare_threads_sp.push_back(p);
#else
		mt_prepare_threads.push_back(boost::thread(
			adjust_process_prepare_thread(
				main_adj_,				// pointer to main adjustment object
				thread_id,				// this new thread's ID (1..n, where n = # cores)
				boost::ref(prep_errors.at(thread_id)),	// reference to this thread's exception_ptr
				boost::ref(prep_errors))));		// reference to all other threads' exception_ptrs
#endif	
	}

#if defined(__ICC) || defined(__INTEL_COMPILER)		// Intel compiler
	for_each(mt_prepare_threads_sp.begin(), mt_prepare_threads_sp.end(), boost::mem_fn(&thread::join));
#else
	for_each(mt_prepare_threads.begin(), mt_prepare_threads.end(), boost::mem_fn(&boost::thread::join));
#endif	

	// prep_error is set during prepareThreadProcessor
	for (UINT32 err=0; err<cores; ++err)
	{
		if (prep_errors.at(err))
		{
			error_ = prep_errors.at(err);
			return;
		}
	}
}


void dna_adjust::PrepareAdjustmentMultiThread()
{
	prepareAdjustmentQueue.reset_blocks_coming();
	
	boost::thread mt_prepare_thread(adjust_prepare_thread(this, boost::ref(prep_error)));

	// Start the forward, reverse and combine threads, which commences the
	// network adjustment
	mt_prepare_thread.join();

	// Were any exceptions thrown?  If so, re-throw and let
	// test stub handle the exception
	if (prep_error)
		// exception thrown in one of the combination adjustments
		boost::rethrow_exception(prep_error);
}



}	// namespace networkadjust
}	// namespace dynadjust

