
// dnageoidintDlg.h : header file
//

#pragma once

#include "RegProc.h"
#include <dynadjust/dnageoid/dnageoid.hpp>

#define SELECT_INPUT_FILE "Select the input file..."
#define GRIDFILE_FILTER "NTv2 Binary Grid File (*.gsb)|*.gsb|NTv2 ASCII Grid File (*.asc)|*.asc||"
#define INPUTDATA_FILTER   "Formatted Text Files (*.dat,*.txt,*.prn)|*.dat;*.txt;*.prn;|CSV (Comma delimited) (*.csv)|*.csv|All Files(*.*)|*.*||"
#define AUSGEOIDDATA_FILTER "AusGeoid DAT Files (*.dat)|*.dat;|All Files(*.*)|*.*||"

const char* const DATE_LONG = " %A, %d %B %Y, %I:%M:%S %p.";			// Wednesday, 24 November 1999 11:19:35 AM.

using namespace dynadjust::geoidinterpolation;
using namespace dynadjust::exception;

// CdnageoidintDlg dialog
class CdnageoidintDlg : public CDialogEx
{
// Construction
public:
	int m_intDataFileOpened;
	int m_intFolderOpened;
	int m_intGridOpened;
	char m_selectedDirectory[MAX_PATH];
	void OnUpdateControls();
	void OnBrowseForFolder();
	int DetermineFileType(const char *cType);
	void PrintLogFileMessage(const char* szMessage);
	
	void OnAbout();

	T_user	tUserSettings;
	FILE*	m_fLogFile;
	bool	m_bLogFileOpened;

	// dnaGeoid DLL exported class
	dna_geoid_interpolation m_dnaGeoid;

	CdnageoidintDlg(CWnd* pParent = NULL);	// standard constructor
	~CdnageoidintDlg();

// Dialog Data
	enum { IDD = IDD_DNAGEOIDINT_DIALOG };
	CString	m_strDataFile;
	CString	m_strAusGeoidDatFile;
	BOOL	m_intDirRecurse;
	BOOL	m_intDirCheckBox;
	int		m_intEllipsoidtoOrthoRadio;
	int		m_intDmsFlagRadio;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	
	CStatusBar	m_statusBar;	// statusbar

	CInputCoords* m_InputCoordDlg;

	geoid_point agCoord_;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpenGridFile();
	afx_msg void OnOpenInputData();
	afx_msg void OnClickDirectoryCheckBox();
	afx_msg void OnClickRecurseDirectory();
	afx_msg void OnEllipsOrtho();
	afx_msg void OnOrthoEllips();
	afx_msg void OnFileDdmmss();
	afx_msg void OnTransform();
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam = 0L, int iPaneIndex = 0);
	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg void OnFileCreateausgeoid();
	afx_msg void OnFileInterpolate();
	afx_msg LRESULT OnInterpolateInputCoords(WPARAM wParam, LPARAM lParam = 0);
	afx_msg void OnBnClickedViewLog();
private:
	void OpenLogFile(void);
	void CloseLogFile(void);
	void GetSystemDate(char *cBuf, const char *format);
	void DeleteLogFile(void);
	void CreateGridIndex(void);

public:
	afx_msg void OnFileSelectausgeoid();
	afx_msg void OnBnClickedInterpolatePoint();
};
