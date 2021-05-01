//============================================================================
// Name         : dnageoid_ext.cpp
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
// Description  : AusGeoid Grid File (NTv2) Interpolation library
//============================================================================

#include <dynadjust/dnageoid/dnageoid.hpp>
#include <dynadjust/dnageoid/dnageoid_ext.hpp>

#ifdef _MSC_VER

using namespace dynadjust::geoidinterpolation;

#ifdef BUILD_DNAGEOID_C_DLL

dna_geoid_interpolation theApp;

/////////////////////////////////////////////////////////////////////////////
// External Procedures
// Created for VB6 and unmanaged .NET support 
extern "C" 
{
	// Reads the gridfile header information into memory
	bool DNAGEOID_CAPI DNAGEOID_CreateGridIndex(const char* fileName, const char* fileType, int* status) 
	{
		try {
			*status = 0;
			theApp.CreateGridIndex(fileName, fileType);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Returns the version of the grid file currently in memory
	bool DNAGEOID_CAPI DNAGEOID_ReportGridVersion(char* version, int* status) 
	{
		try {
			*status = 0;
			theApp.ReportGridVersion(version);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}
	

	// Copies the header information for a grid file to a n_file_par struct
	bool DNAGEOID_CAPI DNAGEOID_ReportGridProperties(const char* fileName, const char* fileType, 
		n_file_par* grid_properties, int* status)
	{
		try {
			*status = 0;
			theApp.ReportGridProperties(fileName, fileType, grid_properties);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}
	
	// Creates a NTv2 Grid file from conventional DAT file format
	bool DNAGEOID_CAPI DNAGEOID_CreateNTv2Grid(const char *datFile,
		n_file_par* grid_properties, int* status)
	{
		try {
			*status = 0;
			theApp.CreateNTv2File(datFile, grid_properties);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}


	// Interpolates AusGeoid values using the nominated method of interpolation
	bool DNAGEOID_CAPI DNAGEOID_InterpolatePoint(geoid_point* geoid_var, 
		const int& method, int* status) 
	{
		int transofrmationStatus(0);
		try {
			*status = 0;
			if (method == BICUBIC)
				transofrmationStatus = DNAGEOID_BiCubicTransformation(geoid_var, status);
			else
				transofrmationStatus = DNAGEOID_BiLinearTransformation(geoid_var, status);

			if (!transofrmationStatus)
				return false;
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}		

		return true;
	}

	// Interpolates AusGeoid values using bi linear interpolation and applies the N value to the supplied height
	bool DNAGEOID_CAPI DNAGEOID_BiLinearTransformation(geoid_point* geoid_var, int* status) 
	{
		try {
			*status = 0;
			theApp.BiLinearTransformation(geoid_var);

			if (geoid_var->cVar.IO_Status != ERR_TRANS_SUCCESS)
			{
				*status = geoid_var->cVar.IO_Status;
				return false;
			}

			
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}		

		return true;
	}

	// Interpolates AusGeoid values using bi cubic interpolation and applies the N value to the supplied height
	bool DNAGEOID_CAPI DNAGEOID_BiCubicTransformation(geoid_point* geoid_var, int* status) 
	{
		
		try {
			*status = 0;
			theApp.BiCubicTransformation(geoid_var);

			if (geoid_var->cVar.IO_Status != ERR_TRANS_SUCCESS)
			{
				*status = geoid_var->cVar.IO_Status;
				return false;
			}
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Interpolates AusGeoid values for a file of coordinates using the nominated method of interpolation
	bool DNAGEOID_CAPI DNAGEOID_FileTransformation(const char* fileIn, const char* fileOut, const int& method, 
		const int& intEllipsoidtoOrtho, const int& intDmsFlag, bool exportDnaGeoidFile, int* status)
	{
		try {
			*status = 0;
			theApp.FileTransformation(fileIn, fileOut, method, intEllipsoidtoOrtho, intDmsFlag, exportDnaGeoidFile);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Exports an Binary grid file to a Ascii format
	bool DNAGEOID_CAPI DNAGEOID_ExporttoAscii(char* gridFile, char* gridType, const char* shiftType, char* outputGrid, int* status)
	{
		try {
			*status = 0;
			theApp.ExportToAscii(gridFile, gridType, shiftType, outputGrid, status);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Exports an Ascii grid file to a Binary format
	bool DNAGEOID_CAPI DNAGEOID_ExporttoBinary(char *gridFile, char *gridType, char* shiftType, char *outputGrid, int *status)
	{
		try {
			*status = 0;
			theApp.ExportToBinary(gridFile, gridType, shiftType, outputGrid, status);
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Returns the file progress as a percentage
	bool DNAGEOID_CAPI DNAGEOID_ReturnFileProgress(int *percentComplete, int *status)
	{
		*percentComplete = 0;

		try {
			*status = 0;
			*percentComplete = theApp.ReturnFileProgress();
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Returns the byte offset
	bool DNAGEOID_CAPI DNAGEOID_GetByteOffset(int *bytesRead, int *status)
	{
		*bytesRead = 0;

		try {
			*status = 0;
			*bytesRead = theApp.GetByteOffset();
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	
	// Sets the byte offset
	bool DNAGEOID_CAPI DNAGEOID_SetByteOffset(int *status)
	{
		try {
			*status = 0;
			theApp.SetByteOffset();
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Returns the number of points transformed in the last FileTransformation call
	bool DNAGEOID_CAPI DNAGEOID_PointsInterpolated(int *pointsInterpolated, int *status)
	{
		*pointsInterpolated = 0;

		try {
			*status = 0;
			*pointsInterpolated = theApp.PointsInterpolated();
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}

	// Returns the number of points not transformed in the last FileTransformation call
	bool DNAGEOID_CAPI DNAGEOID_PointsNotInterpolated(int *pointsNotInterpolated, int *status)
	{
		*pointsNotInterpolated = 0;

		try {
			*status = 0;
			*pointsNotInterpolated = theApp.PointsNotInterpolated();
		}
		catch (const NetGeoidException& e) {
			*status = e.error_no();
			return false;
		}

		return true;
	}
	
}

#endif	// BUILD_DNAGEOID_C_DLL


#endif	// _MSC_VER