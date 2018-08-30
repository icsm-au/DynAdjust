/////////////////////////////////////////////
// Roger Fraser

#include "precompile.h"
#include <winbase.h>
#include <windows.h>
#include <shlobj.h>
#include <ole2.h>
#include <io.h>
//#include "Defines.h"
#include "FileProc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lp, LPARAM pData) 
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg) 
	{
	case BFFM_INITIALIZED: 
		if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir)) 
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
		break;
	case BFFM_SELCHANGED: 
		if (SHGetPathFromIDList((LPITEMIDLIST) lp, szDir)) 
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	default:
		break;
	}
	return 0;
}

CFileProc::CFileProc()
{
	m_fIndex = NULL;
	m_intNumFiles = 1;
	m_lTotalBytes = 0;

	m_sError = "No error";
	m_dwError = 0;
	m_bAskIfReadOnly = true;
	m_bOverwriteMode = false;
	m_bAborted = false;
	m_iRecursionLimit = -1;
	m_strDestinationFile = "";
}

CFileProc::~CFileProc()
{
	ClearFileArrayMemory();
}

void CFileProc::BuildFileArray(const char* filedir, const vector<string>& filetypes, const int& no_filetypes)
{
	char chFilter[_MAX_PATH];
	struct _finddata_t _fFile;    
	int iNumFile, success, FileCount;

	FileCount = 0;
	success = iNumFile = 0;
	
	for (int i=0; i<no_filetypes; i++)
	{
		sprintf(chFilter, "%s\\*.%s", filedir, filetypes.at(i).c_str());
		
		// how many are there
		iNumFile = success = (int)_findfirst(chFilter, &_fFile );
			
		while (success > -1)
		{
			if (_fFile.attrib != _A_SUBDIR)
				FileCount++;
			
			if (success > -1)
				success = _findnext(iNumFile, &_fFile);
		}
	}

	// If there are none
	if (FileCount == 0)
		return;

	if (m_fIndex == NULL)
		m_fIndex = new _fileindex[FileCount + 1];
	if (m_fIndex == NULL)
		return;		// could not allocate memory

	memset(m_fIndex, '\0', sizeof(_fileindex) * FileCount);
	m_intNumFiles = 0;

	// iterate a second time storing the names
	for (int i=0; i<no_filetypes; i++)
	{
		sprintf(chFilter, "%s\\*.%s", filedir, filetypes.at(i).c_str());
		
		// how many are there
		iNumFile = success = (int)_findfirst(chFilter, &_fFile );
			
		while (success > -1)
		{
			if (_fFile.attrib != _A_SUBDIR)
			{
				strcpy(m_fIndex[m_intNumFiles].chfilename, _fFile.name);
				m_intNumFiles++;
				m_lTotalBytes += _fFile.size;
			}
			
			if (success > -1)
				success = _findnext(iNumFile, &_fFile);
		}
	}	
}
	

void CFileProc::EnterSingleFile(char *filename)
{
	// Only a single file to be used	
	m_fIndex = new _fileindex[1];
	memset(m_fIndex, '\0', sizeof(_fileindex));

	strcpy_s(m_fIndex[0].chfilename, _MAX_PATH, filename);
	m_intNumFiles = 1;
}

CString CFileProc::BrowseForDialog(CString msg, HWND hwndOwner)
{
	CString strPath = "";
	LPITEMIDLIST pidlSelected = NULL;
	BROWSEINFO bi = {0};
	LPMALLOC pMalloc = NULL;	
	SHGetMalloc(&pMalloc);
	
	OleInitialize(NULL);
	
	if (pMalloc != NULL)
	{
		ZeroMemory(&bi, sizeof(bi));
		bi.hwndOwner = hwndOwner;
		bi.pidlRoot = 0;
		bi.pszDisplayName = 0;
		bi.lpszTitle = msg;
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_USENEWUI | BIF_VALIDATE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
		pidlSelected = SHBrowseForFolder(&bi);
		if(pidlSelected)
		{
			SHGetPathFromIDList(pidlSelected, m_strDirectory);
			strPath = m_strDirectory;
		}
		pMalloc->Free(pidlSelected);
		pMalloc->Release();
	}
	return strPath;
}

