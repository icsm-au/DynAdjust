/////////////////////////////////////////////
// Roger Fraser

#ifndef     __FILE_PROCEDURES__
#define     __FILE_PROCEDURES__

#include <afx.h>
#include <vector>

#define MAX_FILEPATH 2048

struct _fileindex {
	char chfilename[_MAX_PATH];		// the filename array	
};

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

using namespace std;

#define PATH_ERROR			-1
#define PATH_NOT_FOUND		0
#define PATH_IS_FILE		1
#define PATH_IS_FOLDER		2

#define CSV		"csv"
#define QIF		"qif"
#define MAN		"man"
#define RES		"res"

class CFileProc
{
// Construction
public:
	CFileProc();
	~CFileProc();
	
	bool Delete(CString sPathName); // delete file or folder
	bool Copy(CString sSource, CString sDest); // copy file or folder
	bool Replace(CString sSource, CString sDest); // move file or folder
	bool Rename(CString sSource, CString sDest); // rename file or folder
	int CheckPath(CString sPath);
	bool CanDelete(CString sPathName); // check attributes
		
	int FilterIndex(CString strExt);
	int Exists(CString strFilepath, long *hFile);
	int View(CString strFilepath, CString f_viewer);
	static CString GetExt(CString strFilepath);
	static CString GetFolder(CString strFilepath);
	static CString GetName(CString strFilepath, LPCTSTR lpszExt);
	CString BrowseForDialog(CString strMsg, HWND hwndOwner);

	void ClearFileArrayMemory();
	void BuildFileArray(const char* filedir, const vector<string>& filetype, const int& no_filetypes);
	void EnterSingleFile(char *filename);

	CString FileOpenSave(BOOL bOpen_Save, BOOL bMulti_Files, LPCTSTR lpszDir, char *cNewFile, LPCTSTR lpszTitle, LPCTSTR lpszExt, LPCTSTR strFilter, int iFileType);
	
	char m_strDirectory[MAX_PATH];
	
	int m_intNumFiles;				// number of files to be transformed
	long m_lTotalBytes;
	CString m_strFileExt;
	_fileindex *m_fIndex;

	CString m_strDestinationFile;
	
	
protected:
	bool DoDelete(CString sPathName);
	bool DoCopy(CString sSource, CString sDest, bool bDelteAfterCopy = false);
	bool DoFileCopy(CString sSourceFile, CString sDestFile, bool bDelteAfterCopy = false);
	bool DoFolderCopy(CString sSourceFolder, CString sDestFolder, bool bDelteAfterCopy = false);
	void DoRename(CString sSource, CString sDest);
	bool IsFileExist(CString sPathName);
	void PreparePath(CString &sPath);
	void Initialize();
	void CheckSelfRecursion(CString sSource, CString sDest);
	bool CheckSelfCopy(CString sSource, CString sDest);
	CString ChangeFileName(CString sFileName);
	CString ParseFolderName(CString sPathName);

private:
	CString m_sError;
	DWORD m_dwError;
	bool m_bAskIfReadOnly;
	bool m_bOverwriteMode;
	bool m_bAborted;
	int m_iRecursionLimit;
	
};

#endif