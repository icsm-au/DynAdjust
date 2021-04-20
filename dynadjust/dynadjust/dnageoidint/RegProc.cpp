/////////////////////////////////////////////
// Roger Fraser

#include "precompile.h"
#include <io.h>
//#include "Defines.h"
#include "RegProc.h"
#include "FileProc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRegProc::CRegProc()
{
}

bool CRegProc::CreatePreferencesFile()
{
	TCHAR szPath[MAX_PATH];
	
	// Get path for each computer, non-user specific and non-roaming data.
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
	{
		TCHAR szTempFileName[MAX_PATH];

		if (!PreferencesFolderExists())
			CreatePreferencesFolder(szPath);

		// Generate a temporary file name within the application folder (product-specific path).
		sprintf(szTempFileName, "%s\\%s", szPath, preferencesFile);
		HANDLE hFile = NULL;
		
		// Open the file.
		if ((hFile = CreateFile(szTempFileName, 
									GENERIC_WRITE, 
									0, 
									NULL, 
									CREATE_ALWAYS, 
									FILE_ATTRIBUTE_NORMAL, 
									NULL)) != INVALID_HANDLE_VALUE)
		{
			// Write temporary data (code omitted).
			CloseHandle(hFile);
			return true;
		}
	}
	return false;
}
	

bool CRegProc::CreatePreferencesFolder(char* szPath)
{
	TCHAR szTempFileName[MAX_PATH];

	if (!PreferencesSubFolderExists(icsmFolder))
	{
		sprintf(szTempFileName, "%s\\%s", szPath, icsmFolder);
		if (!CreateDirectory(szTempFileName, NULL)) 
			return false;
	}
	
	if (!PreferencesSubFolderExists(preferencesFolder))
	{
		sprintf(szTempFileName, "%s\\%s", szPath, preferencesFolder);
		if (!CreateDirectory(szTempFileName, NULL)) 
			return false;
	}

	return true;
}
	

bool CRegProc::PreferencesFileExists()
{
	TCHAR szPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		return false;

	TCHAR szTempFileName[MAX_PATH];

	// Generate a temporary file name within the application folder (product-specific path).
	sprintf(szTempFileName, "%s\\%s", szPath, preferencesFile);

	long hFile;
	CFileProc fp;

	if (fp.Exists(szTempFileName, &hFile))
		return true;
	else
		return false;
}
	

bool CRegProc::PreferencesFolderExists()
{
	return PreferencesSubFolderExists(preferencesFolder);	
}

bool CRegProc::PreferencesSubFolderExists(const char* const preferencesSubFolder)
{
	TCHAR szPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		return false;

	TCHAR szTempFileName[MAX_PATH];

	// Generate a temporary file name within the application folder (product-specific path).
	sprintf(szTempFileName, "%s\\%s", szPath, preferencesSubFolder);

	HANDLE hFile = NULL;
		
	// Open the folder.
	if ((hFile = CreateFile(
					szTempFileName, 
					GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
					NULL, 
					OPEN_EXISTING, 
					FILE_FLAG_BACKUP_SEMANTICS, 
					NULL)) != INVALID_HANDLE_VALUE)
	{
		// Write temporary data (code omitted).
		CloseHandle(hFile);
		return true;
	}
	return false;
}

CString CRegProc::GetPreferencesFolder()
{
	TCHAR szPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		return "";
	return (CString)szPath;
}
	

bool CRegProc::GetTemporaryFileSettings(T_user* theUser)
{
	TCHAR szPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		return false;

	CString strTmp;
	long hfile;
	CFileProc fp;
	
	// Open the preferences file from the application folder (product-specific path).
	FILE* fin;
	TCHAR szTempFileName[MAX_PATH];			
	
	sprintf(szTempFileName, "%s\\%s", szPath, preferencesFile);	
	if (fopen_s(&fin, szTempFileName, "rb") != 0)
		return false;

	// grid file type
	theUser->GridType = GetTFString(fin, 4);

	// ---------------------------------------------------------
	// Grid file path	
	// create default
	theUser->GridFile = noGridSpecified;
	
	// get file path and save if file exists
	strTmp = GetTFString(fin, MAX_PATH);
	if (fp.Exists(strTmp, &hfile))
		theUser->GridFile = strTmp;
	// ---------------------------------------------------------

	// ---------------------------------------------------------
	// Log file path (saved to preferences folder)
	// create default
	theUser->LogFile.Format("%s\\%sNterpolate.log", szPath, preferencesFolder);

	// get file path and save if file exists
	strTmp = GetTFString(fin, MAX_PATH);
	if (fp.Exists(strTmp, &hfile))
		theUser->LogFile = strTmp;
	// ---------------------------------------------------------

	fclose(fin);
	return true;
}
	

void CRegProc::InitialiseSettings(T_user *theUser)
{
	// Distortion grid type
	theUser->GridType = "gsb";

	// Grid file path	
	theUser->GridFile = noGridSpecified;
	
	// Log file path
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		theUser->LogFile.Format("%s\\%sNterpolate.log", szPath, preferencesFolder);
	else
		theUser->LogFile = "";
	
	SaveTemporaryFileSettings(theUser);
}
	

void CRegProc::SaveTemporaryFileSettings(T_user *theUser)
{
	TCHAR szPath[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)))
		return;

	FILE* fout;
	TCHAR szTempFileName[MAX_PATH];

	// Generate a temporary file name within the application folder (product-specific path).
	sprintf(szTempFileName, "%s\\%s", szPath, preferencesFile);

	fopen_s(&fout, szTempFileName, "wb");

	// Distortion grid type
	SaveTFString(fout, (char*)((LPCTSTR)theUser->GridType), 4);

	// Grid file
	SaveTFString(fout, (char*)((LPCTSTR)theUser->GridFile), MAX_PATH);

	// Log file
	SaveTFString(fout, (char*)((LPCTSTR)theUser->LogFile), MAX_PATH);

	fclose(fout);
}


void CRegProc::SaveTFDword(FILE* fout, int i)
{
	fwrite(&i, sizeof(int), 1, fout);
}

int CRegProc::GetTFDword(FILE* fin)
{
	int i;
	fread(&i, sizeof(int), 1, fin);
	return i;
}

void CRegProc::SaveTFString(FILE* fout, char* s, int iLen)
{
	fwrite(s, sizeof(char), iLen, fout);	
}

CString CRegProc::GetTFString(FILE* fin, int iLen)
{
	char* str = new char[iLen+1];
	memset(str, '\0', iLen);
	fread(&(*str), sizeof(char), iLen, fin);
	CString s = str;
	delete []str;
	return s;
}