CString CFileProc::FileOpenSave(BOOL OPEN_DLG, BOOL MULTI_FILE, LPCTSTR lpszDir, char *cNewFile, LPCTSTR lpszTitle, LPCTSTR lpszExt, LPCTSTR strFilter, int iFileType)
{
	char szFile[MAX_FILEPATH] = "";
	DWORD Flags;
	int i;
	
	typedef struct tagOFNA { // ofn
		DWORD         nFilterIndex; 
		LPTSTR        lpstrFile; 
		DWORD         nMaxFile; 
		LPCTSTR       lpstrInitialDir;
		LPCTSTR       lpstrTitle; 
		DWORD         Flags; 
		LPCTSTR       lpstrDefExt;
	} OPENFILENAME;

	memset(szFile, '\0', sizeof(szFile));

	if (OPEN_DLG)			// Open File
	{
		if (MULTI_FILE)
			Flags = OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;	// multi files
		else				
			Flags = OFN_HIDEREADONLY;							// single file
	}
	else					// Save File
		Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		
	CFileDialog m_ldFile(OPEN_DLG, lpszExt, cNewFile, Flags, strFilter, NULL);
	
	if (MULTI_FILE)
	{
		m_ldFile.m_ofn.lpstrFile = szFile;
		strcpy(szFile, cNewFile);
		m_ldFile.m_ofn.nMaxFile = sizeof(szFile);	
	}
	else
	{
		m_ldFile.m_ofn.lpstrFile = cNewFile;
	}

	//Initialise the starting variables for the open / save as file dialog box
	m_ldFile.m_ofn.nFilterIndex = iFileType;
	m_ldFile.m_ofn.lpstrInitialDir = lpszDir;
	m_ldFile.m_ofn.lpstrTitle = lpszTitle;
	m_ldFile.m_ofn.lpstrDefExt = lpszExt;
	
	//Show the File Open dialog and return the result
	if (m_ldFile.DoModal() == IDOK)
	{
		m_strFileExt = m_ldFile.GetFileExt();
		
		m_intNumFiles = 1;
		if (MULTI_FILE)
		{
			m_intNumFiles = 0;
			POSITION pos = m_ldFile.GetStartPosition();

			while (pos != NULL)
			{
				 m_ldFile.GetNextPathName(pos);
				 m_intNumFiles++;
			}

			if (m_intNumFiles == 0)
				return "";

			ClearFileArrayMemory();
			m_fIndex = new _fileindex[m_intNumFiles + 1];
			
			if (m_fIndex == NULL)
				return "";		// could not allocate memory

			memset(m_fIndex, '\0', sizeof(_fileindex) * (m_intNumFiles + 1));

			// iterate a second time storing the names
			pos = m_ldFile.GetStartPosition();
			i = 0;
			while (pos != NULL)
			{
				 strcpy(m_fIndex[i].chfilename, m_ldFile.GetNextPathName(pos));
				 i ++;
			}
			
			m_strFileExt = GetExt(m_fIndex[0].chfilename);
			return m_fIndex[0].chfilename;
		}
		else
			return m_ldFile.GetPathName();
	}
	else
		return "";
}



void CFileProc::ClearFileArrayMemory()
{
	// check for existence of allcoated memory for _fileindex struct
	if (m_fIndex != NULL)
	{
		// delete and NULL pointer to sruct
		delete[] m_fIndex;
		m_fIndex = NULL;
	}	
}


CString CFileProc::GetFolder(CString strFilepath)
{
	int i;
	
	i = strFilepath.ReverseFind('\\');
	
	if (i == -1)
		return "";
	else
		return strFilepath.Mid(0, i);
}

CString CFileProc::GetExt(CString strFilepath)
{
	int i;
	
	i = strFilepath.ReverseFind('.');
	
	if (i == -1)
		return "";
	else
		return strFilepath.Mid(i + 1);
}

