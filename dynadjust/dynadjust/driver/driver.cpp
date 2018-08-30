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

using namespace std;

bool CreateNTv2Grid(const char* datGridFile, const char* gsbGridFile)
{
	cout << "+ Creating NTv2 grid file... ";
	
	int status(0);
	n_file_par theGrid;

	strcpy(theGrid.filename, gsbGridFile);
	strcpy(theGrid.filetype, "gsb");

	if (!DNAGEOID_CreateNTv2Grid(datGridFile, &theGrid, &status))
	{
		cout << endl << "Error: " << status << endl;
		return false;
	}

	cout << "done." << endl << endl;
	
	return true;
	
}

	
bool createGridIndex(const char* gridfilePath, const char* gridfileType)
{
	cout << "+ Opening grid file... ";

	int status(0);

	if (!DNAGEOID_CreateGridIndex(gridfilePath, gridfileType, &status))
	{
		cout << "Error: " << status << endl;
		return false;
	}

	cout << "done." << endl << endl;

	return true;
}
		

bool reportGridProperties(const char* gridfilePath, const char* gridfileType)
{
	cout << "+ Reporting grid file properties... ";

	n_file_par grid_properties;
	
	int status(0);

	if (!DNAGEOID_ReportGridProperties(gridfilePath, gridfileType, &grid_properties, &status))
	{
		cout << "Error: " << status << endl;
		return false;
	}

	cout << endl << endl <<
		"----------------------------------------------------------------" << 
		endl <<
		"Grid properties for \"" << gridfilePath << "\":" << endl;
	cout << "+ GS_TYPE  = " << grid_properties.chGs_type << endl;			// grid shift type (GS_TYPE)
	cout << "+ VERSION  = " << grid_properties.chVersion << endl;			// grid file version (VERSION)
	cout << "+ SYSTEM_F = " << grid_properties.chSystem_f << endl;			// reference system (SYSTEM_F)
	cout << "+ SYSTEM_T = " << grid_properties.chSystem_t << endl;			// reference system (SYSTEM_T)
	cout << "+ MAJOR_F  = " << setprecision(3) << fixed << grid_properties.daf << endl;					// semi major of from system (MAJOR_F)
	cout << "+ MAJOR_T  = " << setprecision(3) << fixed << grid_properties.dat << endl;					// semi major of to system (MAJOR_T)
	cout << "+ MINOR_F  = " << setprecision(3) << fixed << grid_properties.dbf << endl;					// semi minor of from system (MINOR_F)
	cout << "+ MINOR_T  = " << setprecision(3) << fixed << grid_properties.dbt << endl;					// semi minor of to system (MINOR_T)
	cout << "+ NUM_OREC = " << grid_properties.iH_info << endl;				// Number of header identifiers (NUM_OREC)
	cout << "+ NUM_SREC = " << grid_properties.iSubH_info << endl;				// Number of sub-header idents (NUM_SREC)
	cout << "+ NUM_FILE = " << grid_properties.iNumsubgrids << endl;			// number of subgrids in file (NUM_FILE)

	for (int i=0; i<grid_properties.iNumsubgrids; ++i)
	{
		cout << "  + SUBGRID " << i << ":" << endl;
		cout << "    SUB_NAME = " << grid_properties.ptrIndex[i].chSubname << endl;  		// name of subgrid (SUB_NAME)
		cout << "    PARENT   = " << grid_properties.ptrIndex[i].chParent << endl;			// name of parent grid (PARENT)
		cout << "    CREATED  = " << grid_properties.ptrIndex[i].chCreated << endl;			// date of creation (CREATED)
		cout << "    UPDATED  = " << grid_properties.ptrIndex[i].chUpdated << endl;			// date of last file update (UPDATED)
		cout << "    S_LAT    = " << grid_properties.ptrIndex[i].dSlat << endl;				// lower latitude (S_LAT)
		cout << "    N_LAT    = " << grid_properties.ptrIndex[i].dNlat << endl;				// upper latitude (N_LAT)
		cout << "    E_LONG   = " << grid_properties.ptrIndex[i].dElong << endl;			// lower longitude (E_LONG)
		cout << "    W_LONG   = " << grid_properties.ptrIndex[i].dWlong << endl;			// upper longitude (W_LONG)
		cout << "    LAT_INC  = " << grid_properties.ptrIndex[i].dLatinc << endl;			// latitude interval (LAT_INC)
		cout << "    LONG_INC = " << grid_properties.ptrIndex[i].dLonginc << endl;			// longitude interval (LONG_INC)
		cout << "    GS_COUNT = " << grid_properties.ptrIndex[i].lGscount << endl;			// number of nodes (GS_COUNT)
	}
	cout << "----------------------------------------------------------------" << 
		endl << endl;

	return true;
}

bool interpolateGridPoint(geoid_point* theGeoidPoint, const int& method)
{
	int status(0);

	if (!DNAGEOID_InterpolatePoint(theGeoidPoint, method, &status))
	{
		if (status == -1)
		{
			cout << "+ The point " << fixed << setprecision(6) << theGeoidPoint->cVar.dLatitude << ", " << theGeoidPoint->cVar.dLongitude << " is outside the grid file." << endl << endl;
			return true;
		}
		cout << "Error: " << status << endl;
		return false;
	}	
	
	cout << "+ Interpolation results for ";
	cout << fixed << setprecision(6) << theGeoidPoint->cVar.dLatitude << ", " << theGeoidPoint->cVar.dLongitude << " (ddd.dddddd):" << endl;

	cout << "  N VALUE      = " << setw(6) << right << setprecision(3) << theGeoidPoint->gVar.dN_value << endl;			// N value
	cout << "  DEFL PRIME M = " << setw(6) << right << theGeoidPoint->gVar.dDefl_meridian << endl;						// Defl PM value
	cout << "  DEFL PRIME V = " << setw(6) << right << theGeoidPoint->gVar.dDefl_primev << endl;						// Defl PV value
	cout << endl;

	return true;

}

bool FileTransformation(const char* fileIn, const char* fileOut, const int& intDmsFlag=0)
{
	cout << "+ Interpolating geoid information using a points file... ";

	
	int method(BICUBIC);
	int intEllipsoidtoOrtho(0);
	bool exportDnaGeoidFile(false);
	int status(0);
	
	if (!DNAGEOID_FileTransformation(fileIn, fileOut, method, 
		intEllipsoidtoOrtho, intDmsFlag, exportDnaGeoidFile, &status))
	{
		cout << "Error: " << status << endl;
		return false;
	}	

	cout << "done." << endl <<
		"+ File written to:" << endl << 
		"  " << fileOut << endl << endl;
	
	int pointsInterpolated(0), pointsNotInterpolated(0);

	if (!DNAGEOID_PointsInterpolated(&pointsInterpolated, &status))
	{
		cout << "Error: " << status << endl;
		return false;
	}

	if (!DNAGEOID_PointsNotInterpolated(&pointsNotInterpolated, &status))
	{
		cout << "Error: " << status << endl;
		return false;
	}

	cout << "+ Points interpolated:     " << pointsInterpolated << endl <<
		"+ Points not interpolated: " << pointsNotInterpolated << endl << endl;

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
	cout << "+ Interpolating from grid file... " << endl;

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
