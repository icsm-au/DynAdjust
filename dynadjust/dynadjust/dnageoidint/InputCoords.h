/////////////////////////////////////////////
// Roger Fraser

//#ifndef     __INPUT_COORDS_DIALOG__
//#define     __INPUT_COORDS_DIALOG__

#pragma once

//#define DMS  0
//#define DMIN 1
//#define DDEG 1

#include "Resource.h"

// CInputCoords dialog

class CInputCoords : public CDialog
{
	DECLARE_DYNAMIC(CInputCoords)

public:
	CInputCoords(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInputCoords();

	inline double GetLatitudeInput() const { return m_lfLatitude; }
	inline double GetLongitudeInput() const { return m_lfLongitude; }
	inline int GetDMS() const { return m_iDMS; }
	
	inline void SetNValue(const double& d) { m_lfNvalue = d; }
	inline void SetPMeridian(const double& d) { m_lfPmeridian = d; }
	inline void SetPVertical(const double& d) { m_lfPvertical = d; }
	
	inline void SetSuccess(bool b) { m_bInterpolateSuccess = b; }

// Dialog Data
	enum { IDD = IDD_INPUT_COORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	CString AddSymbology(CString strNumber, bool bLatisPosSouth, int iFlag);

	int m_iDMS;
	int m_iEllipsoidtoOrthoRadio;
	double m_lfLatitude;
	double m_lfLongitude;
	double m_lfHeight;
	double m_lfHeight2;
	double m_lfNvalue;
	double m_lfPmeridian;
	double m_lfPvertical;
	
	bool	m_bInterpolateSuccess;

	DECLARE_MESSAGE_MAP()
public:
	CString m_strHeight;
	CString m_strLatitude;
	CString m_strLongitude;
	afx_msg void OnEnKillfocusLat();
	afx_msg void OnEnSetfocusLat();
	afx_msg void OnEnKillfocusLong();
	afx_msg void OnEnSetfocusLong();
	afx_msg void OnBnClickedInterpolate();
	afx_msg void OnBnClickedDmmss();
	afx_msg void OnBnClickedDdddd();
	afx_msg LRESULT OnUpdateInterpolationResults(WPARAM wParam, LPARAM lParam = 0);
	afx_msg void OnBnClickedEllipsOrtho2();
	afx_msg void OnBnClickedOrthoEllips2();
};


//#endif