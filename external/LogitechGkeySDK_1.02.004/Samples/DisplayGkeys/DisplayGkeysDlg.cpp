
// DisplayGkeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DisplayGkeys.h"
#include "DisplayGkeysDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CALLBACKMSG (WM_USER + 1)
HWND g_hwnd = NULL;

//#define USE_LOGITECH_DLL_ONLY
#define USE_CALLBACK

#ifndef USE_LOGITECH_DLL_ONLY
#include "LogitechGkey.h"
#else
#define LOGITECH_MAX_MOUSE_BUTTONS 20
#define LOGITECH_MAX_GKEYS 29
#define LOGITECH_MAX_M_STATES 3

typedef struct
{
    unsigned int keyIdx         : 8;        // index of the G key or mouse button, for example, 6 for G6 or Button 6
    unsigned int keyDown        : 1;        // key up or down, 1 is down, 0 is up
    unsigned int mState         : 2;        // mState (1, 2 or 3 for M1, M2 and M3)
    unsigned int mouse          : 1;        // indicate if the Event comes from a mouse, 1 is yes, 0 is no.
    unsigned int reserved1      : 4;        // reserved1
    unsigned int reserved2      : 16;       // reserved2
} GkeyCode;

// Callback used to allow client to react to the Gkey events. It is called in the context of another thread.
typedef void (__cdecl *logiGkeyCB)(GkeyCode gkeyCode, const wchar_t* gkeyOrButtonString, void* context);

typedef struct
{
    logiGkeyCB    gkeyCallBack;
    void*       gkeyContext;
}logiGkeyCBContext;

typedef BOOL (* LPFNDLLGKEYINIT)(logiGkeyCBContext*);
typedef BOOL (* LPFNDLLGKEYINITWITHOUTCALLBACK)();
typedef BOOL (* LPFNDLLGKEYISMOUSEBUTTONPRESSED)(int);
typedef wchar_t* (* LPFNDLLGKEYGETMOUSEBUTTONSTRING)(int);
typedef BOOL (* LPFNDLLGKEYISKEYBOARDGKEYBUTTONPRESSED)(int, int);
typedef wchar_t* (* LPFNDLLGKEYGETKEYBOARDGKEYSTRING)(int, int);
typedef void (* LPFNDLLGKEYSHUTDOWN)();

LPFNDLLGKEYINIT g_lpfnDllGkeyInit = NULL;
LPFNDLLGKEYINITWITHOUTCALLBACK  g_lpfnDllGkeyInitWithoutCallback = NULL;
LPFNDLLGKEYISMOUSEBUTTONPRESSED g_lpfnDllGkeyIsMouseButtonPressed = NULL;
LPFNDLLGKEYGETMOUSEBUTTONSTRING g_lpfnDllGkeyGetMouseButtonString = NULL;
LPFNDLLGKEYISKEYBOARDGKEYBUTTONPRESSED g_lpfnDllGkeyIsKeyboardGkeyPressed = NULL;
LPFNDLLGKEYGETKEYBOARDGKEYSTRING g_lpfnDllGkeyGetKeyboardGkeyString = NULL;
LPFNDLLGKEYSHUTDOWN g_lpfnDllGkeyShutdown = NULL;
#endif

void __cdecl GkeySDKCallback(GkeyCode gkeyCode, wchar_t* gkeyOrButtonString, void* /*pContext*/)
{
    SendMessage(g_hwnd, CALLBACKMSG, (WPARAM)&gkeyCode, (LPARAM)gkeyOrButtonString);
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


// CDisplayGkeysDlg dialog




CDisplayGkeysDlg::CDisplayGkeysDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDisplayGkeysDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDisplayGkeysDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_GKEYS, m_listGkeys);
}

