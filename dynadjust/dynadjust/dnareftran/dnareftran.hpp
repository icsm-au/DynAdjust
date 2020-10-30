//============================================================================
// Name         : dnareftran.hpp
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
// Description  : Reference Frame Transformation library
//============================================================================

#ifndef DNAREFTRAN_H_
#define DNAREFTRAN_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#include <exception>
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/timer/timer.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::filesystem;

#include <include/io/dnaiotpb.hpp>
#include <include/io/dnaiobst.hpp>
#include <include/io/dnaiobms.hpp>
#include <include/io/dnaiodna.hpp>

#include <include/config/dnaexports.hpp>
#include <include/config/dnaversion.hpp>
#include <include/config/dnaversion-stream.hpp>
#include <include/config/dnaconsts.hpp>
#include <include/config/dnaoptions.hpp>
#include <include/exception/dnaexception.hpp>
#include <include/functions/dnatemplatedatetimefuncs.hpp>
#include <include/functions/dnaiostreamfuncs.hpp>
#include <include/functions/dnastringfuncs.hpp>
#include <include/functions/dnastrmanipfuncs.hpp>
#include <include/functions/dnatemplatematrixfuncs.hpp>
#include <include/functions/dnatransparamfuncs.hpp>
#include <include/parameters/dnaepsg.hpp>
#include <include/parameters/dnaellipsoid.hpp>
#include <include/parameters/dnaprojection.hpp>
#include <include/parameters/dnadatum.hpp>
#include <include/parameters/dnatransformationparameters.hpp>
#include <include/parameters/dnaplatemotionmodels.hpp>
#include <include/math/dnamatrix_contiguous.hpp>
#include <include/measurement_types/dnameasurement.hpp>
#include <include/measurement_types/dnageometries.hpp>

using namespace dynadjust::measurements;
using namespace dynadjust::exception;
using namespace dynadjust::iostreams;
using namespace dynadjust::epsg;
using namespace dynadjust::datum_parameters;
using namespace dynadjust::pmm_parameters;
using namespace dynadjust::math;
using namespace dynadjust::geometries;

namespace dynadjust {
namespace referenceframe {

// This class is exported from the dnaSegment.dll
#ifdef _MSC_VER
class DNAREFTRAN_API dna_reftran {
#else
class dna_reftran {
#endif
public:
	dna_reftran();
	virtual ~dna_reftran();

	static void coutVersion();

	void TransformBinaryFiles(const string& bstFile, const string& bmsFile, const string& newFrame, const string& newEpoch="");
	void TransformBinaryStationFile(const string& bstFile, const string& newFrame, const string& newEpoch="");
	void TransformBinaryMeasurementFile(const string& bmsFile, const string& newFrame, const string& newEpoch="");

	// Returns the file progress
	inline int ReturnFileProgress() const { return (int)m_dPercentComplete; }

	// Returns the byte offset
	inline int GetByteOffset() const { return m_iBytesRead; }

	// Sets the byte offset
	inline void SetByteOffset() { m_iBytesRead = 0; }
	inline UINT32 StationsTransformed() const { return m_stnsTransformed; }
	inline UINT32 StationsNotTransformed() const { return m_stnsNotTransformed; }
	inline UINT32 MeasurementsTransformed() const { return m_msrsTransformed; }
	inline UINT32 MeasurementsNotTransformed() const { return m_msrsNotTransformed; }

	// file handling
	void SerialiseDNA(const string& stnfilename, const string& msrfilename, const project_settings& p, bool flagUnused=false);
	void SerialiseDynaML(const string& xmlfilename, const project_settings& p, bool flagUnused=false);
	void SerialiseDynaML(const string& stnfilename, const string& msrfilename, const project_settings& p, bool flagUnused=false);

	void SerialiseDynaMLStn(std::ofstream* xml_file, CDnaProjection& projection, bool flagUnused=false);
	void SerialiseDynaMLMsr(std::ofstream* xml_file);

