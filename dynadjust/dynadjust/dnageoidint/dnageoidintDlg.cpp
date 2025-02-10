
// dnageoidintDlg.cpp : implementation file
//

#include "precompile.h"
#include "InputCoords.h"
#include "FileProc.h"
#include "dnageoidint.h"
#include "dnageoidintDlg.h"
#include "afxdialogex.h"
#include "RegProc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CreatingGrid;

UINT CreateGridFile(LPVOID pParam)
{
	n_file_par grid;
	n_gridfileindex subgrid;

	CdnageoidintDlg* dlg = (CdnageoidintDlg*)pParam;

	strcpy(grid.filename, dlg->tUserSettings.GridFile);
	strcpy(grid.filetype, dlg->tUserSettings.GridType);

	try {
		dlg->m_dnaGeoid.CreateNTv2File(
		(char*)((LPCTSTR)dlg->m_strAusGeoidDatFile), &grid);
	}
	catch (const NetGeoidException& e) {
		dlg->PrintLogFileMessage(e.what());
		dlg->PrintLogFileMessage("\n\n");
		MessageBox(0, e.what(), dlg->m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_ICONEXCLAMATION);
	}

	CreatingGrid = FALSE;
	return 0;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString version;
	version.Format("Version %s, %s", __BINARY_VERSION__, __DATE__);
	((CEdit*)GetDlgItem(IDC_VERSION))->SetWindowText(version);

	CString title;
	title.LoadString(IDS_TT_ABOUT_TITLE);
	SetWindowText(title);

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// CdnageoidintDlg dialog




CdnageoidintDlg::CdnageoidintDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdnageoidintDlg::IDD, pParent)
	, m_fLogFile(NULL)
	, m_bLogFileOpened(false)
	//, m_strDataFile("d:\\my_documents\\geodesy\\ausgeoid\\winter32\\input")
	, m_intDirRecurse(FALSE)
	, m_intDirCheckBox(FALSE)
	, m_intGridOpened(FALSE)
	, m_intDataFileOpened(FALSE)
	, m_intFolderOpened(FALSE)
	, m_intEllipsoidtoOrthoRadio(0)
	, m_intDmsFlagRadio(DMS)		// dd.mmssss
{
	tUserSettings.GridFile = "";
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_InputCoordDlg = NULL;
}

CdnageoidintDlg::~CdnageoidintDlg()
{
	CloseLogFile();
	if (m_InputCoordDlg != NULL)
		delete m_InputCoordDlg;
	//DeleteLogFile();
}

void CdnageoidintDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_GRIDFILE, tUserSettings.GridFile);
	DDX_Text(pDX, IDC_INPUT_FILE, m_strDataFile);
	DDX_Radio(pDX, IDC_ELLIPS_ORTHO, m_intEllipsoidtoOrthoRadio);
	DDX_Radio(pDX, IDC_DDMMSS, m_intDmsFlagRadio);
}

BEGIN_MESSAGE_MAP(CdnageoidintDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSE, OnOpenGridFile)
	ON_BN_CLICKED(IDC_OPEN_DATA, OnOpenInputData)
	ON_BN_CLICKED(IDC_ELLIPS_ORTHO, OnEllipsOrtho)
	ON_BN_CLICKED(IDC_ORTHO_ELLIPS, OnOrthoEllips)
	ON_BN_CLICKED(IDC_DDMMSS, OnFileDdmmss)
	ON_BN_CLICKED(IDC_DDDDDD, OnFileDdmmss)
	ON_BN_CLICKED(IDC_TRANSFORM, OnTransform)
	ON_BN_CLICKED(IDC_VIEW_LOG, &CdnageoidintDlg::OnBnClickedViewLog)
	ON_BN_CLICKED(IDC_INTERPOLATE_POINT, &CdnageoidintDlg::OnBnClickedInterpolatePoint)
	ON_COMMAND(ID_FILE_CREATEAUSGEOID, OnFileCreateausgeoid)
	ON_COMMAND(ID_FILE_INTERPOLATE, OnFileInterpolate)
	ON_COMMAND(ID_FILE_SELECTAUSGEOID, &CdnageoidintDlg::OnFileSelectausgeoid)
	ON_COMMAND(ID_ABOUT_ABOUT, OnAbout)
	ON_MESSAGE(IDM_INTERACTIVE_INTERPOLATE, OnInterpolateInputCoords)