CString CFileProc::GetName(CString strFilepath, LPCTSTR lpszExt)
{
	int i;

	i = strFilepath.ReverseFind('\\');
	
	if (i != -1)
		strFilepath = strFilepath.Mid(i+1);
	else
		return "";

	if (lpszExt == NULL)
		return strFilepath;

	i = strFilepath.ReverseFind('.');

	if (i == -1)
		strFilepath += '.' + lpszExt;
	else
		strFilepath = strFilepath.Mid(0, i+1) + lpszExt;

	return strFilepath;
}

int CFileProc::Exists(CString strFilepath, long *hFile)
{
	struct _finddata_t file_t;

	if (hFile == NULL)		// do not need handle
		return (int)_findfirst((char*)((LPCTSTR)strFilepath), &file_t);
	if ((*hFile = (long)_findfirst((char*)((LPCTSTR)strFilepath), &file_t)) != -1L)
		return 1;
	else
		return 0;
}

// use for points files only
int CFileProc::FilterIndex(CString strExt)
{
	if (!strExt.CompareNoCase(CSV))
		return 2;
	else if (!strExt.CompareNoCase(QIF))
		return 3;
	else if (!strExt.CompareNoCase(MAN))
		return 4;
	else if (!strExt.CompareNoCase(RES))
		return 5;
	else
		return 1;		// txt, dat, prn
}


int CFileProc::View(CString strFilepath, CString f_viewer)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	long hFile;

	if(!Exists(strFilepath, &hFile))
		return -1;

	strFilepath.Insert(0, " \"");
	strFilepath.Insert(0, f_viewer);
	strFilepath +="\"";
	
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;

	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpReserved2 = NULL;
	si.cbReserved2 = 0;
	si.lpTitle = (char*)((LPCTSTR)strFilepath); 
	si.lpDesktop = (LPTSTR)NULL;
	si.dwFlags = STARTF_FORCEONFEEDBACK;

	// run process
	if (!CreateProcess(NULL, (char*)((LPCTSTR)strFilepath), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
						NULL, (LPTSTR)NULL, &si, &pi))		
		return 0;

	// close process and thread handles
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 1;
}


// File Operations
bool CFileProc::Copy(CString sSource, CString sDest)
{
	if (CheckSelfCopy(sSource, sDest)) return true;
	bool bRes;

	bRes = DoCopy(sSource, sDest);

	m_iRecursionLimit = -1;
	return bRes;
}
	

bool CFileProc::CheckSelfCopy(CString sSource, CString sDest)
{
	bool bRes = false;
	if (CheckPath(sSource) == PATH_IS_FOLDER)
	{
		CString sTmp = sSource;
		int pos = sTmp.ReverseFind('\\');
		if (pos != -1)
		{
			sTmp.Delete(pos, sTmp.GetLength() - pos);
			if (sTmp.CompareNoCase(sDest) == 0) bRes = true;
		}
	}
	return bRes;
}
	

int CFileProc::CheckPath(CString sPath)
{
	DWORD dwAttr = GetFileAttributes(sPath);
	if (dwAttr == 0xffffffff) 
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) 
			return PATH_NOT_FOUND;
		return PATH_ERROR;
	}
	if (dwAttr & FILE_ATTRIBUTE_DIRECTORY) 
		return PATH_IS_FOLDER;
	return PATH_IS_FILE;
}
	

