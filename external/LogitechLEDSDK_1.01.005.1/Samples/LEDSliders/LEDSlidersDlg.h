
// LEDSlidersDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CLEDSlidersDlg dialog
class CLEDSlidersDlg : public CDialogEx
{
    // Construction
public:
    CLEDSlidersDlg(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    enum { IDD = IDD_LEDSLIDERS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:

    int GetDeviceType();

    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    CScrollBar m_scrollBarKbdBlue;
    CScrollBar m_scrollBarKbdGreen;
    CScrollBar m_scrollBarKbdRed;
    CScrollBar m_scrollBarMouseBlue;
    CScrollBar m_scrollBarMouseGreen;
    CScrollBar m_scrollBarMouseRed;
    afx_msg void OnBnClickedCancel();
    CButton m_checkKeyboard;
    CButton m_checkMouse;
    afx_msg void OnClickedButtonSaveLighting();
    afx_msg void OnClickedButtonRestoreLighting();
};
