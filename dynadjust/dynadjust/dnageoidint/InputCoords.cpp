// InputCoords.cpp : implementation file
//

#include "precompile.h"
#include <math.h>
#include <include/functions/dnatemplatefuncs.hpp>
#include "InputCoords.h"
#include "dnageoidint.h"

// CInputCoords dialog

IMPLEMENT_DYNAMIC(CInputCoords, CDialog)

CInputCoords::CInputCoords(CWnd* pParent /*=NULL*/)
	: CDialog(CInputCoords::IDD, pParent)
	, m_strLatitude(_T("-12.3056"))
	, m_strLongitude(_T("130.5031"))
	, m_strHeight(_T("0"))
	, m_lfLatitude(-12.3056)
	, m_lfLongitude(130.5031)
	, m_lfHeight(0.)
	, m_iDMS(DMS)
	, m_iEllipsoidtoOrthoRadio(0)
	, m_bInterpolateSuccess(false)
{

}

//agCoord.cVar.dNum1 = -12.248869;	// S 12° 14' 55.92960"
//agCoord.cVar.dNum2 = 142.060050;	// E 142° 03' 36.18000"
	
CInputCoords::~CInputCoords()
{
}

void CInputCoords::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	int ht = 0;
	DDX_Text(pDX, IDC_LAT, m_strLatitude);
	DDX_Text(pDX, IDC_LONG, m_strLongitude);
	DDX_Text(pDX, IDC_HT1, m_strHeight);
	DDX_Radio(pDX, IDC_DMMSS, m_iDMS);
	DDX_Radio(pDX, IDC_ELLIPS_ORTHO2, ht);
}


BEGIN_MESSAGE_MAP(CInputCoords, CDialog)
	ON_EN_KILLFOCUS(IDC_LAT, &CInputCoords::OnEnKillfocusLat)
	ON_EN_KILLFOCUS(IDC_LONG, &CInputCoords::OnEnKillfocusLong)
	ON_EN_SETFOCUS(IDC_LAT, &CInputCoords::OnEnSetfocusLat)
	ON_EN_SETFOCUS(IDC_LONG, &CInputCoords::OnEnSetfocusLong)
	ON_BN_CLICKED(IDC_INTERPOLATE, &CInputCoords::OnBnClickedInterpolate)
	ON_BN_CLICKED(IDC_DMMSS, &CInputCoords::OnBnClickedDmmss)
	ON_BN_CLICKED(IDC_DDDDD, &CInputCoords::OnBnClickedDdddd)
	ON_BN_CLICKED(IDC_ELLIPS_ORTHO2, &CInputCoords::OnBnClickedEllipsOrtho2)
	ON_BN_CLICKED(IDC_ORTHO_ELLIPS2, &CInputCoords::OnBnClickedOrthoEllips2)
	ON_MESSAGE(IDM_UPDATE_INTERPOLATION_RESULTS, &CInputCoords::OnUpdateInterpolationResults)	
END_MESSAGE_MAP()


// CInputCoords message handlers

BOOL CInputCoords::OnInitDialog()
{
	CDialog::OnInitDialog();

	OnEnKillfocusLong();
	GotoDlgCtrl(GetDlgItem(IDC_LAT));

	GetDlgItem(IDC_HT1_LABEL)->SetWindowText("Height (h):");
	GetDlgItem(IDC_HT2_LABEL)->SetWindowText("Height (H):");

	CString title;
	title.LoadString(IDS_TT_INTERPOLATE_POINT_TITLE);
	SetWindowText(title);

	return TRUE;
}
	

void CInputCoords::OnEnKillfocusLat()
{
	GetDlgItem(IDC_LAT)->GetWindowText(m_strLatitude);

	m_strLatitude.Trim();
	if (m_strLatitude.IsEmpty())
		m_strLatitude = "0.";
	
	m_lfLatitude = atof(m_strLatitude);		
	GetDlgItem(IDC_LAT)->SetWindowText(AddSymbology(m_strLatitude, true, 1));
}
	