bool CFileProc::DoCopy(CString sSource, CString sDest, bool bDelteAfterCopy)
{
	CheckSelfRecursion(sSource, sDest);
	// source not found
	if (CheckPath(sSource) == PATH_NOT_FOUND)
	{
		CString sError = sSource + CString(" not found");
		return false;
	}
	// dest not found
	if (CheckPath(sDest) == PATH_NOT_FOUND)
	{
		CString sError = sDest + CString(" not found");
		return false;
	}
	// folder to file
	if (CheckPath(sSource) == PATH_IS_FOLDER && CheckPath(sDest) == PATH_IS_FILE) 
		return false;
	
	// folder to folder
	if (CheckPath(sSource) == PATH_IS_FOLDER && CheckPath(sDest) == PATH_IS_FOLDER) 
	{
		CFileFind ff;
		CString sError = sSource + CString(" not found");
		PreparePath(sSource);
		PreparePath(sDest);
		sSource += "*.*";
		if (!ff.FindFile(sSource)) 
		{
			ff.Close();
			return false;
		}
		if (!ff.FindNextFile()) 
		{
			ff.Close();
			return false;
		}
		CString sFolderName = ParseFolderName(sSource);
		if (!sFolderName.IsEmpty()) // the source is not drive
		{
			sDest += sFolderName;
			PreparePath(sDest);
			if (!CreateDirectory(sDest, NULL))
			{
				DWORD dwErr = GetLastError();
				if (dwErr != 183)
				{
					ff.Close();
					return false;
				}
			}
		}
		ff.Close();
		DoFolderCopy(sSource, sDest, bDelteAfterCopy);
	}
	// file to file
	if (CheckPath(sSource) == PATH_IS_FILE && CheckPath(sDest) == PATH_IS_FILE) 
	{
		if (!DoFileCopy(sSource, sDest))
			return false;
	}

	// file to folder
	if (CheckPath(sSource) == PATH_IS_FILE && CheckPath(sDest) == PATH_IS_FOLDER) 
	{
		PreparePath(sDest);
		char drive[MAX_PATH], dir[MAX_PATH], name[MAX_PATH], ext[MAX_PATH];
		_splitpath(sSource, drive, dir, name, ext);
		sDest = sDest + CString(name) + CString(ext);
		if (!DoFileCopy(sSource, sDest))
			return false;
	}
	return true;
}
	

bool CFileProc::DoFileCopy(CString sSourceFile, CString sDestFile, bool bDelteAfterCopy)
{
	BOOL bOvrwriteFails = FALSE;
	if (!m_bOverwriteMode)
	{
		while (IsFileExist(sDestFile)) 
		{
			sDestFile = ChangeFileName(sDestFile);
		}
		bOvrwriteFails = TRUE;
	}
	if (!CopyFile(sSourceFile, sDestFile, bOvrwriteFails)) 
	{
		m_strDestinationFile = "";
		return false;
	}

	m_strDestinationFile = sDestFile;

	if (bDelteAfterCopy)
	{
		DoDelete(sSourceFile);
	}
	return true;
}
	

bool CFileProc::DoFolderCopy(CString sSourceFolder, CString sDestFolder, bool bDelteAfterCopy)
{
	CFileFind ff;
	CString sPathSource = sSourceFolder;
	BOOL bRes = ff.FindFile(sPathSource);
	while (bRes)
	{
		bRes = ff.FindNextFile();
		if (ff.IsDots()) continue;
		if (ff.IsDirectory()) // source is a folder
		{
			if (m_iRecursionLimit == 0) continue;
			sPathSource = ff.GetFilePath() + CString("\\") + CString("*.*");
			CString sPathDest = sDestFolder + ff.GetFileName() + CString("\\");
			if (CheckPath(sPathDest) == PATH_NOT_FOUND) 
			{
				if (!CreateDirectory(sPathDest, NULL))
				{
					ff.Close();
					return false;
				}
			}
			if (m_iRecursionLimit > 0) 
				m_iRecursionLimit --;
			if (!DoFolderCopy(sPathSource, sPathDest, bDelteAfterCopy))
				return false;
		}
		else // source is a file
		{
			CString sNewFileName = sDestFolder + ff.GetFileName();
			if (!DoFileCopy(ff.GetFilePath(), sNewFileName, bDelteAfterCopy))
				return false;
		}
	}
	ff.Close();
	return true;
}
	
bool CFileProc::DoDelete(CString sPathName)
{
	CFileFind ff;
	CString sPath = sPathName;

	if (CheckPath(sPath) == PATH_IS_FILE)
	{
		if (!CanDelete(sPath)) 
		{
			m_bAborted = true;
			return false;
		}
		if (!DeleteFile(sPath)) 
			return false;
		return true;
	}

	PreparePath(sPath);
	sPath += "*.*";

	BOOL bRes = ff.FindFile(sPath);
	while(bRes)
	{
		bRes = ff.FindNextFile();
		if (ff.IsDots())
			continue;
		if (ff.IsDirectory())
		{
			sPath = ff.GetFilePath();
			DoDelete(sPath);
		}
		else DoDelete(ff.GetFilePath());
	}
	ff.Close();
	if (!RemoveDirectory(sPathName) && !m_bAborted) 
		return false;
	return true;
}
	

