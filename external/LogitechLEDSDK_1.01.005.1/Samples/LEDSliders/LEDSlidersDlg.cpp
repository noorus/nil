
// LEDSlidersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LEDSliders.h"
#include "LEDSlidersDlg.h"
#include "afxdialogex.h"

//#define USE_LOGITECH_DLL_ONLY

#ifndef USE_LOGITECH_DLL_ONLY
#pragma comment(lib, "LogitechLed.lib")

#include "LogitechLed.h"
#else
// Device types for LogiLedSaveCurrentLighting, LogiLedSetLighting, LogiLedRestoreLighting
const int LOGITECH_LED_MOUSE = 0x0001;
const int LOGITECH_LED_KEYBOARD = 0x0002;
const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD;

typedef BOOL (* LPFNDLLINIT)();
typedef BOOL (* LPFNDLLSAVECURRENTLIGHTING)(int);
typedef BOOL (* LPFNDLLSETLIGHTING)(int, int, int, int);
typedef BOOL (* LPFNDLLRESTORELIGHTING)(int);
typedef void (* LPFNDLLSHUTDOWN)();

LPFNDLLINIT g_lpfnDllInit = NULL;
LPFNDLLSAVECURRENTLIGHTING g_lpfnDllSaveCurrentLighing = NULL;
LPFNDLLSETLIGHTING g_lpfnDllSetLighting = NULL;
LPFNDLLRESTORELIGHTING g_lpfnDllRestoreLighting = NULL;
LPFNDLLSHUTDOWN g_lpfnDllShutdown = NULL;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLEDSlidersDlg dialog



CLEDSlidersDlg::CLEDSlidersDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CLEDSlidersDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLEDSlidersDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCROLLBAR_MOUSE_BLUE, m_scrollBarMouseBlue);
    DDX_Control(pDX, IDC_SCROLLBAR_MOUSE_GREEN, m_scrollBarMouseGreen);
    DDX_Control(pDX, IDC_SCROLLBAR_MOUSE_RED, m_scrollBarMouseRed);
    DDX_Control(pDX, IDC_CHECK_KEYBOARD, m_checkKeyboard);
    DDX_Control(pDX, IDC_CHECK_MOUSE, m_checkMouse);
}

BEGIN_MESSAGE_MAP(CLEDSlidersDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDCANCEL, &CLEDSlidersDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_LIGHTING, &CLEDSlidersDlg::OnClickedButtonSaveLighting)
    ON_BN_CLICKED(IDC_BUTTON_RESTORE_LIGHTING, &CLEDSlidersDlg::OnClickedButtonRestoreLighting)
END_MESSAGE_MAP()


// CLEDSlidersDlg message handlers

BOOL CLEDSlidersDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here
    m_scrollBarMouseBlue.SetScrollRange(0, 100);
    m_scrollBarMouseBlue.SetScrollPos(0);
    m_scrollBarMouseGreen.SetScrollRange(0, 100);
    m_scrollBarMouseGreen.SetScrollPos(0);
    m_scrollBarMouseRed.SetScrollRange(0, 100);
    m_scrollBarMouseRed.SetScrollPos(0);

    m_checkKeyboard.SetCheck(1);
    m_checkMouse.SetCheck(1);

#ifndef USE_LOGITECH_DLL_ONLY
    LogiLedInit();
#else
    HINSTANCE logiDllHandle = LoadLibrary(L"LogitechLed.dll");
    if (logiDllHandle != NULL)
    {
        g_lpfnDllInit = (LPFNDLLINIT)GetProcAddress(logiDllHandle, "LogiLedInit");
        g_lpfnDllSaveCurrentLighing = (LPFNDLLSAVECURRENTLIGHTING)GetProcAddress(logiDllHandle, "LogiLedSaveCurrentLighting");
        g_lpfnDllSetLighting = (LPFNDLLSETLIGHTING)GetProcAddress(logiDllHandle, "LogiLedSetLighting");
        g_lpfnDllRestoreLighting = (LPFNDLLRESTORELIGHTING)GetProcAddress(logiDllHandle, "LogiLedRestoreLighting");
        g_lpfnDllShutdown = (LPFNDLLSHUTDOWN)GetProcAddress(logiDllHandle, "LogiLedShutdown");

        g_lpfnDllInit();
    }