void CInputCoords::OnEnSetfocusLat()
{
	GetDlgItem(IDC_LAT)->SetWindowText(m_strLatitude);
	((CEdit*)GetDlgItem(IDC_LAT))->SetSel(0, -1, FALSE);
}
	

void CInputCoords::OnEnKillfocusLong()
{
	GetDlgItem(IDC_LONG)->GetWindowText(m_strLongitude);

	m_strLongitude.Trim();
	if (m_strLongitude.IsEmpty())
		m_strLongitude = "0.";

	m_lfLongitude = atof(m_strLongitude);	
	GetDlgItem(IDC_LONG)->SetWindowText(AddSymbology(m_strLongitude, true, 2));
}
	

void CInputCoords::OnEnSetfocusLong()
{
	GetDlgItem(IDC_LONG)->SetWindowText(m_strLongitude);
	((CEdit*)GetDlgItem(IDC_LONG))->SetSel(0, -1, FALSE);
}
	

void CInputCoords::OnBnClickedInterpolate()
{
	if (GetFocus()->GetDlgCtrlID() == IDC_LAT)
		GetDlgItem(IDC_LAT)->GetWindowText(m_strLatitude);
	if (GetFocus()->GetDlgCtrlID() == IDC_LONG)
		GetDlgItem(IDC_LONG)->GetWindowText(m_strLongitude);

	m_bInterpolateSuccess = false;
	m_lfLatitude = atof(m_strLatitude);	
	m_lfLongitude = atof(m_strLongitude);

	GetDlgItem(IDC_HT1)->GetWindowText(m_strHeight);
	m_lfHeight = atof(m_strHeight);

	AfxGetMainWnd()->SendMessage(IDM_INTERACTIVE_INTERPOLATE, 0, 0);

	OnEnSetfocusLat();
	GotoDlgCtrl(GetDlgItem(IDC_LAT));
}
	

CString CInputCoords::AddSymbology(CString strNumber, bool bLatisPosSouth, int iFlag)
{
	int precision, decimal;
	CString strBuf;

	strBuf = strNumber;

	decimal = strNumber.Find('.');

	//iFlag: (1) Lat/East Input, (2) Long/North Input, (3) Lat/East Output, (4) Long/North Output
	precision = strBuf.GetLength() - decimal - 1;
	
	if (m_iDMS == DMS)
	{
		// Add symbols for degrees minutes and seconds
		if (decimal > -1)
		{
			// found a decimal point!
			if (decimal == 0)				// decimal point at start, ie >>   .0123
			{
				strBuf.Insert(decimal, '0');
				decimal++;
			}

			strBuf.SetAt(decimal, ' '); 
			strBuf.Insert(decimal, '°');
						
			// add zero after "tens" minute or "tens" second value
			if ((iFlag == 1 || iFlag == 2) && (precision == 1 || precision == 3) )
			{
				strBuf += "0";
				precision++;
			}
										
			if (precision > 1)						// add minute symbol
				strBuf.Insert((decimal+4), "\' ");
										
			if (precision > 3)
				strBuf.Insert((decimal+8), '"');
				
			if (precision > 4)
			{
				strBuf.SetAt((decimal+8), '.');
				strBuf += "\"";
			}
		
		}	
		else
			strBuf += "°";		// couldn't find a decimal point
	}
	else
	{
		// must be decimal degrees format...simply truncate value at required precision
		if (decimal == 0)
		{
			strBuf.Insert(decimal, '0');
			decimal++;
		}
		
		strBuf = strBuf.Mid(0, decimal + precision + 1);
		strBuf += "°";
	}
	
	if (iFlag < 3 || iFlag > 2)
	{
		// Show North and South notation
		if (strNumber[0] == '-' || strNumber[0] == 's' || strNumber[0] == 'S' || 
			strNumber[0] == 'w' || strNumber[0] == 'W')
		{
			if (iFlag == 1 || iFlag == 3)	// input/output latitude
				strBuf.SetAt(0, 'S');
			else							// input/output longitude
				strBuf.SetAt(0, 'W');
		}
		else
		{
			if (iFlag == 1 || iFlag == 3)
				strBuf = "N" + strBuf;
			else
				strBuf = "E" + strBuf;
		}
		
		strBuf = strBuf.Mid(0, 1) + " " + strBuf.Mid(1);
	}

	return strBuf;
}



