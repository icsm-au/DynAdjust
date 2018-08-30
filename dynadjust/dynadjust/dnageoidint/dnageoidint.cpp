
// dnageoidint.cpp : Defines the class behaviors for the application.
//

#include "precompile.h"
#include "InputCoords.h"
#include "RegProc.h"
#include "dnageoidint.h"
#include "dnageoidintDlg.h"
#include <tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// http://www.codeproject.com/KB/MFC/checkapprunning.aspx
// Helper callback-function for function AppIsAllreadyRunning()
// see description of EnumWindows() for details
BOOL CALLBACK ShowAppEnum (HWND hwnd, LPARAM lParam)
{
	DWORD dwID;
	CString strAppName, strBuffer;
	char title[256];

	// Test main application name
	strAppName.Format("%s", __BINARY_NAME__);
	GetWindowThreadProcessId(hwnd, &dwID) ;
	if(dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd,title,256);
		strBuffer = title;
		if (strBuffer.Left(strAppName.GetLength()) == strAppName)
		{
			if (!IsWindowVisible(hwnd))
				ShowWindow(hwnd,SW_SHOW); 
			SetForegroundWindow(hwnd);
			return TRUE;
		}
	}

	// Test old about box (About AusGeoid-Interpolation)
	strAppName = "About AusGeoid-Interpolation";
	GetWindowThreadProcessId(hwnd, &dwID) ;
	if(dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd,title,256);
		strBuffer = title;
		if (strBuffer.Left(strAppName.GetLength()) == strAppName)
		{
			if (!IsWindowVisible(hwnd))
				ShowWindow(hwnd,SW_SHOW); 
			SetForegroundWindow(hwnd);
			return TRUE;
		}
	}

	// Test new about box (title held in IDS_TT_ABOUT_TITLE)
	strAppName.LoadString(IDS_TT_ABOUT_TITLE);
	GetWindowThreadProcessId(hwnd, &dwID) ;
	if(dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd,title,256);
		strBuffer = title;
		if (strBuffer.Left(strAppName.GetLength()) == strAppName)
		{
			if (!IsWindowVisible(hwnd))
				ShowWindow(hwnd,SW_SHOW); 
			SetForegroundWindow(hwnd);
			return TRUE;
		}
	}

	// Test about box
	strAppName.LoadString(IDS_TT_ABOUT_TITLE);
	GetWindowThreadProcessId(hwnd, &dwID) ;
	if(dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd,title,256);
		strBuffer = title;
		if (strBuffer.Left(strAppName.GetLength()) == strAppName)
		{
			if (!IsWindowVisible(hwnd))
				ShowWindow(hwnd,SW_SHOW); 
			SetForegroundWindow(hwnd);
			return TRUE;
		}
	}

	// Test Interpolate Point dialog
	strAppName.LoadString(IDS_TT_INTERPOLATE_POINT_TITLE);
	GetWindowThreadProcessId(hwnd, &dwID) ;
	if(dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd,title,256);
		strBuffer = title;
		if (strBuffer.Left(strAppName.GetLength()) == strAppName)
		{
			if (!IsWindowVisible(hwnd))
				ShowWindow(hwnd,SW_SHOW); 
			SetForegroundWindow(hwnd);
			return TRUE;
		}
	}

	return TRUE;
}

// http://www.codeproject.com/KB/MFC/checkapprunning.aspx
// Checks, if an application with this name is running
//
// bShow ..... TRUE: bring application to foreground, if running 
//             FALSE: only check, don't move to the application
//
// return: FALSE: application is not running
//         TRUE: application runs
BOOL AppIsAlreadyRunning(BOOL bShow=TRUE)
{
	BOOL bRunning=FALSE;
	CString strAppName;
	strAppName.Format("%s", __BINARY_NAME__);
	strAppName += _T(".exe");
	DWORD dwOwnPID = GetProcessId(GetCurrentProcess());
	HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32* processInfo=new PROCESSENTRY32;
	processInfo->dwSize=sizeof(PROCESSENTRY32);
	int index=0;
	while(Process32Next(hSnapShot,processInfo)!=FALSE)
	{
		if (!strcmp(processInfo->szExeFile,strAppName))
		{
			if (processInfo->th32ProcessID != dwOwnPID)
			{
				if (bShow)
					EnumWindows(ShowAppEnum, processInfo->th32ProcessID);
				bRunning=TRUE;
				break;
			}
		}
	}
	CloseHandle(hSnapShot);
	delete processInfo;
	return bRunning;
}

// CdnageoidintApp

BEGIN_MESSAGE_MAP(CdnageoidintApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CdnageoidintApp construction

CdnageoidintApp::CdnageoidintApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

}

CdnageoidintApp::~CdnageoidintApp()
{	
}

// The one and only CdnageoidintApp object

CdnageoidintApp theApp;


// CdnageoidintApp initialization

BOOL CdnageoidintApp::InitInstance()
{
	// Is GeoidInt already running?
	if (AppIsAlreadyRunning())
		return FALSE;


	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	
	CdnageoidintDlg dlg;
	m_pMainWnd = &dlg;
	
	CRegProc Registry;

	// Read grid file location from preferences file
	if (Registry.PreferencesFileExists())
	{
		Registry.GetTemporaryFileSettings(&dlg.tUserSettings);
	}
	else if (Registry.CreatePreferencesFile())
	{
		Registry.InitialiseSettings(&dlg.tUserSettings);
	}
	else
	{
		CString msg;
		msg.Format("Error: Unable to load user settings.\n\n");
		msg += "You may not have read/write access to\n" + Registry.GetPreferencesFolder() + ".";
		msg += "\n\nContact your local system administrator for assistance.";
		AfxMessageBox(msg);
	}
	
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// Place code here to handle when the dialog is
		// dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// Place code here to handle when the dialog is
		// dismissed with Cancel
	}
	
	Registry.SaveTemporaryFileSettings(&dlg.tUserSettings);

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