	bool PrintTransformedStationCoordinatestoSNX(const project_settings& p);

	void Identify_Plate();

	void LoadTectonicPlateParameters(const string& pltfileName, const string& pmmfileName);

private:

	void LoadBinaryStationFile(const string& bstfileName);
	void LoadBinaryMeasurementFile(const string& bmsfileName);

	void WriteBinaryStationFile(const string& bstfileName);
	void WriteBinaryMeasurementFile(const string& bmsfileName);

	void TransformStationRecords(const string& newFrame, const string& newEpoch);
	void TransformMeasurementRecords(const string& newFrame, const string& newEpoch);

	double DetermineElapsedTime(const CDnaDatum& datumFrom, const CDnaDatum& datumTo,
		transformation_parameter_set& transParams, transformationType transType);
	
	void ObtainHelmertParameters(const CDnaDatum& datumFrom, const CDnaDatum& datumTo, 
		transformation_parameter_set& transParams, double& timeElapsed, transformationType transType);
	
	UINT32 DetermineTectonicPlate(const string& plate);

	void ObtainPlateMotionParameters(it_vstn_t& stn_it, double* reduced_parameters, const CDnaDatum& datumFrom, const CDnaDatum& datumTo,
		transformation_parameter_set& transformParameters);

	void JoinTransformationParameters(it_vstn_t& stn_it, double* reduced_parameters, const CDnaDatum& datumFrom, const CDnaDatum& datumTo,
		transformation_parameter_set& transformParameters, transformationType transType);

	void Transform(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod, 
		const CDnaDatum& datumFrom, transformation_parameter_set& transformParameters);

	void TransformDynamic(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod,
		const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
		transformationType transType);

	void TransformFrames_Join(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod,
		const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
		transformationType transType);

	void TransformFrames_PlateMotionModel(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod,
		const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters);

	void TransformFrames_WithoutPlateMotionModel(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod,
		const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters,
		transformationType transType);

	void TransformEpochs_PlateMotionModel(it_vstn_t& stn_it, const matrix_2d& coordinates, matrix_2d& coordinates_mod,
		const CDnaDatum& datumFrom, const CDnaDatum& datumTo, transformation_parameter_set& transformParameters);

	void TransformStation(it_vstn_t& stn_it, const CDnaDatum& datumFrom,
		transformation_parameter_set& transformParameters);
	
	void TransformMeasurement(it_vmsr_t& msr_it, const CDnaDatum& datumFrom,
		transformation_parameter_set& transformParameters);
	void TransformMeasurement_GX(it_vmsr_t& msr_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters);
	void TransformMeasurement_Y(it_vmsr_t& msr_it, const CDnaDatum& datumFrom, 
		transformation_parameter_set& transformParameters);

	void IdentifyStationPlate();
	
	void CalculateRotations();

	double							m_dPercentComplete;			// percentage of bytes read from file
	int								m_iBytesRead;				// bytes read from file
	UINT32							m_stnsTransformed;
	UINT32							m_stnsNotTransformed;
	UINT32							m_msrsTransformed;
	UINT32							m_msrsNotTransformed;

	binary_file_meta_t				bst_meta_;
	binary_file_meta_t				bms_meta_;

	vstn_t							bstBinaryRecords_;
	vmsr_t							bmsBinaryRecords_;
	
	project_settings				projectSettings_;
	CDnaDatum						datumTo_;
	CDnaDatum						datumFrom_;

	bool							transformationPerformed_;

	v_string_v_doubledouble_pair	global_plates_;				// Tectonic plate boundaries
	v_plate_motion_eulers			plate_motion_eulers_;		// Euler parameters corresponding to each plate
	v_plate_motion_cartesians		plate_motion_cartesians_;	// Helmert parameters computed from Euler parameters

	v_string_uint32_pair 			vplateMap_;					// Plate Map index sorted on plate ID
};

}	// namespace referenceframe
}	// namespace dynadjust


#endif /* DNAREFTRAN_H_ */