END_MESSAGE_MAP()

const UINT lpIDArray[] =
{
	IDS_TT_IDLEMESSAGE,
	IDS_TT_PROGRESS
};


void CdnageoidintDlg::OnAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


// CdnageoidintDlg message handlers

BOOL CdnageoidintDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	OpenLogFile();

	// Create status bar at the bottom of the dialog window
	if (m_statusBar.Create(this))
	{
		m_statusBar.SetIndicators(lpIDArray, sizeof(lpIDArray)/sizeof(UINT));

		// Make a sunken or recessed border around the first pane
		m_statusBar.SetPaneInfo(0, IDS_TT_IDLEMESSAGE, SBPS_STRETCH, NULL );
		m_statusBar.SetPaneInfo(1, IDS_TT_PROGRESS, SBPS_NOBORDERS, 85);
		m_statusBar.SetPaneStyle(0, SBPS_NOBORDERS | SBPS_STRETCH);
		
		// We need to resize the dialog to make room for control bars.
		// First, figure out how big the control bars are.
		CRect rcClientStart;
		CRect rcClientNow;
		GetClientRect(rcClientStart);
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
					   0, reposQuery, rcClientNow);

		// Now move all the controls so they are in the same relative
		// position within the remaining client area as they would be
		// with no control bars.
		CPoint ptOffset(rcClientNow.left - rcClientStart.left,
						rcClientNow.top - rcClientStart.top);

		CRect  rcChild;
		CWnd* pwndChild = GetWindow(GW_CHILD);
		while (pwndChild)
		{
			pwndChild->GetWindowRect(rcChild);
			ScreenToClient(rcChild);
			rcChild.OffsetRect(ptOffset);
			pwndChild->MoveWindow(rcChild, FALSE);
			pwndChild = pwndChild->GetNextWindow();
		}

		// Adjust the dialog window dimensions
		CRect rcWindow;
		GetWindowRect(rcWindow);
		rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
		rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
		MoveWindow(rcWindow, FALSE);

		// And position the control bars
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

		// Finally, center the dialog on the screen
		CenterWindow();
	}
	else
	{
		TRACE0("Failed to create statusbar.\n");
	}

	long l;

	CFileProc fp;
	if (tUserSettings.GridFile.CompareNoCase(noGridSpecified) == 0)
		m_intGridOpened = FALSE;
	else if (fp.Exists(tUserSettings.GridFile, &l))
		CreateGridIndex();
	else
		m_intGridOpened = FALSE;

	OnUpdateControls();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CdnageoidintDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CdnageoidintDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CdnageoidintDlg::OnUpdateControls()
{
	if (m_intGridOpened)
	{
		GetMenu()->EnableMenuItem(ID_FILE_INTERPOLATE, MF_ENABLED);

		GetDlgItem(IDC_INTERPOLATE_POINT)->EnableWindow(TRUE);
		GetDlgItem(IDC_ST_OPENBOX)->EnableWindow(TRUE);
		GetDlgItem(IDC_INPUT_FILE)->EnableWindow(TRUE);
		GetDlgItem(IDC_OPEN_DATA)->EnableWindow(TRUE);
		//GetDlgItem(IDC_CHECK_DIR)->EnableWindow(TRUE);
		
		m_intDirRecurse = FALSE;

		// Update status bar
		if(m_intDirCheckBox)
			OnSetMessageString(IDS_TT_DIRMESSAGE);
		else
			OnSetMessageString(IDS_TT_FILEMESSAGE);	
		
		if (m_intDataFileOpened || m_intFolderOpened)
		{
			// Update status bar
			OnSetMessageString(IDS_TT_COMPUTEMESSAGE);
			//GetDlgItem(IDC_ST_HTBOX)->EnableWindow(TRUE);
		}

		GetDlgItem(IDC_ELLIPS_ORTHO)->EnableWindow(TRUE);
		GetDlgItem(IDC_ORTHO_ELLIPS)->EnableWindow(TRUE);
		GetDlgItem(IDC_TRANSFORM)->EnableWindow(TRUE);
		GetDlgItem(IDC_VIEW_LOG)->EnableWindow(TRUE);

		GetDlgItem(IDC_DDMMSS)->EnableWindow(TRUE);
		GetDlgItem(IDC_DDDDDD)->EnableWindow(TRUE);
	}
	else
	{
		GetMenu()->EnableMenuItem(ID_FILE_INTERPOLATE, MF_GRAYED);

		GetDlgItem(IDC_INTERPOLATE_POINT)->EnableWindow(FALSE);
		GetDlgItem(IDC_ST_OPENBOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_INPUT_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPEN_DATA)->EnableWindow(FALSE);
		//GetDlgItem(IDC_CHECK_DIR)->EnableWindow(FALSE);
		//GetDlgItem(IDC_CHECK_RECURSE)->EnableWindow(FALSE);
		//GetDlgItem(IDC_ST_HTBOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_ELLIPS_ORTHO)->EnableWindow(FALSE);
		GetDlgItem(IDC_ORTHO_ELLIPS)->EnableWindow(FALSE);
		GetDlgItem(IDC_TRANSFORM)->EnableWindow(FALSE);
		GetDlgItem(IDC_VIEW_LOG)->EnableWindow(FALSE);
	
		GetDlgItem(IDC_DDMMSS)->EnableWindow(FALSE);
		GetDlgItem(IDC_DDDDDD)->EnableWindow(FALSE);	
	}
	
	UpdateData(FALSE);
}

