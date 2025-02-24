//============================================================================
// Name         : driver.cpp
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
// Description  : AusGeoid Grid File (NTv2) Interpolation library Test Stub
//============================================================================

// When building driver, dnageoid.dll needs to be built with standard C API:

//  1. uncomment '//#define BUILD_DNAGEOID_C_DLL' to define BUILD_DNAGEOID_C_DLL (in dnageoid.hpp)
//
//  2. add def file to project:
//     .\dynadjust_1_00_00\dynadjust\dnageoid\dnageoid_ext.def
//     dnageoid > Properties > Configuration Properties > Linker > Input > Module Definition File = dnageoid_ext.def
//
//     Note that this def file doesn't need to be included in order to build driver, but
//     should be included to permit backward compatibility.  That is, by using the def file, new versions 
//     of dnageoid can be rebuilt without having to rebuild the driver.


#include <ostream>
#include <iostream>
#include <iomanip>
#include <string>

#include <dynadjust/dnageoid/dnageoid_ext.hpp>

bool CreateNTv2Grid(const char* datGridFile, const char* gsbGridFile)
{
	std::cout << "+ Creating NTv2 grid file... ";
	
	int status(0);
	n_file_par theGrid;

	strcpy(theGrid.filename, gsbGridFile);
	strcpy(theGrid.filetype, "gsb");

	if (!DNAGEOID_CreateNTv2Grid(datGridFile, &theGrid, &status))
	{
		std::cout << std::endl << "Error: " << status << std::endl;
		return false;
	}

	std::cout << "done." << std::endl << std::endl;
	
	return true;
	
}

	
bool createGridIndex(const char* gridfilePath, const char* gridfileType)
{
	std::cout << "+ Opening grid file... ";

	int status(0);

	if (!DNAGEOID_CreateGridIndex(gridfilePath, gridfileType, &status))
	{
		std::cout << "Error: " << status << std::endl;
		return false;
	}

	std::cout << "done." << std::endl << std::endl;

	return true;
}
		

bool reportGridProperties(const char* gridfilePath, const char* gridfileType)
{
	std::cout << "+ Reporting grid file properties... ";

	n_file_par grid_properties;
	
	int status(0);

	if (!DNAGEOID_ReportGridProperties(gridfilePath, gridfileType, &grid_properties, &status))
	{
		std::cout << "Error: " << status << std::endl;
		return false;
	}

	std::cout << std::endl << std::endl <<
		"----------------------------------------------------------------" << 
	 	std::endl <<
		"Grid properties for \"" << gridfilePath << "\":" << std::endl;
	std::cout << "+ GS_TYPE  = " << grid_properties.chGs_type << std::endl;			// grid shift type (GS_TYPE)
	std::cout << "+ VERSION  = " << grid_properties.chVersion << std::endl;			// grid file version (VERSION)
	std::cout << "+ SYSTEM_F = " << grid_properties.chSystem_f << std::endl;			// reference system (SYSTEM_F)
	std::cout << "+ SYSTEM_T = " << grid_properties.chSystem_t << std::endl;			// reference system (SYSTEM_T)
	std::cout << "+ MAJOR_F  = " << std::setprecision(3) << std::fixed << grid_properties.daf << std::endl;					// semi major of from system (MAJOR_F)
	std::cout << "+ MAJOR_T  = " << std::setprecision(3) << std::fixed << grid_properties.dat << std::endl;					// semi major of to system (MAJOR_T)
	std::cout << "+ MINOR_F  = " << std::setprecision(3) << std::fixed << grid_properties.dbf << std::endl;					// semi minor of from system (MINOR_F)
	std::cout << "+ MINOR_T  = " << std::setprecision(3) << std::fixed << grid_properties.dbt << std::endl;					// semi minor of to system (MINOR_T)
	std::cout << "+ NUM_OREC = " << grid_properties.iH_info << std::endl;				// Number of header identifiers (NUM_OREC)
	std::cout << "+ NUM_SREC = " << grid_properties.iSubH_info << std::endl;				// Number of sub-header idents (NUM_SREC)
	std::cout << "+ NUM_FILE = " << grid_properties.iNumsubgrids << std::endl;			// number of subgrids in file (NUM_FILE)

	for (int i=0; i<grid_properties.iNumsubgrids; ++i)
	{
		std::cout << "  + SUBGRID " << i << ":" << std::endl;
		std::cout << "    SUB_NAME = " << grid_properties.ptrIndex[i].chSubname << std::endl;  		// name of subgrid (SUB_NAME)
		std::cout << "    PARENT   = " << grid_properties.ptrIndex[i].chParent << std::endl;			// name of parent grid (PARENT)
		std::cout << "    CREATED  = " << grid_properties.ptrIndex[i].chCreated << std::endl;			// date of creation (CREATED)
		std::cout << "    UPDATED  = " << grid_properties.ptrIndex[i].chUpdated << std::endl;			// date of last file update (UPDATED)
		std::cout << "    S_LAT    = " << grid_properties.ptrIndex[i].dSlat << std::endl;				// lower latitude (S_LAT)
		std::cout << "    N_LAT    = " << grid_properties.ptrIndex[i].dNlat << std::endl;				// upper latitude (N_LAT)
		std::cout << "    E_LONG   = " << grid_properties.ptrIndex[i].dElong << std::endl;			// lower longitude (E_LONG)
		std::cout << "    W_LONG   = " << grid_properties.ptrIndex[i].dWlong << std::endl;			// upper longitude (W_LONG)
		std::cout << "    LAT_INC  = " << grid_properties.ptrIndex[i].dLatinc << std::endl;			// latitude interval (LAT_INC)
		std::cout << "    LONG_INC = " << grid_properties.ptrIndex[i].dLonginc << std::endl;			// longitude interval (LONG_INC)
		std::cout << "    GS_COUNT = " << grid_properties.ptrIndex[i].lGscount << std::endl;			// number of nodes (GS_COUNT)
	}
	std::cout << "----------------------------------------------------------------" << 
	 	std::endl << std::endl;

	return true;
}