#endif

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLEDSlidersDlg::OnPaint()
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
HCURSOR CLEDSlidersDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}



void CLEDSlidersDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    // TODO: Add your message handler code here and/or call default
    int CurPos = pScrollBar->GetScrollPos();

    // Determine the new position of scroll box.
    switch (nSBCode)
    {
    case SB_LEFT:      // Scroll to far left.
        CurPos = 0;
        break;

    case SB_RIGHT:      // Scroll to far right.
        CurPos = 122;
        break;

    case SB_ENDSCROLL:   // End scroll.
        break;

    case SB_LINELEFT:      // Scroll left.
        if (CurPos > 0)
            CurPos--;
        break;

    case SB_LINERIGHT:   // Scroll right.
        if (CurPos < 122)
            CurPos++;
        break;

    case SB_PAGELEFT:    // Scroll one page left.
        {
            // Get the page size. 
            SCROLLINFO   info;
            pScrollBar->GetScrollInfo(&info, SIF_ALL);

            if (CurPos > 0)
                CurPos = max(0, CurPos - (int) info.nPage);
        }
        break;

    case SB_PAGERIGHT:      // Scroll one page right
        {
            // Get the page size. 
            SCROLLINFO   info;
            pScrollBar->GetScrollInfo(&info, SIF_ALL);

            if (CurPos < 122)
                CurPos = min(122, CurPos + (int) info.nPage);
        }
        break;

    case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
        CurPos = nPos;      // of the scroll box at the end of the drag operation.
        break;

    case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
        CurPos = nPos;     // position that the scroll box has been dragged to.
        break;
    }

    pScrollBar->SetScrollPos(CurPos);

    int red = m_scrollBarMouseRed.GetScrollPos();
    int green = m_scrollBarMouseGreen.GetScrollPos();
    int blue = m_scrollBarMouseBlue.GetScrollPos();

    if (pScrollBar == &m_scrollBarKbdRed)
    {
        red = CurPos;
    }
    else if (pScrollBar == &m_scrollBarKbdGreen)
    {
        green = CurPos;
    }
    else if (pScrollBar == &m_scrollBarKbdBlue)
    {
        blue = CurPos;
    }

#ifndef USE_LOGITECH_DLL_ONLY
    LogiLedSetLighting(GetDeviceType(), red, green, blue);
#else
    g_lpfnDllSetLighting(GetDeviceType(), red, green, blue);
#endif

    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLEDSlidersDlg::OnBnClickedCancel()
{
#ifndef USE_LOGITECH_DLL_ONLY
    LogiLedShutdown();
#else
    g_lpfnDllShutdown();
#endif

    CDialogEx::OnCancel();
}

void CLEDSlidersDlg::OnClickedButtonSaveLighting()
{
#ifndef USE_LOGITECH_DLL_ONLY
    LogiLedSaveCurrentLighting(GetDeviceType());
#else
    g_lpfnDllSaveCurrentLighing(GetDeviceType());
#endif
}

void CLEDSlidersDlg::OnClickedButtonRestoreLighting()
{
#ifndef USE_LOGITECH_DLL_ONLY
    LogiLedRestoreLighting(GetDeviceType());
#else
    g_lpfnDllRestoreLighting(GetDeviceType());
#endif
}

int CLEDSlidersDlg::GetDeviceType()
{
    int deviceType = 0;
    if (m_checkKeyboard.GetCheck())
        deviceType = deviceType | LOGITECH_LED_KEYBOARD;
    if (m_checkMouse.GetCheck())
        deviceType = deviceType | LOGITECH_LED_MOUSE;

    return deviceType;
}