void CdnageoidintDlg::OnOpenGridFile()
{
	CFileDialog cfOpenFileDlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GRIDFILE_FILTER);

	if (cfOpenFileDlg.DoModal () == IDOK)
	{
		tUserSettings.GridFile = cfOpenFileDlg.GetPathName();
		tUserSettings.GridType = CFileProc::GetExt(tUserSettings.GridFile);
		CreateGridIndex();
	}

	if (tUserSettings.GridFile.IsEmpty())
	{
		m_intGridOpened = FALSE;
		m_intDataFileOpened = FALSE;
	}

	OnUpdateControls();
}


void CdnageoidintDlg::CreateGridIndex()
{
	// open grid file
	try {
		m_dnaGeoid.CreateGridIndex((char*)((LPCTSTR)tUserSettings.GridFile), (char*)((LPCTSTR)tUserSettings.GridType));

		char version[21];
		m_dnaGeoid.ReportGridVersion(version);
		std::stringstream ss;
		ss << "Opened Grid file  " << std::left << tUserSettings.GridFile << ", Version " << version << std::endl << std::endl;
		PrintLogFileMessage(ss.str().c_str());	
	}
	catch (const NetGeoidException& e) {
		PrintLogFileMessage(e.what());
		PrintLogFileMessage("\n\n");
		MessageBox(e.what(), m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_OK | MB_ICONEXCLAMATION);
		m_intGridOpened = FALSE;
		return;
	}
	m_intGridOpened = TRUE;

}