void CInputCoords::OnBnClickedDmmss()
{
	m_iDMS = DMS;
	OnEnSetfocusLat();
	OnEnKillfocusLat();
	OnEnSetfocusLong();
	OnEnKillfocusLong();
	GotoDlgCtrl(GetDlgItem(IDC_LAT));
}

void CInputCoords::OnBnClickedDdddd()
{
	m_iDMS = DDEG;
	OnEnSetfocusLat();
	OnEnKillfocusLat();
	OnEnSetfocusLong();
	OnEnKillfocusLong();
	GotoDlgCtrl(GetDlgItem(IDC_LAT));
}


LRESULT CInputCoords::OnUpdateInterpolationResults(WPARAM wParam, LPARAM lParam)
{
	char buf[50];

	sprintf(buf, "%.3f", m_lfNvalue);
	GetDlgItem(IDC_N_VALUE)->SetWindowText(buf);
	sprintf(buf, "%.3f", m_lfPmeridian);
	GetDlgItem(IDC_DEFL_PMERIDIAN)->SetWindowText(buf);
	sprintf(buf, "%.3f", m_lfPvertical);
	GetDlgItem(IDC_DEFL_PVERTICAL)->SetWindowText(buf);

	// convert to decimal degrees
	if (m_iDMS == DMS)
	{	
		m_lfLatitude = DmstoDeg(m_lfLatitude);
		m_lfLongitude = DmstoDeg(m_lfLongitude);
	}
	
	// Astronomic longitude
	m_lfLongitude +=  m_lfPvertical / 3600. / cos(Radians(m_lfLatitude));

	// Astronomic latitude correction
	m_lfLatitude += m_lfPmeridian / 3600.;		

	if (m_iDMS == DMS)
	{
		m_lfLatitude = DegtoDms(m_lfLatitude);
		m_lfLongitude = DegtoDms(m_lfLongitude);
	}
	
	sprintf(buf, "%.9f", m_lfLatitude);
	GetDlgItem(IDC_LAT_OUT)->SetWindowText(AddSymbology(buf, true, 1));

	sprintf(buf, "%.9f", m_lfLongitude);
	GetDlgItem(IDC_LONG_OUT)->SetWindowText(AddSymbology(buf, true, 2));
	
	// Height
	if (m_iEllipsoidtoOrthoRadio == 0)
		m_lfHeight2 = m_lfHeight - m_lfNvalue;			// H = h - N
	else
		m_lfHeight2 = m_lfHeight + m_lfNvalue;			// h = H + N
	
	sprintf(buf, "%.3f", m_lfHeight2);
	GetDlgItem(IDC_HT2)->SetWindowText(buf);

	m_bInterpolateSuccess = true;

	return 0;
}

void CInputCoords::OnBnClickedEllipsOrtho2()
{
	m_iEllipsoidtoOrthoRadio = 0;
	GetDlgItem(IDC_HT1_LABEL)->SetWindowText("Height (h):");
	GetDlgItem(IDC_HT2_LABEL)->SetWindowText("Height (H):");

	if (m_bInterpolateSuccess)
	{
		m_lfHeight2 = m_lfHeight - m_lfNvalue;
	
		char cbuf[16];
		sprintf(cbuf, "%.3f", m_lfHeight2);
		GetDlgItem(IDC_HT2)->SetWindowText(cbuf);

	}
}

void CInputCoords::OnBnClickedOrthoEllips2()
{
	m_iEllipsoidtoOrthoRadio = 1;
	GetDlgItem(IDC_HT1_LABEL)->SetWindowText("Height (H):");
	GetDlgItem(IDC_HT2_LABEL)->SetWindowText("Height (h):");

	if (m_bInterpolateSuccess)
	{
		m_lfHeight2 = m_lfHeight + m_lfNvalue;

		char cbuf[16];
		sprintf(cbuf, "%.3f", m_lfHeight2);
		GetDlgItem(IDC_HT2)->SetWindowText(cbuf);
	}
}
