//============================================================================
// Name         : dnareftran.cpp
// Author       : Roger Fraser
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
// Description  : Reference Frame Transformation library
//============================================================================

#include <dynadjust/dnareftran/dnareftran.hpp>

namespace dynadjust { 
namespace referenceframe {

dna_reftran::dna_reftran()
{
#ifdef _MSC_VER
#if (_MSC_VER < 1900)
	{
		// this function is obsolete in MS VC++ 14.0, VS2015
		// Set scientific format to print two places for the exponent
		_set_output_format(_TWO_DIGIT_EXPONENT);
	}
#endif
#endif
}

//dna_reftran::dna_reftran(const dna_reftran& newdnaReftran)
//{
//	
//}

dna_reftran::~dna_reftran()
{

}

void dna_reftran::coutVersion()
{
	string msg;
	fileproc_help_header(&msg);
	cout << msg << endl;
}

void dna_reftran::TransformBinaryFiles(const string& bstFile, const string& bmsFile, const string& newFrame, const string& newEpoch)
{
	// TODO - Would it be faster to use memory mapping instead of reading
	// binary files into memory?  Not sure.
	// Either way, using memory mapped files would provide a greater capacity
	// to run extremely large adjustments.

	// 1. Load binary files
	// load the binary station file into memory
	LoadBinaryStationFile(bstFile);
	// load the binary measurement file into memory
	LoadBinaryMeasurementFile(bmsFile);

	datumTo_.SetDatumFromName(newFrame, newEpoch);

	// 2. Transform measurements first (because pre-transformed station 
	// coordinates are required)
	TransformMeasurementRecords(newFrame, newEpoch);

	// Were any measurements updated?
	if (transformationPerformed_)
	{
		// write the binary measurement file
		WriteBinaryMeasurementFile(bmsFile);
	}

	// 3. Transform stations
	TransformStationRecords(newFrame, newEpoch);

	// Were any coordinates updated?
	if (transformationPerformed_)
	{
		// write the binary station file
		WriteBinaryStationFile(bstFile);
	}

}

void dna_reftran::TransformBinaryStationFile(const string& bstFile, const string& newFrame, const string& newEpoch)
{
	SetByteOffset();

	// TODO - Would it be faster to use memory mapping instead of reading
	// binary files into memory?  Not sure.
	// Either way, using memory mapped files would provide a greater capacity
	// to run extremely large adjustments.

	// load the binary station file into memory
	LoadBinaryStationFile(bstFile);
	
	// Transform the station coordinates from one reference frame and epoch to another
	TransformStationRecords(newFrame, newEpoch);

	// Were any coordinates updated?
	if (!transformationPerformed_)
		return;

	// write the binary station file
	WriteBinaryStationFile(bstFile);
}

void dna_reftran::TransformBinaryMeasurementFile(const string& bmsFile, const string& newFrame, const string& newEpoch)
{
	SetByteOffset();
	
	// TODO - Would it be faster to use memory mapping instead of reading
	// binary files into memory?  Not sure.
	// Either way, using memory mapped files would provide a greater capacity
	// to run extremely large adjustments.

	// load the binary measurement file into memory
	LoadBinaryMeasurementFile(bmsFile);

	// Transform the measurements from one reference frame and epoch to another
	TransformMeasurementRecords(newFrame, newEpoch);

	// Were any measurements updated?
	if (!transformationPerformed_)
		return;

	// write the binary measurement file
	WriteBinaryMeasurementFile(bmsFile);

}

void dna_reftran::LoadBinaryStationFile(const string& bstfileName)
{
	try {
		// Load binary stations data.  Throws runtime_error on failure.
		dna_io_bst bst;
		bst.load_bst_file(bstfileName, &bstBinaryRecords_, bst_meta_);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
}


void dna_reftran::WriteBinaryStationFile(const string& bstfileName)
{
	string strEpsg(datumTo_.GetEpsgCode_s());
	string strEpoch(datumTo_.GetEpoch_s());

	// update binary file meta
	sprintf(bst_meta_.modifiedBy, "%s", __BINARY_NAME__);
	sprintf(bst_meta_.epsgCode, "%s", strEpsg.substr(0, STN_EPSG_WIDTH).c_str());
	sprintf(bst_meta_.epoch, "%s", strEpoch.substr(0, STN_EPOCH_WIDTH).c_str());
	
	try {
		// write binary stations data.  Throws runtime_error on failure.
		dna_io_bst bst;
		bst.write_bst_file(bstfileName, &bstBinaryRecords_, bst_meta_);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
}

void dna_reftran::LoadBinaryMeasurementFile(const string& bmsfileName)
{
	try {
		// Load binary measurements data.  Throws runtime_error on failure.
		dna_io_bms bms;
		bms.load_bms_file(bmsfileName, &bmsBinaryRecords_, bms_meta_);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
}
	

void dna_reftran::WriteBinaryMeasurementFile(const string& bmsfileName)
{
	string strEpsg(datumTo_.GetEpsgCode_s());
	string strEpoch(datumTo_.GetEpoch_s());

	// update binary file meta
	sprintf(bms_meta_.modifiedBy, "%s", __BINARY_NAME__);
	sprintf(bms_meta_.epsgCode, "%s", strEpsg.substr(0, STN_EPSG_WIDTH).c_str());
	sprintf(bms_meta_.epoch, "%s", strEpoch.substr(0, STN_EPOCH_WIDTH).c_str());
	
	try {
		// write binary measurement data.  Throws runtime_error on failure.
		dna_io_bms bms;
		bms.write_bms_file(bmsfileName, &bmsBinaryRecords_, bms_meta_);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
}
	

void dna_reftran::ObtainPlateMotionParameters(double* reduced_parameters, 
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters)
{
	transformationType transformation_type = __plate_motion_model__;

	// Use plate motion model to project coordinates between epochs
	// NOTE: This only retrieves the Australian Plate Motion Model.
	// TODO: Need a way of interpolating from a global (ITRF) plate motion
	//       model to determine which plate to use. Otherwise, the wrong
	//       results will be produced if the Aus plate is used to transform
	//       points on other plates.
	determinePlateMotionModelParameters<UINT32>(transformParameters);
	double timeElapsed = DetermineElapsedTime(datumFrom, datumTo, 
		transformParameters, transformation_type);
	ReduceParameters<double>(transformParameters.parameters_, 
		reduced_parameters, timeElapsed);
}
	
// This function is called only when:
//	- there are no direct parameters between the 'from' and 'to' frames.
//  - at least one of the input/output frames is static
// NOTE: Joining uses itrf2014 as the "stepping" frame
// If an exception is thrown, there's not much more we can do
void dna_reftran::JoinTransformationParameters(double* reduced_parameters, 
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
	transformationType transType)
{
	
	transformation_parameter_set transP_a, transP_b;
	CDnaDatum datumStep(epsgCodeFromName<UINT32, string>(ITRF2014_s));
	
	// Set the reference epoch to the epoch of the data
	switch (transType)
	{
	case __static_to_static__:
		// this case doesn't need an epoch
		break;
	case __static_to_dynamic__:
	case __dynamic_to_dynamic__:
		datumStep.SetEpoch(datumTo.GetEpoch());
		break;
	case __dynamic_to_static__:
		datumStep.SetEpoch(datumFrom.GetEpoch());
		break;
	}

	double reduced_parameters_step[7], timeElapsed_b(0.0);
	double timeElapsed = 0.;

	transformationType transformation_type;


	// datumFrom -> Step
	try
	{
		// reset transformation_type
		if (datumFrom.isStatic())
			transformation_type = __static_to_step__;
		else // (datumFrom.isDynamic())
			transformation_type = __dynamic_to_step__;

		// 1. Obtain parameters for the first step (to ITRF2014)
		// The time elapsed will be a function of:
		//		- epoch of datumFrom, and 
		//		- reference epoch of the transformation parameters
		ObtainHelmertParameters(datumFrom, datumStep, transP_a,
			timeElapsed, transformation_type);

		// 2. Reduce the first-step parameters to the appropriate unit and format
		if (datumFrom.isDynamic() || datumStep.isDynamic())
			ReduceParameters<double>(transP_a.parameters_, reduced_parameters, timeElapsed);
		else
			ReduceParameters<double>(transP_a.parameters_, reduced_parameters, timeElapsed, false);

#ifdef _MSDEBUG
		TRACE("Reduced parameters (1):\n");
		TRACE("%11.8f\n", reduced_parameters[0]);
		TRACE("%11.8f\n", reduced_parameters[1]);
		TRACE("%11.8f\n", reduced_parameters[2]);
		TRACE("%11.8g\n", reduced_parameters[3]);
		TRACE("%11.8g\n", reduced_parameters[4]);
		TRACE("%11.8g\n", reduced_parameters[5]);
		TRACE("%11.8g\n\n", reduced_parameters[6]);
#endif

	}
	catch (RefTranException& rft) {
		// No parameters exist
		switch (rft.exception_type())
		{
		case REFTRAN_TRANS_ON_PLATE_REQUIRED:
			ObtainPlateMotionParameters(reduced_parameters, datumFrom, datumTo, transformParameters);
			break;
		default:
			stringstream error_msg;
			error_msg << "Attempting to join parameters between " << 
				datumFrom_.GetName() << " and " << datumStep.GetName() << ":" << endl <<
				"    " << rft.what();
			throw RefTranException(error_msg.str());
		}
	}

	// Step -> datumTo
	try
	{
		// reset transformation_type
		if (datumTo.isStatic())
			transformation_type = __step_to_static__;
		else // (datumFrom.isDynamic())
			transformation_type = __step_to_dynamic__;

		// 3. Obtain parameters for the second step (to "DatumTo_")
		//    timeElapsed_b is set, but not used.
		ObtainHelmertParameters(datumStep, datumTo, transP_b,
			timeElapsed_b, transformation_type);
		
		// 4. Reduce the second-step parameters to the appropriate unit and format
		if (datumStep.isDynamic() || datumTo.isDynamic())
			ReduceParameters<double>(transP_b.parameters_, reduced_parameters_step, timeElapsed_b);
		else
			ReduceParameters<double>(transP_b.parameters_, reduced_parameters_step, timeElapsed_b, false);

#ifdef _MSDEBUG
		TRACE("Reduced parameters (2):\n");
		TRACE("%11.8f\n", reduced_parameters_step[0]);
		TRACE("%11.8f\n", reduced_parameters_step[1]);
		TRACE("%11.8f\n", reduced_parameters_step[2]);
		TRACE("%11.8g\n", reduced_parameters_step[3]);
		TRACE("%11.8g\n", reduced_parameters_step[4]);
		TRACE("%11.8g\n", reduced_parameters_step[5]);
		TRACE("%11.8g\n\n", reduced_parameters_step[6]);
#endif

	}
	catch (RefTranException& rft) {
		// No parameters exist
		switch (rft.exception_type())
		{
		case REFTRAN_TRANS_ON_PLATE_REQUIRED:
			ObtainPlateMotionParameters(reduced_parameters_step, datumFrom, datumTo, transformParameters);
			break;
		default:
			stringstream error_msg;
			error_msg << "Attempting to join parameters between " <<
				datumStep.GetName() << " and " << datumTo.GetName() << ":" << endl <<
				"    " << rft.what();
			throw RefTranException(error_msg.str());
		}
	}

	// 6. Sum the reduced parameters
	reduced_parameters[0] += reduced_parameters_step[0];
	reduced_parameters[1] += reduced_parameters_step[1];
	reduced_parameters[2] += reduced_parameters_step[2];
	reduced_parameters[3] += reduced_parameters_step[3];
	reduced_parameters[4] += reduced_parameters_step[4];
	reduced_parameters[5] += reduced_parameters_step[5];
	reduced_parameters[6] += reduced_parameters_step[6];

#ifdef _MSDEBUG
	TRACE("Joined (1 + 2) parameters:\n");
	TRACE("%11.8f\n", reduced_parameters[0]);
	TRACE("%11.8f\n", reduced_parameters[1]);
	TRACE("%11.8f\n", reduced_parameters[2]);
	TRACE("%11.8g\n", reduced_parameters[3]);
	TRACE("%11.8g\n", reduced_parameters[4]);
	TRACE("%11.8g\n", reduced_parameters[5]);
	TRACE("%11.8g\n\n", reduced_parameters[6]);
#endif

	// Since this was the result of a 'join', force 
	// reinitialisation of datumFrom and datumTo codes
	transformParameters.from_to_ = uint32_uint32_pair(0, 0);
}
	

void dna_reftran::TransformEpochs_PlateMotionModel(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters)
{
	// At this point, a REFTRAN_TRANS_ON_PLATE_REQUIRED exception has been 
	// thrown, caught and re-directed to here.
	// Try propagating the parameters using the Aus Plate Motion Model.
	// If this attempt raises an exception, it will be caught by the 
	// calling method
	double reduced_parameters[7];
	ObtainPlateMotionParameters(reduced_parameters, datumFrom, datumTo, transformParameters);

#ifdef _MSDEBUG
	TRACE("Final reduced parameters:\n");
	TRACE("%11.8f\n", reduced_parameters[0]);
	TRACE("%11.8f\n", reduced_parameters[1]);
	TRACE("%11.8f\n", reduced_parameters[2]);
	TRACE("%11.8g\n", reduced_parameters[3]);
	TRACE("%11.8g\n", reduced_parameters[4]);
	TRACE("%11.8g\n", reduced_parameters[5]);
	TRACE("%11.8g\n\n", reduced_parameters[6]);
#endif

	// Transform!
	Transform_7parameter<double>(coordinates, coordinates_mod, reduced_parameters);

#ifdef _MSDEBUG
	coordinates.trace("coords", "%.4f ");
	coordinates_mod.trace("coords_mod", "%.4f ");
#endif
}
	

void dna_reftran::TransformFrames_PlateMotionModel(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters)
{
	// For this scenario, three steps are involved.  The sequence is very similar to
	// TransformFrames_Join, the exception being the PMM must be used to transform
	// between epochs once the coordinates have been transformed to the step frame (ITRF).
	// The steps in this scenario are:
	//	1. Transform datumFrom to ITRF2014 (using the epoch of the input dynamic frame)
	//	2. Apply PMM (transform from input epoch to output epoch on ITRF2014)
	//  3. Transform ITRF2014 to datumTo (using the epoch of the output dynamic frame)

	// Create the step datum and set the epoch to the epoch of the input data
	CDnaDatum datumStep1(epsgCodeFromName<UINT32, string>(ITRF2014_s), datumFrom.GetEpoch());
	CDnaDatum datumStep2(epsgCodeFromName<UINT32, string>(ITRF2014_s), datumTo.GetEpoch());

	matrix_2d coordinates_tmp(coordinates);

	//	1. Transform datumFrom to ITRF2014 (using the epoch of the input dynamic frame)
	if (datumFrom.GetEpsgCode_i() != datumStep1.GetEpsgCode_i())
	{
		TransformFrames_WithoutPlateMotionModel(coordinates, coordinates_mod, datumFrom, datumStep1,
			transformParameters, __dynamic_to_dynamic__);
		coordinates_tmp = coordinates_mod;
	}

	//	2. Apply PMM (transform from input epoch to output epoch on ITRF2014)
	// Create the step datum and set the epoch to the epoch of the output data
		
	TransformEpochs_PlateMotionModel(coordinates_tmp, coordinates_mod, datumStep1, datumStep2, transformParameters);

	//  3. Transform ITRF2014 to datumTo (using the epoch of the output dynamic frame)
	if (datumStep2.GetEpsgCode_i() != datumTo.GetEpsgCode_i())
	{
		coordinates_tmp = coordinates_mod;
		TransformFrames_WithoutPlateMotionModel(coordinates_tmp, coordinates_mod, datumStep2, datumTo,
			transformParameters, __dynamic_to_dynamic__);
	}
}
	
// Called by:
// - Transform(), when transforming between static-static, static-dynamic and dynamic-static
// - TransformDynamic(), when transforming between dynamic-dynamic when input/output epochs are the same
// Primary scenario - parameters exist between the two frames.
// Secondary scenario - no parameters exist, in which case a REFTRAN_DIRECT_PARAMS_UNAVAILABLE exception is
// thrown and caught, and TransformFrames_Join handles the transformation using ITRF2014 as a step.

void dna_reftran::TransformFrames_WithoutPlateMotionModel(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
	transformationType transType)
{
	double timeElapsed;
	double reduced_parameters[7];

	try {
		// Primary scenario:   parameters exist between datumFrom and datumTo_
		// Secondary scenario: no parameters exist, in which case an exception
		//   of type REFTRAN_DIRECT_PARAMS_UNAVAILABLE will be thrown. In this case,
		//   the exception will be caught and the transformation handled in two steps
		ObtainHelmertParameters(datumFrom, datumTo, transformParameters,
			timeElapsed, transType);

#ifdef _MSDEBUG
		TRACE("Raw parameters:\n");
		TRACE("%11.8f\n", transformParameters.parameters_[0]);
		TRACE("%11.8f\n", transformParameters.parameters_[1]);
		TRACE("%11.8f\n", transformParameters.parameters_[2]);
		TRACE("%11.8f\n", transformParameters.parameters_[3]);
		TRACE("%11.8f\n", transformParameters.parameters_[4]);
		TRACE("%11.8f\n", transformParameters.parameters_[5]);
		TRACE("%11.8f\n", transformParameters.parameters_[6]);
		TRACE("%11.8f\n", transformParameters.parameters_[7]);
		TRACE("%11.8f\n", transformParameters.parameters_[8]);
		TRACE("%11.8f\n", transformParameters.parameters_[9]);
		TRACE("%11.8f\n", transformParameters.parameters_[10]);
		TRACE("%11.8f\n", transformParameters.parameters_[11]);
		TRACE("%11.8f\n", transformParameters.parameters_[12]);
		TRACE("%11.8f\n\n", transformParameters.parameters_[13]);
#endif

		// Reduce the parameters to the appropriate unit and format
		if (datumFrom.isDynamic() || datumTo.isDynamic())
			ReduceParameters<double>(transformParameters.parameters_, reduced_parameters, timeElapsed);
		else
			ReduceParameters<double>(transformParameters.parameters_, reduced_parameters, timeElapsed, false);

#ifdef _MSDEBUG
		TRACE("Final reduced parameters:\n");
		TRACE("%11.8f\n", reduced_parameters[0]);
		TRACE("%11.8f\n", reduced_parameters[1]);
		TRACE("%11.8f\n", reduced_parameters[2]);
		TRACE("%11.8g\n", reduced_parameters[3]);
		TRACE("%11.8g\n", reduced_parameters[4]);
		TRACE("%11.8g\n", reduced_parameters[5]);
		TRACE("%11.8g\n\n", reduced_parameters[6]);
#endif

		// Transform!
		Transform_7parameter<double>(coordinates, coordinates_mod, reduced_parameters);

#ifdef _MSDEBUG
		coordinates.trace("coords", "%.4f ");
		coordinates_mod.trace("coords_mod", "%.4f ");
#endif

	}
	catch (RefTranException& rft) {
		// No parameters exist
		switch (rft.exception_type())
		{
		case REFTRAN_DIRECT_PARAMS_UNAVAILABLE:
			TransformFrames_Join(coordinates, coordinates_mod, datumFrom, datumTo, transformParameters, transType);
			break;
		default:
			throw RefTranException(rft.what());
		}
	}
}


void dna_reftran::TransformDynamic(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
	transformationType transType)
{
	epochSimilarity epoch_similarity;
	frameSimilarity frame_similarity;

	bool applyPMMParameters(false);

	//   If REFTRAN_TRANS_ON_PLATE_REQUIRED is thrown, the exception is caught 
	//   and the plate motion model is applied.  The only problem is, the 
	//   plate motion model currently implemented within DynAdjust is the 
	//   Australian plate motion model. Hence, this cannot be used for
	//   points not on the Australian plate!!!

	// Are input and output epochs different?
	if (datumFrom.GetEpoch() == datumTo.GetEpoch())
		epoch_similarity = __epoch_epoch_same__;
	else
		epoch_similarity = __epoch_epoch_diff__;

	// Are input and output frames different?
	if (datumFrom.GetEpsgCode_i() == datumTo_.GetEpsgCode_i())
		frame_similarity = __frame_frame_same__;
	else
		frame_similarity = __frame_frame_diff__;


	if (frame_similarity == __frame_frame_diff__ &&
		epoch_similarity == __epoch_epoch_same__)
	{
		TransformFrames_WithoutPlateMotionModel(coordinates, coordinates_mod, datumFrom, datumTo,
			transformParameters, transType);
	}
	else
	{
		// if the input and output reference frames are equal, OR
		// if the input and output reference frames are different AND
		// the input and output epochs are different

		TransformFrames_PlateMotionModel(coordinates, coordinates_mod, datumFrom, datumTo,
			transformParameters);
	}
}
	

void dna_reftran::Transform(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, transformation_parameter_set& transformParameters)
{
	transformationType transformation_type;	

	datumFrom_ = datumFrom;

	if (datumFrom.isStatic() && datumTo_.isStatic())
		transformation_type = __static_to_static__;
	else if (datumFrom.isStatic() && datumTo_.isDynamic())
		transformation_type = __static_to_dynamic__;
	else if (datumFrom.isDynamic() && datumTo_.isStatic())
		transformation_type = __dynamic_to_static__;
	else //if (datumFrom.isDynamic() && datumTo_.isDynamic())
		transformation_type = __dynamic_to_dynamic__;

	// Handle transformation based upon type
	switch (transformation_type)
	{
	// 1. Static to static
	//   a) if params exist, direct trans
	//   b) if no params exist, throw/catch, join on ITRF2014
	case __static_to_static__:
	
	// 2. Static to dynamic
	//   a) if params exist, direct trans
	//   b) if no params exist, throw/catch, join on ITRF2014
	case __static_to_dynamic__:

	// 3. Dynamic to static
	//   a) if params exist, direct trans
	//   b) if no params exist, throw/catch, join on ITRF2014
	case __dynamic_to_static__:

		// At least one frame is static
		TransformFrames_WithoutPlateMotionModel(coordinates, coordinates_mod, datumFrom, datumTo_,
			transformParameters, transformation_type);
		break;

	// 4. Dynamic to dynamic
	//   a) if different frames, and epoch_in = epoch_out
	//     i) if params exist, direct trans
	//    ii) if no params exist, throw/catch join on ITRF2014
	//
	//   b) if different frames, and epoch_in != epoch_out 
	//     ** 3 steps involving ITRF2014 and PMM are req'd regardless 
	//        of whether params exist or not
	//     1. trans from-datum to ITRF2014
	//     2. apply PMM between epochs
	//     3. trans ITRF2014 to to-datum
	//   c) if same frame, and epoch_in != epoch_out 
	//     1. apply PMM between epochs
	case __dynamic_to_dynamic__:
		// Both frames are dynamic
		TransformDynamic(coordinates, coordinates_mod, datumFrom, datumTo_,
			transformParameters, transformation_type);
		break;
	}
}
	
// At this point, a REFTRAN_DIRECT_PARAMS_UNAVAILABLE exception has been 
// thrown, caught and re-directed to here.
// Try joining two sets associated with ITRF2014
// If this attempt raises an exception, it will be caught by the 
// calling method
void dna_reftran::TransformFrames_Join(const matrix_2d& coordinates, matrix_2d& coordinates_mod,
	const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
	transformationType transType)
{
	double reduced_parameters[7];
	JoinTransformationParameters(reduced_parameters, datumFrom, datumTo, transformParameters, transType);

	// Transform!
	Transform_7parameter<double>(coordinates, coordinates_mod, reduced_parameters);

#ifdef _MSDEBUG
	coordinates.trace("coords", "%.4f ");
	coordinates_mod.trace("coords_mod", "%.4f ");
#endif
}
	

double dna_reftran::DetermineElapsedTime(const CDnaDatum& datumFrom, const CDnaDatum& datumTo, 
	transformation_parameter_set& transParams, transformationType transType)
{
	stringstream ss;
	double dTime(0.0);

	try
	{
		// Formula is always (Dt = t = t0) where the transformation is in the direction of
		// the published parameters, and where:
		//	  t  = epoch of the 'data' coordinate
		//    t0 = epoch of the reference epoch of the transformation parameters
		//  
		// If the reverse direction is required, then:
		//    t  = epoch of the 'to' coordinate+
		//    parameters are negated
		//
		// For example:
		// 1. From ITRF2008@2016.202 -> To GDA94
		//    - Parameters are Dawson Woods ITRF->GDA94 (ref. epoch t0 = 1994.0)
		//    - parameterSet.paramDirection_ is set to __paramForward__
		//    - elapsedTime is calculated as:
		//        Dt =  t(from) - t0
		//        Dt = 2016.202 - 1994.0
		//
		// 2. From GDA94 -> To ITRF2008@2016.202
		//    - Parameters are Dawson Woods ITRF->GDA94 (ref. epoch t0 = 1994.0)
		//    - parameterSet.paramDirection_ is set to __paramReverse__
		//    - values in parameterSet.parameters_ are multiplied by -1 by reverse() method
		//    - elapsedTime is calculated as:
		//        Dt =    t(to) - t0
		//        Dt = 2016.202 - 1994.0
		//
		// TOD - this is not right.  Since from and to epochs are different,
		//       a step is needed
		// 3. From ITRF2008@2016.202 -> To ITRF2005@2012.112
		//    - Parameters are IERS ITRF2008->ITRF2005 (ref. epoch t0 = 2000.0)
		//    - parameterSet.paramDirection_ is set to __paramForward__
		//    - elapsedTime is calculated as:
		//        Dt =    t(to) - t(from)
		//        Dt = 2016.202 - 2012.112
		//
		// TOD - this is not right.  Since from and to epochs are different,
		//       a step is needed
		// 4. From ITRF2005@2012.112 -> To ITRF2008@2016.202
		//    - Parameters are IERS ITRF2008->ITRF2005 (ref. epoch t0 = 2000.0)
		//    - parameterSet.paramDirection_ is set to __paramReverse__
		//    - values in parameterSet.parameters_ are multiplied by -1 by reverse() method
		//    - elapsedTime is calculated as:
		//        Dt =    t(to) - t(from)
		//        Dt = 2016.202 - 2012.112
		//

#ifdef _MSDEBUG
		ss << "From frame: " << datumFrom.GetName() << " -> to: " << datumTo.GetName();
		TRACE("%s\n", ss.str().c_str());
#endif
		ss.str("");

		date dt;
		double dt0(transParams.reference_epoch_);
		
		switch (transType)
		{
		case __static_to_static__:
			// There is no influence from time on the parameters
			dTime = 0.;

#ifdef _MSDEBUG
			ss << "Static epochs - 0.0 (zero) time difference";
			TRACE("%s\n", ss.str().c_str());
#endif
			return dTime;
			break;

		case __static_to_dynamic__:
		case __dynamic_to_static__:
		
			if (transParams.paramDirection_ == __paramForward__)
				dt = datumFrom.GetEpoch();
			else 
				dt = datumTo.GetEpoch();
			break;

		case __dynamic_to_dynamic__:

			dt = datumFrom.GetEpoch();			
			break;

		case __plate_motion_model__:
			
			// Assume PMM parameters are always given in the positive direction
			dt = datumFrom.GetEpoch();
			dt0 = referenceEpoch<double>(datumTo.GetEpoch());
			break;


			// When a step frame is involved, either the input datum or output datum
			// will be dynamic.  In both cases, the epoch of the coordinates on the 
			// dynamic datum is needed. 

		case __static_to_step__:
			// datumFrom  = static frame
			// datumTo    = step frame (ITRF2014)
			// datumTo_   = dynamic frame*
		case __step_to_dynamic__:
			// datumFrom  = step frame (ITRF2014)
			// datumTo    = dynamic frame
			// datumTo_   = dynamic frame*
			dt = datumTo_.GetEpoch();
			break;
		
		case __dynamic_to_step__:
			// datumFrom_ = dynamic frame*
			// datumFrom  = dynamic frame
			// datumTo    = step frame (ITRF2014)
		case __step_to_static__:
			// datumFrom_ = dynamic frame*
			// datumFrom  = step frame (ITRF2014)
			// datumTo    = static frame
			dt = datumFrom_.GetEpoch();
		}

		dTime = elapsedTime<double>(dt, dt0);

#ifdef _MSDEBUG
		ss << "Epoch from: " << dt << " -> epoch to: " << dt0 <<
			" = " << setprecision(4) << fixed << dTime;
		TRACE("%s\n", ss.str().c_str());
#endif

	}
	catch (std::out_of_range& e)
	{
		throw RefTranException(e.what());
	}
	catch (...)
	{
		stringstream ss;
		ss << "DetermineElapsedTime(): an error occurred whilst computing the elapsed time.";
		throw RefTranException(ss.str());
	}

	return dTime;
}
	

void dna_reftran::ObtainHelmertParameters(const CDnaDatum& datumFrom, const CDnaDatum& datumTo,
	transformation_parameter_set& transformParameters, double& timeElapsed, transformationType transType)
{
	// Does this transformation require a different set of parameters?
	if (transformParameters.from_to_.first != datumFrom.GetEpsgCode_i() ||
		transformParameters.from_to_.second != datumTo.GetEpsgCode_i())
	{		
		transformParameters.from_to_.first = datumFrom.GetEpsgCode_i();
		transformParameters.from_to_.second = datumTo.GetEpsgCode_i();

		// Primary scenario:   parameters exist between datumFrom and datumTo_
		// Secondary scenario: no parameters exist, in which case an exception
		//   of type REFTRAN_DIRECT_PARAMS_UNAVAILABLE will be thrown.
		determineHelmertParameters<UINT32>(transformParameters);
	}

	// TODO - here, timeElapsed is computed for every record.  In the interest of achieving
	// better performance, this function call could be modified such that DetermineElapsedTime
	// is called only when the epochs from the previous measurement are different.
	if (datumFrom.isDynamic() || datumTo.isDynamic())
		timeElapsed = DetermineElapsedTime(datumFrom, datumTo, transformParameters, transType);
	else
		timeElapsed = 0.;
}

void dna_reftran::TransformStationRecords(const string& newFrame, const string& newEpoch)
{
	it_vstn_t stn_it;
	CDnaDatum datumFrom;

	// Create the transformation parameters to be used for the
	// entire set of station records.  If a station is in a different
	// frame, obtain new parameters
	transformation_parameter_set transformationParameters;
	
	transformationPerformed_ = false;
	m_stnsTransformed = m_stnsNotTransformed = 0;

	try {
		// 1. Get the datum (and epoch) of the desired system
		datumTo_.SetDatumFromName(newFrame, newEpoch);
		
		// 2. For every station, get the datum, then transform
		//    TransformStation takes
		for (stn_it=bstBinaryRecords_.begin(); stn_it!=bstBinaryRecords_.end(); ++stn_it)
		{
			// a. Get datum of current station
			if (trimstr(string(stn_it->epoch)).empty())
				datumFrom.SetDatum(stn_it->epsgCode);
			else
				datumFrom.SetDatumFromEpsg(stn_it->epsgCode, stn_it->epoch);

			// b. test if a transformation is required
			if (datumFrom == datumTo_)
			{
				m_stnsNotTransformed++;
				continue;
			}

			// c. Transform
			TransformStation(stn_it, datumFrom, transformationParameters);

			// d. Update meta
			sprintf(stn_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
			sprintf(stn_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());
			transformationPerformed_ = true;
			m_stnsTransformed++;
		}
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
	catch (const RefTranException& e) {
		throw RefTranException(e.what());
	}
}
	

void dna_reftran::TransformStation(it_vstn_t& stn_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters)
{
	matrix_2d coordinates(3, 1), coordinates_mod(3, 1);

	// 1. Convert to cartesian.  Why?  Native form of coordinates in bst file 
	// is geographic
	GeoToCart<double>(
		stn_it->currentLatitude, stn_it->currentLongitude, stn_it->currentHeight, 
		coordinates.getelementref(0, 0), 
		coordinates.getelementref(1, 0),
		coordinates.getelementref(2, 0), 
		datumFrom.GetEllipsoidRef());

	// 2. Transform!
	Transform(coordinates, coordinates_mod, datumFrom, transformParameters);

	// 3. Convert back to geographic
	CartToGeo<double>(coordinates_mod.get(0, 0), 
		coordinates_mod.get(1, 0), coordinates_mod.get(2, 0),
		&stn_it->currentLatitude, &stn_it->currentLongitude, &stn_it->currentHeight, 
		datumTo_.GetEllipsoidRef());
}
	

void dna_reftran::TransformMeasurementRecords(const string& newFrame, const string& newEpoch)
{
	it_vmsr_t msr_it;
	CDnaDatum datumFrom;
	
	// Create the transformation parameters to be used for the
	// entire set of measurement records.  If a measurement is in a different
	// frame, obtain new parameters
	transformation_parameter_set transformationParameters;
	
	transformationPerformed_ = false;
	m_msrsTransformed = m_msrsNotTransformed = 0;

	try {
		// 1. Get the datum (and epoch) of the desired system
		datumTo_.SetDatumFromName(newFrame, newEpoch);
	
		// 2. For every measurement, get the datum, determine parameters, then transform
		for (msr_it=bmsBinaryRecords_.begin(); msr_it!=bmsBinaryRecords_.end(); ++msr_it)
		{
			// a. ignore measurements not subject to geodetic datum
			switch (msr_it->measType)
			{
			// Local reference frame measurements not subject to
			// ellipsoid or reference frame/epoch
			case 'A':	// Horizontal angle
			case 'B':	// Geodetic azimuth
			case 'C':	// Chord dist
			case 'D':	// Directions
			case 'E':	// Ellipsoid arc
			case 'H':	// Orthometric height
			case 'I':	// Astronomic latitude
			case 'J':	// Astronomic longitude
			case 'K':	// Astronomic azimuth
			case 'L':	// Level difference
			case 'M':	// MSL arc
			case 'P':	// Geodetic latitude
			case 'Q':	// Geodetic longitude
			case 'R':	// Ellipsoidal height
			case 'S':	// Slope distance
			case 'V':	// Zenith angle
			case 'Z':	// Vertical angle
				continue;
			}

			if (msr_it->measStart != xMeas)
				continue;

			// b. Get datum of current measurement
			if (trimstr(string(msr_it->epoch)).empty())
				datumFrom.SetDatum(msr_it->epsgCode);
			else
				datumFrom.SetDatumFromEpsg(msr_it->epsgCode, msr_it->epoch);
			
			// c. test if a transformation is required
			if (datumFrom == datumTo_)
			{
				m_msrsNotTransformed++;
				continue;
			}

			// d. Transform!
			TransformMeasurement(msr_it, datumFrom, transformationParameters);
			
			// e. Update meta
			transformationPerformed_ = true;
		}
	}
	catch (const runtime_error& e) {
		stringstream error_msg;
		error_msg << e.what() << endl <<
			"    - Measurement type: " << measurement_name<char, string>(msr_it->measType) << endl <<
			"    - From:             " << bstBinaryRecords_.at(msr_it->station1).stationName << endl <<
			"    - To:               " << bstBinaryRecords_.at(msr_it->station2).stationName << endl <<
			"    - Frame and epoch:  " << datumFromEpsgString<string>(msr_it->epsgCode) << " @ " << 
			msr_it->epoch << endl;
		throw RefTranException(error_msg.str());
	}
	catch (const RefTranException& e) {
		stringstream error_msg;
		error_msg << e.what() << endl <<
			"    - Measurement type: " << measurement_name<char, string>(msr_it->measType) << endl <<
			"    - From:             " << bstBinaryRecords_.at(msr_it->station1).stationName << endl <<
			"    - To:               " << bstBinaryRecords_.at(msr_it->station2).stationName << endl <<
			"    - Frame and epoch:  " << datumFromEpsgString<string>(msr_it->epsgCode) << " @ " <<
			msr_it->epoch << endl;
		throw RefTranException(error_msg.str());
	}
}

void dna_reftran::TransformMeasurement(it_vmsr_t& msr_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters)
{
	// a. ignore measurements not subject to geodetic datum
	switch (msr_it->measType)
	{
	case 'B':	// Geodetic azimuth
	case 'C':	// Chord dist
	case 'E':	// Ellipsoid arc
	case 'I':	// Astronomic latitude
	case 'J':	// Astronomic longitude
	case 'K':	// Astronomic azimuth
	case 'M':	// MSL arc
	case 'P':	// Geodetic latitude
	case 'Q':	// Geodetic longitude
	case 'R':	// Ellipsoidal height
		// Some form of transformation if reference frame/epoch is different?
		break;
	case 'G':	// GPS baseline
	case 'X':	// GPS baseline cluster
		TransformMeasurement_GX(msr_it, datumFrom, transformParameters);
		break;
	case 'Y':	// GPS point cluster
		// 7 or 14 parameter transformation required
		TransformMeasurement_Y(msr_it, datumFrom, transformParameters);
		break;
	}
}


void dna_reftran::TransformMeasurement_GX(it_vmsr_t& msr_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters)
{
	UINT32 cluster_bsl, bsl_count(msr_it->vectorCount1);
	UINT32 covariance_count;
	it_vstn_t stn1_it, stn2_it;

	matrix_2d coordinates1(3, 1), coordinates2(3, 1), 
		coordinates1_mod(3, 1), coordinates2_mod(3, 1);

	//// 1. Get upper triangular a-priori measurements variance matrix
	//matrix_2d vmat;
	//GetGPSVarianceMatrix<it_vmsr_t>(msr_it, vmat);
	
	for (cluster_bsl=0; cluster_bsl<bsl_count; ++cluster_bsl)
	{
		covariance_count = msr_it->vectorCount2;
		
		// Get stations
		stn1_it = bstBinaryRecords_.begin() + msr_it->station1;
		stn2_it = bstBinaryRecords_.begin() + msr_it->station2;

		// 1. Convert station 1 coordinates to cartesian.
		GeoToCart<double>(
			stn1_it->currentLatitude, stn1_it->currentLongitude, stn1_it->currentHeight, 
			coordinates1.getelementref(0, 0), 
			coordinates1.getelementref(1, 0),
			coordinates1.getelementref(2, 0), 
			datumFrom.GetEllipsoidRef());

		// 2. Compute station 2 coordinates from station 1 and GPS vector
		coordinates2 = coordinates1;
		
		// X
		coordinates2.elementadd(0, 0, msr_it->term1);
		msr_it++;

		// Y
		coordinates2.elementadd(1, 0, msr_it->term1);
		msr_it++;

		// Z
		coordinates2.elementadd(2, 0, msr_it->term1);
		
		// 3. Transform station 1
		Transform(coordinates1, coordinates1_mod, datumFrom, transformParameters);
				
		// 5. Transform station 2
		Transform(coordinates2, coordinates2_mod, datumFrom, transformParameters);
		
		// update transformation count
		m_msrsTransformed += 3;

		// Calculate and assign new 'transformed' baseline elements
		// Go back to X element
		msr_it -= 2;
		msr_it->term1 = coordinates2_mod.get(0, 0) - coordinates1_mod.get(0, 0);
		//TRACE("\nTransformed baseline\n");
		//TRACE("%.4f\n", msr_it->term1);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());
		msr_it++;
		
		// Y
		msr_it->term1 = coordinates2_mod.get(1, 0) - coordinates1_mod.get(1, 0);
		//TRACE("%.4f\n", msr_it->term1);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());
		msr_it++;
		// Z
		msr_it->term1 = coordinates2_mod.get(2, 0) - coordinates1_mod.get(2, 0);
		//TRACE("%.4f\n", msr_it->term1);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());

		// skip covariances until next baseline
		if (covariance_count > 0)
			msr_it += covariance_count * 3;

		if (covariance_count > 0)
			msr_it++;
	}
			
}

void dna_reftran::TransformMeasurement_Y(it_vmsr_t& msr_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters)
{
	UINT32 cluster_pnt, pnt_count(msr_it->vectorCount1);
	UINT32 covariance_count;

	matrix_2d coordinates(3, 1), coordinates_mod(3, 1);
	
	_COORD_TYPE_ coordType(CDnaStation::GetCoordTypeC(msr_it->coordType));

	for (cluster_pnt=0; cluster_pnt<pnt_count; ++cluster_pnt)
	{
		covariance_count = msr_it->vectorCount2;
		
		// Get X
		coordinates.put(0, 0, msr_it->term1);
		msr_it++;

		// Get Y
		coordinates.put(1, 0, msr_it->term1);
		msr_it++;

		// Get Z
		coordinates.put(2, 0, msr_it->term1);

		// Go back to X element
		msr_it -= 2;
		
		// LLH?  Convert to Cartesian?
		if (coordType == LLH_type_i || coordType == LLh_type_i)
		{
			GeoToCart<double>(
				coordinates.get(0, 0), 
				coordinates.get(1, 0),
				coordinates.get(2, 0), 
				coordinates.getelementref(0, 0), 
				coordinates.getelementref(1, 0),
				coordinates.getelementref(2, 0), 
				datumFrom.GetEllipsoidRef());
		}
		
		// Transform
		Transform(coordinates, coordinates_mod, datumFrom, transformParameters);

		// update transformation count
		m_msrsTransformed += 3;

		// LLH?  Convert to geographic?
		if (coordType == LLH_type_i || coordType == LLh_type_i)
		{
			CartToGeo<double>(
				coordinates_mod.get(0, 0), 
				coordinates_mod.get(1, 0),
				coordinates_mod.get(2, 0), 
				coordinates_mod.getelementref(0, 0), 
				coordinates_mod.getelementref(1, 0),
				coordinates_mod.getelementref(2, 0), 
				datumFrom.GetEllipsoidRef());
		}
		
		// Assign 'transformed' elements
		msr_it->term1 = coordinates_mod.get(0, 0);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());
		msr_it++;
		// Y
		msr_it->term1 = coordinates_mod.get(1, 0);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());
		msr_it++;
		// Z
		msr_it->term1 = coordinates_mod.get(2, 0);
		sprintf(msr_it->epsgCode, "%s", datumTo_.GetEpsgCode_s().c_str());
		sprintf(msr_it->epoch, "%s", datumTo_.GetEpoch_s().c_str());

		// skip covariances until next point		
		if (covariance_count > 0)
			msr_it += covariance_count * 3;

		if (covariance_count > 0)
			msr_it++;
	}
			
}

void dna_reftran::SerialiseDNA(const string& stnfilename, const string& msrfilename, 
								const project_settings& p, bool flagUnused)
{
	CDnaProjection projection;
	try {
		// write binary stations data.  Throws runtime_error on failure.
		dna_io_dna dna;
		string comment(" transformed to ");
		comment.append(datumTo_.GetName());
		if (datumTo_.isDynamic())
			comment.append(", epoch ").append(datumTo_.GetEpoch_s());
		comment.append(".  Exported by reftran.");
		dna.write_dna_files(&bstBinaryRecords_, &bmsBinaryRecords_, 
			stnfilename, msrfilename, p, 
			datumTo_, projection, flagUnused, "Station coordinates" + comment, "GNSS measurements" + comment);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
}

void dna_reftran::SerialiseDynaML(const string& stnfilename, const string& msrfilename, 
	const project_settings& p, bool flagUnused)
{
	// Open DynaML Station file
	std::ofstream dynaml_stn_file;
	try {
		// Create DynaML station file.  Throws runtime_error on failure.
		file_opener(dynaml_stn_file, stnfilename);
		// write header
		dynaml_header(dynaml_stn_file, "Station File", datumTo_.GetName(), datumTo_.GetEpoch_s());
		// Write header comment line
		string comment("Station coordinates transformed to ");
		comment.append(datumTo_.GetName());
		if (datumTo_.isDynamic())
			comment.append(", epoch ").append(datumTo_.GetEpoch_s());
		comment.append(".  Exported by reftran.");
		dynaml_comment(dynaml_stn_file, comment);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}

	CDnaProjection projection;
	
	// Write DynaML Station file.  Throws runtime_error on failure.
	try {
		SerialiseDynaMLStn(&dynaml_stn_file, projection, flagUnused);
		dynaml_footer(dynaml_stn_file);
		dynaml_stn_file.close();
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}

	// Open DynaML Measurement file
	std::ofstream dynaml_msr_file;
	try {
		// Create DynaML measurement file.  Throws runtime_error on failure.
		file_opener(dynaml_msr_file, msrfilename);
		// write header
		dynaml_header(dynaml_msr_file, "Measurement File", datumTo_.GetName(), datumTo_.GetEpoch_s());
		// Write header comment line
		string comment("GNSS measurements transformed to ");
		comment.append(datumTo_.GetName());
		if (datumTo_.isDynamic())
			comment.append(", epoch ").append(datumTo_.GetEpoch_s());
		comment.append(".  Exported by reftran.");
		dynaml_comment(dynaml_msr_file, comment);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}
	
	// Write DynaML Measurement file.  Throws runtime_error on failure.
	try {
		SerialiseDynaMLMsr(&dynaml_msr_file);
		dynaml_footer(dynaml_msr_file);
		dynaml_msr_file.close();
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}	
}
	

void dna_reftran::SerialiseDynaML(const string& xmlfilename, 
	const project_settings& p, bool flagUnused)
{
	std::ofstream dynaml_xml_file;
	try {
		// Create DynaML station and measurement file.  Throws runtime_error on failure.
		file_opener(dynaml_xml_file, xmlfilename);
		// write header
		dynaml_header(dynaml_xml_file, "Combined File", datumTo_.GetName(), datumTo_.GetEpoch_s());
		// Write header comment line
		string comment("Station coordinates and measurements transformed to ");
		comment.append(datumTo_.GetName());
		if (datumTo_.isDynamic())
			comment.append(", epoch ").append(datumTo_.GetEpoch_s());
		comment.append(".  Exported by reftran.");
		dynaml_comment(dynaml_xml_file, comment);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}

	

	CDnaProjection projection;

	try {
		// Write combined DynaML station and binary measurement data.  Throws runtime_error on failure.

		//1. Stations
		SerialiseDynaMLStn(&dynaml_xml_file, projection, flagUnused);

		// 2. Measurements
		SerialiseDynaMLMsr(&dynaml_xml_file);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}

	dynaml_footer(dynaml_xml_file);
	dynaml_xml_file.close();


}

void dna_reftran::SerialiseDynaMLStn(std::ofstream* xml_file, CDnaProjection& projection, bool flagUnused/*=false*/)
{
	UINT32 epsgCode(LongFromString<UINT32>(bstBinaryRecords_.at(0).epsgCode));
	string datum(datumFromEpsgCode<string, UINT32>(epsgCode));
	string epoch(referenceepochFromEpsgCode<UINT32>(epsgCode));
	dnaStnPtr stnPtr(new CDnaStation(datum, epoch));
	
	it_vstn_t _it_stn;

	if (flagUnused) 
	{
		for (_it_stn=bstBinaryRecords_.begin(); _it_stn!=bstBinaryRecords_.end(); ++_it_stn)
		{
			if (_it_stn->unusedStation)
				continue;
			stnPtr->SetStationRec(*_it_stn);
			stnPtr->WriteDNAXMLStnCurrentEstimates(xml_file,
				datumTo_.GetEllipsoidRef(), &projection, dynaml);
		}
	}
	else
	{
		for (_it_stn=bstBinaryRecords_.begin(); _it_stn!=bstBinaryRecords_.end(); ++_it_stn)
		{
			stnPtr->SetStationRec(*_it_stn);
			stnPtr->WriteDNAXMLStnCurrentEstimates(xml_file,
				datumTo_.GetEllipsoidRef(), &projection, dynaml);
		}	
	}
}
	

void dna_reftran::SerialiseDynaMLMsr(std::ofstream* xml_file)
{
	dnaMsrPtr msrPtr;
	it_vmsr_t _it_msr;
	
	for (_it_msr=bmsBinaryRecords_.begin(); _it_msr!=bmsBinaryRecords_.end(); ++_it_msr)
	{
		ResetMeasurementPtr<char>(&msrPtr, _it_msr->measType);
		msrPtr->SetMeasurementRec(bstBinaryRecords_, _it_msr);
		msrPtr->WriteDynaMLMsr(xml_file);
	}
}

bool dna_reftran::PrintTransformedStationCoordinatestoSNX(const project_settings& p)
{
	std::ofstream sinex_file;

	try {
		// Open output file stream.  Throws runtime_error on failure.
		file_opener(sinex_file, p.o._snx_file);
	}
	catch (const runtime_error& e) {
		throw RefTranException(e.what());
	}

	//dna_io_snx snx;

	sinex_file.close();

	return true;
}


}	// namespace referenceframe
}	// namespace dynadjust
