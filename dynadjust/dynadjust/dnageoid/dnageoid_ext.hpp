//============================================================================
// Name         : dnageoid_ext.hpp
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

#ifndef DNAGEOID_C_EXT_H_
#define DNAGEOID_C_EXT_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

const int ERR_AUS_BINARY              = -6;
const int ERR_GRID_PARAMETERS         = -5;
const int ERR_GRID_MEMORY             = -4;
const int ERR_GRID_UNOPENED           = -3;
const int ERR_FINDSUBGRID_GRIDERR     = -2;
const int ERR_FINDSUBGRID_OUTSIDE     = -1;
const int ERR_TRANS_SUCCESS            = 0;
const int ERR_GRIDFILE_READ            = 1;
const int ERR_GRIDFILE_TYPE            = 2;
const int ERR_GRID_CORRUPT             = 3;
const int ERR_INFILE_READ              = 4;
const int ERR_OUTFILE_WRITE            = 5;
const int ERR_INFILE_TYPE              = 6;
const int ERR_OUTFILE_TYPE             = 7;
const int ERR_NODATA_READ              = 8;
const int ERR_LINE_TOO_SHORT           = 9;
const int ERR_INVALID_INPUT            = 10;
const int ERR_READING_DATA             = 11;
const int ERR_INVALID_ZONE             = 12;
const int ERR_PT_OUTSIDE_GRID          = 13;
const int ERR_GRIDFILE_ERROR           = 14;
const int ERR_READ_ASC_SHIFT           = 15;
const int ERR_READ_BIN_SHIFT           = 16;
const int ERR_NUM_CSV_FIELDS           = 17;
const int ERR_QIF_COORD_MATCH          = 18;
const int ERR_QIF_COORD_READ           = 19;
const int ERR_QIF_NODATA_READ          = 20;
const int ERR_RES_COORD_MATCH          = 21;
const int ERR_RES_COORD_TYPE           = 22;
const int ERR_INTERPOLATION_TYPE       = 23;

// interpolation method
typedef enum _GEOID_INTERPOLATION_TYPE_
{
	BILINEAR = 0,
	BICUBIC = 1
} GEOID_INTERPOLATION_TYPE;


///////////////////////////////////////////////////
// Define typedef struct interpolant
typedef struct {
	double dLatitude;		// Latitude of the interpolation point
	double dLongitude;		// Longitude of the interpolation point
	double dHeight;			// Height of the interpolation point (either Orthometric or Ellipsoidal)
	int iDatum;				// ANS/GRS80
	int iHeightSystem;		// 0 (zero) indicates Orthometric, 1 (one) Ellipsoidal
	int IO_Status;			// Holds status type for Point Input Output success\n
							/* -1:	The grid file has not been opened. Returned when a call is made to BiLinearTransformation without prviously calling CreateGridIndex.
								0:	Success...Point read and written.
								1:	Could not read from grid file...perhaps a wrong format or file is corrupted
								2:	Unable to read this type of grid file
								3:	The grid file is corrupt
							   12:	Invalid Zone...cannot transform to geographic from grid coords
							   13:	Point is outside limits of the grid
							   14:	There is an error in the grid
							   15:	Could not retrieve shifts from Ascii file
							   16:	Could not retrieve shifts from Binary file
							*/
} interpolant;

// AusGeoid values
typedef struct {
	double dN_value;			// N value
	double dDefl_meridian;		// Deflection in the prime meridian
	double dDefl_primev;		// Deflection in the prime vertical
} geoid_values;

typedef struct {
	interpolant	cVar;			// The interpolation point
	geoid_values gVar;			// AusGeoid values for this point
} geoid_point;


///////////////////////////////////////////////////
// NTv2 specific typedef struct gridfileindex
typedef struct gridfileindex {
public:
	gridfileindex()
		: dSlat(-165600.), dNlat(-28800.), dElong(-576000.), dWlong(-388800.)
		, dLatinc(60.), dLonginc(60.), lGscount(0)
	{
		strcpy(chSubname, "AUSGEOID");
		strcpy(chParent, "NONE    ");
		strcpy(chCreated, "01012010");
		strcpy(chUpdated, "01012010");
	}

	char chSubname[9];  		// name of subgrid (SUB_NAME)
	char chParent[9];			// name of parent grid (PARENT)
	char chCreated[9];			// date of creation (CREATED)
	char chUpdated[9];			// date of last file update (UPDATED)
	double dSlat;				// lower latitude (S_LAT)
	double dNlat;				// upper latitude (N_LAT)
	double dElong;				// lower longitude (E_LONG)
	double dWlong;				// upper longitude (W_LONG)
	double dLatinc;				// latitude interval (LAT_INC)
	double dLonginc;			// longitude interval (LONG_INC)
	long lGscount;				// number of nodes (GS_COUNT)
	int iGridPos;				// position of first record of grid shifts
} n_gridfileindex; 