/////////////////////////////////////////////////////////////////////////
// DetermineFileType: Determines the distortion grid's file type from a char array.
//
/////////////////////////////////////////////////////////////////////////
// On Entry:   The grid file type is passed by cType.
// On Exit:    The grid file type is compared against "asc" or "gsb".
//			   An integer is returned indicating the filetype.
/////////////////////////////////////////////////////////////////////////
int CdnageoidintDlg::DetermineFileType(const char *cType)
{
	// case insensitive
	if (boost::iequals(cType, ASC))		// asc "ASCII" file
		return TYPE_ASC;			
	else if (boost::iequals(cType, GSB))	// gsb "Binary" file
		return TYPE_GSB;			
	else if (boost::iequals(cType, TXT) ||	// dat/txt/prn file
			 boost::iequals(cType, DAT) ||	// ..
			 boost::iequals(cType, PRN))	// ..
		return TYPE_DAT;
	else if (boost::iequals(cType, CSV))	// csv file
		return TYPE_CSV;
	else
		return -1;					// Unsupported filetype
}



void CdnageoidintDlg::OnOpenInputData()
{
	UpdateData(TRUE);

	int iExt = 0;
	char szExt[4] = "TXT";
	char szFile[MAX_FILEPATH] = "";

	if (!m_strDataFile.IsEmpty())
	{
		sprintf(szFile, "%s", CFileProc::GetName(m_strDataFile, NULL).GetString());	
		int i = DetermineFileType(CFileProc::GetExt(m_strDataFile));
		switch (i)
		{
		case TYPE_DAT:
			iExt = 1;
			sprintf(szExt, "TXT");
			break;
		case TYPE_CSV:
			iExt = 2;
			sprintf(szExt, "CSV");
			break;
		}
	}

	CString strFolder = CFileProc::GetFolder(m_strDataFile);

	// show File Open dialog
	CFileProc f;
	CString file = f.FileOpenSave(TRUE, FALSE, strFolder, szFile, SELECT_INPUT_FILE, szExt, INPUTDATA_FILTER, iExt);
	
	if (file.IsEmpty())	// user pressed cancel
		return;
	
	switch (m_intDirCheckBox)
	{
	case 0:			
		m_strDataFile = file;
		m_intDataFileOpened = TRUE;
		m_intFolderOpened = FALSE;
	
		OnUpdateControls();

		break;
	case 1:
		// show Browse for Folder dialog
		OnBrowseForFolder();		
		OnUpdateControls();
		break;
	default:
		TRACE0("Error retrieving checkbox status.");
		break;
	}	
}

void CdnageoidintDlg::OnBrowseForFolder()
{
	LPITEMIDLIST pidlSelected = NULL;
	BROWSEINFO bi = {0};
	LPMALLOC pMalloc = NULL;

	SHGetMalloc(&pMalloc);

	if (pMalloc != NULL)
	{
		ZeroMemory(&bi, sizeof(bi));

		bi.hwndOwner = NULL;
		bi.pidlRoot = 0;
		bi.pszDisplayName = 0;
		bi.lpszTitle = "Select a folder below, then click OK.";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;

		pidlSelected = SHBrowseForFolder(&bi);

		if(pidlSelected)
		{
			SHGetPathFromIDList(pidlSelected, m_selectedDirectory);
			m_strDataFile = m_selectedDirectory;
			m_intFolderOpened = TRUE;
			m_intDataFileOpened = FALSE;
		}

		pMalloc->Free(pidlSelected);
		pMalloc->Release();
	}
}

void CdnageoidintDlg::OnClickDirectoryCheckBox()
{
	UpdateData(TRUE);

	switch (m_intDirCheckBox)
	{
	case 0:
		GetDlgItem(IDC_CHECK_RECURSE)->EnableWindow(FALSE);
		if (!m_intDataFileOpened && !m_intFolderOpened)
			OnSetMessageString(IDS_TT_FILEMESSAGE);

		if (m_intFolderOpened)
		{
			m_intFolderOpened = FALSE;
			m_strDataFile = "";
		}
		break;
	case 1:
		GetDlgItem(IDC_CHECK_RECURSE)->EnableWindow(TRUE);
		if (!m_intDataFileOpened && !m_intFolderOpened)
		{
			if (m_intDirRecurse)
				OnSetMessageString(IDS_TT_RECURSEMESSAGE);
			else
				OnSetMessageString(IDS_TT_DIRMESSAGE);
		}
		
		if (m_intDataFileOpened)
		{
			m_intDataFileOpened = FALSE;
			m_strDataFile = "";
		}
		break;
	default:
		TRACE0("Error retrieving checkbox status.");
		break;
	}

	UpdateData(FALSE);
}