BEGIN_MESSAGE_MAP(CDisplayGkeysDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDisplayGkeysDlg message handlers

BOOL CDisplayGkeysDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    g_hwnd = m_hWnd;

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    SetTimer(1, 30,NULL);

    logiGkeyCBContext gkeyContext;
    ZeroMemory(&gkeyContext, sizeof(gkeyContext));
    gkeyContext.gkeyCallBack = (logiGkeyCB)GkeySDKCallback;
    gkeyContext.gkeyContext = NULL;

#ifndef USE_LOGITECH_DLL_ONLY
#ifdef USE_CALLBACK
    LogiGkeyInit(&gkeyContext);
#else
    LogiGkeyInit(NULL);
#endif // USE_CALLBACK
#else
HINSTANCE logiDllHandle = LoadLibrary(L"LogitechGkey.dll");
ASSERT(NULL != logiDllHandle);
if (logiDllHandle != NULL)
{
    g_lpfnDllGkeyInit = (LPFNDLLGKEYINIT)GetProcAddress(logiDllHandle, "LogiGkeyInit");
    ASSERT(NULL != g_lpfnDllGkeyInit);
    g_lpfnDllGkeyInitWithoutCallback = (LPFNDLLGKEYINITWITHOUTCALLBACK)GetProcAddress(logiDllHandle, "LogiGkeyInitWithoutCallback");
    ASSERT(NULL != g_lpfnDllGkeyInitWithoutCallback);
    g_lpfnDllGkeyIsMouseButtonPressed = (LPFNDLLGKEYISMOUSEBUTTONPRESSED)GetProcAddress(logiDllHandle, "LogiGkeyIsMouseButtonPressed");
    ASSERT(NULL != g_lpfnDllGkeyIsMouseButtonPressed);
    g_lpfnDllGkeyGetMouseButtonString = (LPFNDLLGKEYGETMOUSEBUTTONSTRING)GetProcAddress(logiDllHandle, "LogiGkeyGetMouseButtonString");
    ASSERT(NULL != g_lpfnDllGkeyGetMouseButtonString);
    g_lpfnDllGkeyIsKeyboardGkeyPressed = (LPFNDLLGKEYISKEYBOARDGKEYBUTTONPRESSED)GetProcAddress(logiDllHandle, "LogiGkeyIsKeyboardGkeyPressed");
    ASSERT(NULL != g_lpfnDllGkeyIsKeyboardGkeyPressed);
    g_lpfnDllGkeyGetKeyboardGkeyString = (LPFNDLLGKEYGETKEYBOARDGKEYSTRING)GetProcAddress(logiDllHandle, "LogiGkeyGetKeyboardGkeyString");
    ASSERT(NULL != g_lpfnDllGkeyGetKeyboardGkeyString);
    g_lpfnDllGkeyShutdown = (LPFNDLLGKEYSHUTDOWN)GetProcAddress(logiDllHandle, "LogiGkeyShutdown");
    ASSERT(NULL != g_lpfnDllGkeyShutdown);

#ifdef USE_CALLBACK
    if (g_lpfnDllGkeyInit)
    {
        g_lpfnDllGkeyInit(&gkeyContext);
    }
#else
    if (g_lpfnDllGkeyInitWithoutCallback)
    {
        g_lpfnDllGkeyInitWithoutCallback();
    }
#endif
}
#endif

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDisplayGkeysDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDisplayGkeysDlg::OnPaint()
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
HCURSOR CDisplayGkeysDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDisplayGkeysDlg::OnTimer(UINT_PTR nIDEvent)
{
#ifndef USE_LOGITECH_DLL_ONLY
for (int index = 6; index <= LOGITECH_MAX_MOUSE_BUTTONS; index++)
    {
    if (LogiGkeyIsMouseButtonPressed(index))
    {
        m_listGkeys.InsertString(m_listGkeys.GetCount(), LogiGkeyGetMouseButtonString(index));
        m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
    }
}

for (int index = 1; index <= LOGITECH_MAX_GKEYS; index++)
{
    for (int mKeyIndex = 1; mKeyIndex <= LOGITECH_MAX_M_STATES; mKeyIndex++)
    {
        if (LogiGkeyIsKeyboardGkeyPressed(index, mKeyIndex))
        {
            m_listGkeys.InsertString(m_listGkeys.GetCount(), LogiGkeyGetKeyboardGkeyString(index, mKeyIndex));
            m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
        }
    }
}
#else
if (NULL == g_lpfnDllGkeyIsMouseButtonPressed || NULL == g_lpfnDllGkeyGetMouseButtonString
    || NULL == g_lpfnDllGkeyIsKeyboardGkeyPressed || NULL == g_lpfnDllGkeyGetKeyboardGkeyString)
    return;

for (int index = 6; index <= LOGITECH_MAX_MOUSE_BUTTONS; index++)
{
    if (g_lpfnDllGkeyIsMouseButtonPressed(index))
    {
        m_listGkeys.InsertString(m_listGkeys.GetCount(), g_lpfnDllGkeyGetMouseButtonString(index));
        m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
    }
}

for (int index = 1; index <= LOGITECH_MAX_GKEYS; index++)
{
    for (int mKeyIndex = 1; mKeyIndex <= LOGITECH_MAX_M_STATES; mKeyIndex++)
    {
        if (g_lpfnDllGkeyIsKeyboardGkeyPressed(index, mKeyIndex))
        {
            m_listGkeys.InsertString(m_listGkeys.GetCount(), g_lpfnDllGkeyGetKeyboardGkeyString(index, mKeyIndex));
            m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
        }
    }
}
#endif

    CDialogEx::OnTimer(nIDEvent);
}


void CDisplayGkeysDlg::OnClose()
{
#ifndef USE_LOGITECH_DLL_ONLY
    LogiGkeyShutdown();
#else
    if (g_lpfnDllGkeyShutdown)
    {
        g_lpfnDllGkeyShutdown();
    }
#endif

    CDialogEx::OnClose();
}


BOOL CDisplayGkeysDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    // TODO: Add your specialized code here and/or call the base class

    if (message == CALLBACKMSG)
    {
        GkeyCode* gkeyCode = (GkeyCode*)wParam;
        //wchar_t* text = (wchar_t*)lParam; // Can use text from callback, or use SDK function
        wchar_t* text = NULL;

        if (gkeyCode->mouse)
        {
#ifndef USE_LOGITECH_DLL_ONLY
            text = LogiGkeyGetMouseButtonString(gkeyCode->keyIdx);
#else
            text = g_lpfnDllGkeyGetMouseButtonString(gkeyCode->keyIdx);
#endif
            lstrcatW(text, (1 == gkeyCode->keyDown) ? L" down" : L" up");
            m_listGkeys.InsertString(m_listGkeys.GetCount(), text);
            m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
        }
        else
        {
#ifndef USE_LOGITECH_DLL_ONLY
            text = LogiGkeyGetKeyboardGkeyString(gkeyCode->keyIdx, gkeyCode->mState);
#else
            text = g_lpfnDllGkeyGetKeyboardGkeyString(gkeyCode->keyIdx, gkeyCode->mState);
#endif
            lstrcatW(text, (1 == gkeyCode->keyDown) ? L" down" : L" up");
            m_listGkeys.InsertString(m_listGkeys.GetCount(), text);
            m_listGkeys.SetCaretIndex(m_listGkeys.GetCount() - 1);
        }
    }

    return CDialogEx::OnWndMsg(message, wParam, lParam, pResult);
}