///////////////////////////////////////////////////
// NTv2 specific typedef struct filepar
typedef struct file_par {
public:
	file_par()
		: ptrIndex(0), daf(6378137.), dat(6378137.), dbf(6356752.314), dbt(6356752.314)
		, iH_info(11), iSubH_info(11), iNumsubgrids(1), Can_Format(true)
	{
		memset(filename, 0, 601);
		strcpy(filetype, "GSB");
		strcpy(chGs_type, "SECONDS");
		strcpy(chVersion, "1.0.0.0");
		strcpy(chSystem_f, "GDA94   ");
		strcpy(chSystem_t, "AHD_1971");
	}

	n_gridfileindex* ptrIndex;	// pointer to an index (an array) of grid sub files
	char filename[601];			// Distortion grid file name
	char filetype[4];			// Distortion grid file type (ascii: "asc" = 0; binary: "gsb" = 1)
	char chGs_type[9];			// grid shift type (GS_TYPE)
	char chVersion[9];			// grid file version (VERSION)
	char chSystem_f[9];			// reference system (SYSTEM_F)
	char chSystem_t[9];			// reference system (SYSTEM_T)
	double daf;					// semi major of from system (MAJOR_F)
	double dat;					// semi major of to system (MAJOR_T)
	double dbf;					// semi minor of from system (MINOR_F)
	double dbt;					// semi minor of to system (MINOR_T)
	double dLat;				// user input latitude
	double dLatacc;				// interpolated latitude accuracy
	double dLong;				// user input longitude
	double dLongacc;			// interpolated longitude accuracy
	int iH_info;				// Number of header identifiers (NUM_OREC)
	int iSubH_info;				// Number of sub-header idents (NUM_SREC)
	int iNumsubgrids;			// number of subgrids in file (NUM_FILE)
	size_t iGfilelength;		// Size of the grid file in bytes
	int iPointFlag;				// a four way flag indicating the location of the point within subgrid
	int iTheGrid;				// The array index of the most dense sub grid
	bool Can_Format;			// Format of Binary Grid File (true = Canadian; false = Australian)
} n_file_par;



#ifdef _MSC_VER

// define BUILD_DNAGEOID_DLL when building libdnageoid.dll
#ifdef BUILD_DNAGEOID_C_DLL

/* DLL export */
#define DNAGEOID_CAPI __declspec(dllexport) __stdcall
#else
/* EXE import */
#define DNAGEOID_CAPI __declspec(dllimport) __stdcall
#endif


extern "C" {
	// Reads the gridfile header information into memory
	bool DNAGEOID_CAPI DNAGEOID_CreateGridIndex(const char* fileName, const char* fileType, int* status);

	// Returns the version of the grid file currently in memory
	bool DNAGEOID_CAPI DNAGEOID_ReportGridVersion(char* version, int* status);

	// Copies the header information for a grid file to a n_file_par struct
	bool DNAGEOID_CAPI DNAGEOID_ReportGridProperties(const char* fileName, const char* fileType, n_file_par* grid_properties, int* status);
	
	// Creates a NTv2 Grid file from conventional DAT file format
	bool DNAGEOID_CAPI DNAGEOID_CreateNTv2Grid(const char *datFile, n_file_par* grid_properties, int* status);
	
	// Interpolates AusGeoid values using the nominated method of interpolation
	bool DNAGEOID_CAPI DNAGEOID_InterpolatePoint(geoid_point* geoid_var, const int& method, int* status);
	
	// Interpolates AusGeoid values using bi linear interpolation and applies the N value to the supplied height
	bool DNAGEOID_CAPI DNAGEOID_BiLinearTransformation(geoid_point* geoid_var, int* status);
	
	// Interpolates AusGeoid values using bi cubic interpolation and applies the N value to the supplied height
	bool DNAGEOID_CAPI DNAGEOID_BiCubicTransformation(geoid_point* geoid_var, int* status);
	
	// Interpolates AusGeoid values for a file of coordinates using the nominated method of interpolation
	bool DNAGEOID_CAPI DNAGEOID_FileTransformation(const char* fileIn, const char* fileOut, const int& method, const int& intEllipsoidtoOrtho, const int& intDmsFlag, bool exportDnaGeoidFile, int* status);
	
	// Exports an Binary grid file to a Ascii format
	bool DNAGEOID_CAPI DNAGEOID_ExporttoAscii(char *gridFile, char *gridType, char *outputGrid, int *status);
	
	// Exports an Ascii grid file to a Binary format
	bool DNAGEOID_CAPI DNAGEOID_ExporttoBinary(char *gridFile, char *gridType, char *outputGrid, int *status);

	// Returns the file progress as a percentage
	bool DNAGEOID_CAPI DNAGEOID_ReturnFileProgress(int *percentComplete, int *status);

	// Returns the byte offset
	bool DNAGEOID_CAPI DNAGEOID_GetByteOffset(int *bytesRead, int *status);

	// Sets the byte offset
	bool DNAGEOID_CAPI DNAGEOID_SetByteOffset(int *status);

	// Returns the number of points transformed in the last FileTransformation call
	bool DNAGEOID_CAPI DNAGEOID_PointsInterpolated(int *bytesRead, int *status);
	
	// Returns the number of points not transformed in the last FileTransformation call
	bool DNAGEOID_CAPI DNAGEOID_PointsNotInterpolated(int *bytesRead, int *status);
}

#endif	// _MSC_VER



#endif  // DNAGEOID_C_EXT_H_