void CdnageoidintDlg::OnClickRecurseDirectory()
{
	UpdateData(TRUE);

	if (!m_intDataFileOpened && !m_intFolderOpened)
	{
		if (m_intDirRecurse)
			OnSetMessageString(IDS_TT_RECURSEMESSAGE);
		else
			OnSetMessageString(IDS_TT_DIRMESSAGE);
	}

	// TRACE1("Directory checkbox status: %d\n", m_intDirRecurse);
}

void CdnageoidintDlg::OnEllipsOrtho() 
{
	UpdateData(TRUE);
}
	

void CdnageoidintDlg::OnOrthoEllips() 
{
	UpdateData(TRUE);	
}
	

void CdnageoidintDlg::OnFileDdmmss()
{
	UpdateData(TRUE);
}
	

void CdnageoidintDlg::OnTransform() 
{
	char chDirectory[MAX_PATH], *ext;
	char chFilename[MAX_PATH];
	char chFilenameOut[MAX_PATH];
	int i;
	CString msg;

	if (!m_intDataFileOpened && !m_intFolderOpened)
		return;

	UpdateData(TRUE);

	CFileProc fileProc;
	
	// open grid file
	try {
		m_dnaGeoid.CreateGridIndex((char*)((LPCTSTR)tUserSettings.GridFile), (char*)((LPCTSTR)tUserSettings.GridType));
	}
	catch (const NetGeoidException& e) {
		PrintLogFileMessage(e.what());
		PrintLogFileMessage("\n\n");
		MessageBox(e.what(), m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	tUserSettings.GridFileStatus = 0;		
	
	strcpy(chDirectory, m_strDataFile);

	std::vector<std::string> file_type_array;
	file_type_array.push_back("dat");
	file_type_array.push_back("csv");
	
	// create the list of files to process
	if (m_intFolderOpened)
		fileProc.BuildFileArray(chDirectory, file_type_array, (int)file_type_array.size());		// working with a directory
	else
	{
		// working with a single file
		i = m_strDataFile.ReverseFind('\\');

		if (i != -1)
		{
			strcpy(chFilename, m_strDataFile.Mid(i+1));
			strcpy(chDirectory, m_strDataFile.Mid(0, i));
		}
		else
			return;			// invalid file
		
		fileProc.EnterSingleFile(chFilename);	
	}

	if (m_intEllipsoidtoOrthoRadio == 0)
		PrintLogFileMessage("Converting ellipsoidal heights to orthometric\n\n");
	else
		PrintLogFileMessage("Converting orthometric heights to ellipsoidal\n\n");
	
	char message[200];
	sprintf(message, "%d file(s) to be processed\n\n", fileProc.m_intNumFiles);
	PrintLogFileMessage(message);

	// no file to transform
	if (fileProc.m_fIndex == NULL)
		MessageBox("No *.dat or *.csv files were found in the specified directory.\n", "Warning", MB_OK | MB_ICONWARNING);
	else
	{
		// display the hourglass cursor
		BeginWaitCursor();
		PrintLogFileMessage("Transforming files...\n\n");

		for (i=0; i<fileProc.m_intNumFiles; i++)
		{
			msg = "Transforming file ";
			msg += fileProc.m_fIndex[i].chfilename;

			strcpy(chFilename, chDirectory);
			strcat(chFilename, "\\");
			strcat(chFilename, fileProc.m_fIndex[i].chfilename);
			m_statusBar.SetPaneText(0, msg, TRUE);

			sprintf(message, "%s\n", chFilename);
			PrintLogFileMessage(message);

			strcpy(chFilenameOut, chFilename);
			if ((ext = strrchr(chFilenameOut, '.')) == NULL)
				sprintf(chFilenameOut, "%s_out", chFilename);
			else
			{
				char extension[4];
				strcpy(extension, ext+1);
				chFilenameOut[ext-chFilenameOut] = '\0';
				sprintf(chFilenameOut, "%s_out.%s", chFilenameOut, extension);
			}
			
			try {
				m_dnaGeoid.FileTransformation(chFilename, chFilenameOut, BICUBIC, !m_intEllipsoidtoOrthoRadio, m_intDmsFlagRadio, false);
			}
			catch (const NetGeoidException& e) {
				PrintLogFileMessage(e.what());
				PrintLogFileMessage("\n\n");
				MessageBox(e.what(), m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_OK | MB_ICONEXCLAMATION);
				return;
			}			
		}

		PrintLogFileMessage("\nDone!\n\n");

		// restore the default (arrow) cursor
		EndWaitCursor();

		OnSetMessageString(IDS_TT_FILEMESSAGE);
		
		MessageBox("All files were successfully transformed.\n\nView the log file for details.\n", "Transformation complete",
			MB_OK | MB_ICONINFORMATION);
	}	
}


LRESULT CdnageoidintDlg::OnSetMessageString(WPARAM wParam, LPARAM lParam, int iPaneIndex)
{
	UINT    nIDMsg = (UINT)wParam;
	CString strMsg;

	if (nIDMsg)
	{
		if (strMsg.LoadString(nIDMsg) != 0)
			m_statusBar.SetPaneText(iPaneIndex, strMsg, TRUE);
		else
			TRACE1("Warning: no message line prompt for ID %x%04X\n", nIDMsg);
	}

	return nIDMsg;
}

void CdnageoidintDlg::OnFileCreateausgeoid()
{
	CFileDialog cfOpenFileDlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, AUSGEOIDDATA_FILTER);

	// show File Open dialog
	if (cfOpenFileDlg.DoModal () != IDOK)
		return;

	// Back up old settings
	T_user	currentSettings;
	currentSettings = tUserSettings;

	BeginWaitCursor();
	m_strAusGeoidDatFile = cfOpenFileDlg.GetPathName();
	tUserSettings.GridFile = m_strAusGeoidDatFile + ".gsb";
	tUserSettings.GridType = "gsb";

	CString strbuf;

	strbuf.Format("Creating NTv2 grid file from %s... ", m_strAusGeoidDatFile);
	PrintLogFileMessage(strbuf);

	// set up progress bar
	RECT rc;
	BOOL FileProgCreated;
	m_statusBar.GetItemRect(1, &rc);
	CProgressCtrl *FileProgress = new CProgressCtrl;
	if (FileProgress->Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, rc, &m_statusBar, 0))
	{
		FileProgCreated = TRUE;
		FileProgress->SetRange(0, 100);
	}

	m_statusBar.SetPaneStyle(1, SBPS_NORMAL);
	CreatingGrid = TRUE;

	AfxBeginThread(CreateGridFile, (void*)this, THREAD_PRIORITY_NORMAL);
	
	if (FileProgCreated)
	{
		int file_progress;		
		CFileProc fp;
		while (CreatingGrid)
		{
			Sleep(100);
			file_progress = m_dnaGeoid.ReturnFileProgress();
			FileProgress->SetPos(file_progress);
			if (file_progress < 0.0)
				strbuf.Format("Checking %s...", fp.GetName(m_strAusGeoidDatFile, 0));
			else
				strbuf.Format("Creating %s...", fp.GetName(tUserSettings.GridFile, 0));
			m_statusBar.SetPaneText(0, strbuf, TRUE);
			UpdateWindow();
		}
		delete FileProgress;
	}

	m_statusBar.SetPaneText(0, "Done.", TRUE);
	EndWaitCursor();

	PrintLogFileMessage("Done\n\n");
	strbuf.Format("Successfully created %s!\n\n", tUserSettings.GridFile);
	PrintLogFileMessage(strbuf);

	// Restore old settings
	tUserSettings = currentSettings;
}