bool CFileProc::IsFileExist(CString sPathName)
{
	HANDLE hFile;
	hFile = CreateFile(sPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return false;
	CloseHandle(hFile);
	return true;
}
	

void CFileProc::PreparePath(CString &sPath)
{
	if(sPath.Right(1) != "\\") sPath += "\\";
}
	

void CFileProc::CheckSelfRecursion(CString sSource, CString sDest)
{
	if (sDest.Find(sSource) != -1)
	{
		int i = 0, count1 = 0, count2 = 0;
		for(i = 0; i < sSource.GetLength(); i ++)	if (sSource[i] == '\\') count1 ++;
		for(i = 0; i < sDest.GetLength(); i ++)	if (sDest[i] == '\\') count2 ++;
		if (count2 >= count1) m_iRecursionLimit = count2 - count1;
	}
}
	

bool CFileProc::CanDelete(CString sPathName)
{
	DWORD dwAttr = GetFileAttributes(sPathName);
	if (dwAttr == -1) return false;
	if (dwAttr & FILE_ATTRIBUTE_READONLY)
	{
		if (m_bAskIfReadOnly)
		{
			CString sTmp = sPathName;
			int pos = sTmp.ReverseFind('\\');
			if (pos != -1) sTmp.Delete(0, pos + 1);
			CString sText = sTmp + CString(" is read olny. Do you want delete it?");
			int iRes = MessageBox(NULL, sText, _T("Warning"), MB_YESNOCANCEL | MB_ICONQUESTION);
			switch (iRes)
			{
				case IDYES:
				{
					if (!SetFileAttributes(sPathName, FILE_ATTRIBUTE_NORMAL)) return false;
					return true;
				}
				case IDNO:
				{
					return false;
				}
				case IDCANCEL:
				{
					m_bAborted = true;
					return false;
				}
			}
		}
		else
		{
			if (!SetFileAttributes(sPathName, FILE_ATTRIBUTE_NORMAL)) return false;
			return true;
		}
	}
	return true;
}


CString CFileProc::ChangeFileName(CString sFileName)
{
	CString sName, sNewName, sResult;
	char drive[MAX_PATH];
	char dir  [MAX_PATH];
	char name [MAX_PATH];
	char ext  [MAX_PATH];
	_splitpath((LPCTSTR)sFileName, drive, dir, name, ext);
	sName = name;

	int pos = sName.Find("Copy ");
	if (pos == -1)
	{
		sNewName = CString("Copy of ") + sName + CString(ext);
	}
	else
	{
		int pos1 = sName.Find('(');
		if (pos1 == -1)
		{
			sNewName = sName;
			sNewName.Delete(0, 8);
			sNewName = CString("Copy (1) of ") + sNewName + CString(ext);
		}
		else
		{
			CString sCount;
			int pos2 = sName.Find(')');
			if (pos2 == -1)
			{
				sNewName = CString("Copy of ") + sNewName + CString(ext);
			}
			else
			{
				sCount = sName.Mid(pos1 + 1, pos2 - pos1 - 1);
				sName.Delete(0, pos2 + 5);
				int iCount = atoi((LPCTSTR)sCount);
				iCount ++;
				sNewName.Format("%s%d%s%s%s", "Copy (", iCount, ") of ", (LPCTSTR)sName, ext);
			}
		}
	}

	sResult = CString(drive) + CString(dir) + sNewName;

	return sResult;
}
	

CString CFileProc::ParseFolderName(CString sPathName)
{
	CString sFolderName = sPathName;
	int pos = sFolderName.ReverseFind('\\');
	if (pos != -1) sFolderName.Delete(pos, sFolderName.GetLength() - pos);
	pos = sFolderName.ReverseFind('\\');
	if (pos != -1) sFolderName = sFolderName.Right(sFolderName.GetLength() - pos - 1);
	else sFolderName.Empty();
	return sFolderName;
}
	

bool CFileProc::Delete(CString sPathName)
{
	return DoDelete(sPathName);
}