bool interpolateGridPoint(geoid_point* theGeoidPoint, const int& method)
{
	int status(0);

	if (!DNAGEOID_InterpolatePoint(theGeoidPoint, method, &status))
	{
		if (status == -1)
		{
			std::cout << "+ The point " << std::fixed << std::setprecision(6) << theGeoidPoint->cVar.dLatitude << ", " << theGeoidPoint->cVar.dLongitude << " is outside the grid file." << std::endl << std::endl;
			return true;
		}
		std::cout << "Error: " << status << std::endl;
		return false;
	}	
	
	std::cout << "+ Interpolation results for ";
	std::cout << std::fixed << std::setprecision(6) << theGeoidPoint->cVar.dLatitude << ", " << theGeoidPoint->cVar.dLongitude << " (ddd.dddddd):" << std::endl;

	std::cout << "  N VALUE      = " << std::setw(6) << std::right << std::setprecision(3) << theGeoidPoint->gVar.dN_value << std::endl;			// N value
	std::cout << "  DEFL PRIME M = " << std::setw(6) << std::right << theGeoidPoint->gVar.dDefl_meridian << std::endl;						// Defl PM value
	std::cout << "  DEFL PRIME V = " << std::setw(6) << std::right << theGeoidPoint->gVar.dDefl_primev << std::endl;						// Defl PV value
	std::cout << std::endl;

	return true;

}

bool FileTransformation(const char* fileIn, const char* fileOut, const int& intDmsFlag=0)
{
	std::cout << "+ Interpolating geoid information using a points file... ";

	
	int method(BICUBIC);
	int intEllipsoidtoOrtho(0);
	bool exportDnaGeoidFile(false);
	int status(0);
	
	if (!DNAGEOID_FileTransformation(fileIn, fileOut, method, 
		intEllipsoidtoOrtho, intDmsFlag, exportDnaGeoidFile, &status))
	{
		std::cout << "Error: " << status << std::endl;
		return false;
	}	

	std::cout << "done." << std::endl <<
		"+ File written to:" << std::endl << 
		"  " << fileOut << std::endl << std::endl;
	
	int pointsInterpolated(0), pointsNotInterpolated(0);

	if (!DNAGEOID_PointsInterpolated(&pointsInterpolated, &status))
	{
		std::cout << "Error: " << status << std::endl;
		return false;
	}

	if (!DNAGEOID_PointsNotInterpolated(&pointsNotInterpolated, &status))
	{
		std::cout << "Error: " << status << std::endl;
		return false;
	}

	std::cout << "+ Points interpolated:     " << pointsInterpolated << std::endl <<
		"+ Points not interpolated: " << pointsNotInterpolated << std::endl << std::endl;

	return true;
}
	

int main(int argc, char* argv[])
{
	char datGridFile[256];
	strcpy(datGridFile, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\AUSGeoid09_NSW_ACT_CLIP.dat");
	
	char gsbGridFile[256];
	sprintf(gsbGridFile, "%s%s", datGridFile, ".gsb");

	strcpy(gsbGridFile, "C:\\Data\\dist\\ausgeoid09.gsb");

	// Create a NTv2 grid file from a Winter DAT file
	//if (!CreateNTv2Grid(datGridFile, gsbGridFile))
	//	return EXIT_FAILURE;

	// Print a grid file's properties

	if (!reportGridProperties(gsbGridFile, "gsb"))
		return EXIT_FAILURE;
	
	// 1. Call createGridIndex() once
	if (!createGridIndex(gsbGridFile, "gsb"))
		return EXIT_FAILURE;

	geoid_point theGeoidPoint;

	// Decimal degrees
	theGeoidPoint.cVar.dLatitude = -37.678634;
	theGeoidPoint.cVar.dLongitude = 147.678634;
	theGeoidPoint.cVar.dHeight = 100.0;
	theGeoidPoint.cVar.iHeightSystem = 0;

	// 2. Call interpolateGridPoint() as many times as required
	std::cout << "+ Interpolating from grid file... " << std::endl;

	for (int i=0; i<4; ++i)
	{
		theGeoidPoint.cVar.dLatitude += 1.0 * i;
		theGeoidPoint.cVar.dLongitude += 2.0 * i;

		if (!interpolateGridPoint(&theGeoidPoint, BICUBIC))
			return EXIT_FAILURE;
	}
	
	
	// 3. As required, call FileTransformation
	char fileIn[256], fileOut[256];
	
	// 19 points (one bad) in decimal degrees
	strcpy(fileIn, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\balltran.csv");
	strcpy(fileOut, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\balltran_out.txt");
	FileTransformation(fileIn, fileOut, 1);

	// 10 points in decimal degrees
	strcpy(fileIn, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\gda94_ddeg.csv");
	strcpy(fileOut, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\gda94_ddeg_out.csv");
	FileTransformation(fileIn, fileOut, 1);

	// 10 points in degrees, minutes and seconds
	strcpy(fileIn, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\gda94_dms.csv");
	strcpy(fileOut, "C:\\Data\\vs10\\projects\\geoidDllTest\\sample_files\\gda94_dms_out.csv");
	FileTransformation(fileIn, fileOut);

	return EXIT_SUCCESS;
}