// File menu | interpolate point
void CdnageoidintDlg::OnFileInterpolate()
{	
	if (tUserSettings.GridFile.CompareNoCase(noGridSpecified) == 0)
	{
		MessageBox("A grid file has not been selected.\n\nClick Browse to select a NTv2 geoid grid file.", "No grid file", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (m_InputCoordDlg == NULL)
	{
		m_InputCoordDlg = new CInputCoords();
		if(!m_InputCoordDlg->Create(IDD_INPUT_COORD, this))   //Create failed.
			return;
	}	
	m_InputCoordDlg->ShowWindow(SW_SHOW);

	if (m_intGridOpened == FALSE && agCoord_.cVar.IO_Status != 0)
	{
		try {
			m_dnaGeoid.CreateGridIndex((char*)((LPCTSTR)tUserSettings.GridFile), (char*)((LPCTSTR)tUserSettings.GridType));
			m_InputCoordDlg->SetSuccess(true);
		}
		catch (const NetGeoidException& e) {
			tUserSettings.GridFileStatus = agCoord_.cVar.IO_Status;
			PrintLogFileMessage(e.what());
			PrintLogFileMessage("\n\n");
			MessageBox(e.what(), m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_OK | MB_ICONEXCLAMATION);
			m_InputCoordDlg->SetSuccess(false);
			return;
		}
	}
}


	

LRESULT CdnageoidintDlg::OnInterpolateInputCoords(WPARAM wParam, LPARAM lParam)
{
	if (m_intGridOpened == FALSE)
	{
		MessageBox("A grid file has not been selected.\n\nClick Browse to select a NTv2 geoid grid file.", "No grid file", MB_OK | MB_ICONEXCLAMATION);
		return 0;	
	}

	if (m_InputCoordDlg == NULL)
		return 0;

	char message[200];

	switch (m_InputCoordDlg->GetDMS())
	{
	case DMS:
		agCoord_.cVar.dLatitude = DmstoDeg(m_InputCoordDlg->GetLatitudeInput());
		agCoord_.cVar.dLongitude = DmstoDeg(m_InputCoordDlg->GetLongitudeInput());
		break;
	//case DMIN:
	//	agCoord_.cVar.dLatitude = DmintoDeg(m_InputCoordDlg->GetLatitudeInput());
	//	agCoord_.cVar.dLongitude = DmintoDeg(m_InputCoordDlg->GetLongitudeInput());
	//	break;
	case DDEG:
		agCoord_.cVar.dLatitude = m_InputCoordDlg->GetLatitudeInput();	// S 12� 14' 55.92960"
		agCoord_.cVar.dLongitude = m_InputCoordDlg->GetLongitudeInput();	// E 142� 03' 36.18000"	
		break;
	}

	try {
		m_dnaGeoid.BiCubicTransformation(&agCoord_);
		m_InputCoordDlg->SetSuccess(true);
	}
	catch (const NetGeoidException& e) {
		PrintLogFileMessage(e.what());
		PrintLogFileMessage("\n\n");

		if (agCoord_.cVar.IO_Status == ERR_FINDSUBGRID_OUTSIDE || 
			agCoord_.cVar.IO_Status == ERR_PT_OUTSIDE_GRID)
		{
			// 13:	Point is outside limits of the grid
			std::stringstream ss;
			ss << ">>  " << std::fixed << std::setprecision(9) << agCoord_.cVar.dLatitude << ", " << std::fixed << std::setprecision(9) << agCoord_.cVar.dLongitude << std::endl << std::endl;
			PrintLogFileMessage(ss.str().c_str());
		}

		MessageBox(e.what(), m_dnaGeoid.ErrorCaption(e.error_no()).c_str(), MB_OK | MB_ICONEXCLAMATION);
		m_InputCoordDlg->SetSuccess(false);
		return 0;
	}

	m_InputCoordDlg->SetNValue(agCoord_.gVar.dN_value);
	m_InputCoordDlg->SetPMeridian(agCoord_.gVar.dDefl_meridian);
	m_InputCoordDlg->SetPVertical(agCoord_.gVar.dDefl_primev);

	m_InputCoordDlg->SendMessage(IDM_UPDATE_INTERPOLATION_RESULTS, 0, 0);

	sprintf(message, "Latitude:%12.7f, Longitude:%12.7f, N:%12.4f, dPm:%10.4f, dPv:%10f\n", 
		agCoord_.cVar.dLatitude, agCoord_.cVar.dLongitude,
		agCoord_.gVar.dN_value, agCoord_.gVar.dDefl_meridian, agCoord_.gVar.dDefl_primev);
	PrintLogFileMessage(message);

	return 0;
}
	

void CdnageoidintDlg::OnBnClickedViewLog()
{
	fclose(m_fLogFile);
	CFileProc fp;

	fp.Copy(tUserSettings.LogFile, fp.GetFolder(tUserSettings.LogFile));
	fp.View(fp.m_strDestinationFile, "notepad.exe");

	if (fopen_s(&m_fLogFile, tUserSettings.LogFile, "a") != 0)
		m_bLogFileOpened = false;
	else
		m_bLogFileOpened = true;
}

void CdnageoidintDlg::OpenLogFile(void)
{
	if (fopen_s(&m_fLogFile, tUserSettings.LogFile, "w") != 0)
	{
		m_bLogFileOpened = false;
		return;
	}
	else
		m_bLogFileOpened = true;


	PrintLogFileMessage("//////////////  AusGeoid-Interpolation Log File  //////////////\n\n");

	// get system date
	char cBuf[50];
	char message[250];
	CString strHeader;
	GetSystemDate(cBuf, DATE_LONG);
	sprintf(message, "Date:  %s\n\n\n", cBuf);
	PrintLogFileMessage(message);
}
	
/////////////////////////////////////////////////////////////////////////
// GetSystemDate: Retrieves the system date based on am local time.
/////////////////////////////////////////////////////////////////////////
// On Entry:  cBuf is an empty char array. format is the format specifier for
//			  the date style.
// On Exit:   The system date/time is printed to cBuf.
/////////////////////////////////////////////////////////////////////////
void CdnageoidintDlg::GetSystemDate(char *cBuf, const char *format)
{
	struct tm *today;
	time_t ltime;	
	time(&ltime);
	today = localtime(&ltime);
	strftime(cBuf, 100, format, today);
}
	

void CdnageoidintDlg::CloseLogFile(void)
{
	fclose(m_fLogFile);
	m_bLogFileOpened = false;
}
	

void CdnageoidintDlg::PrintLogFileMessage(const char* szMessage)
{
	if (!m_bLogFileOpened)
		return;
	fprintf_s(m_fLogFile, "%s", szMessage);
	fflush(m_fLogFile);
}

void CdnageoidintDlg::DeleteLogFile(void)
{
	CFileProc fileProc;
	std::vector<std::string> file_type_array;
	file_type_array.push_back("log");

	char chDirectory[MAX_PATH];
	char chFilename[MAX_PATH];

	strcpy(chDirectory, fileProc.GetFolder(tUserSettings.LogFile));
	fileProc.BuildFileArray(chDirectory, file_type_array, (int)file_type_array.size());

	for (int i=0; i<fileProc.m_intNumFiles; i++)
	{
		strcpy(chFilename, chDirectory);
		strcat(chFilename, "\\");
		strcat(chFilename, fileProc.m_fIndex[i].chfilename);
		fileProc.Delete(chFilename);	
	}
}

void CdnageoidintDlg::OnFileSelectausgeoid()
{
	OnOpenGridFile();
}

void CdnageoidintDlg::OnBnClickedInterpolatePoint()
{
	OnFileInterpolate();
}

