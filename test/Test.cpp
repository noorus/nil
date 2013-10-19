#define NTDDI_VERSION NTDDI_WS08SP4
#define _WIN32_WINNT _WIN32_WINNT_WS08
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <dbt.h>
#include <devguid.h>
#include <hidclass.h>

GUID guid = { 0x4D1E55B2L, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

class Window {
protected:
  HINSTANCE mInstance;
  ATOM mClass;
  HWND mWindow;
  HDEVNOTIFY mNotifications;
public:
  Window();
  void registerNotifications();
  void unregisterNotifications();
  void pump();
  ~Window();
  static LRESULT CALLBACK wndProc( HWND window, UINT message,
    WPARAM wParam, LPARAM lParam );
};

void Window::registerNotifications()
{
  wprintf_s( L"Window::registerNotifications()\r\n" );

  DEV_BROADCAST_DEVICEINTERFACE filter;
  ZeroMemory( &filter, sizeof( filter ) );

  filter.dbcc_size = sizeof( filter );
  filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  filter.dbcc_classguid = guid;

  mNotifications = RegisterDeviceNotification( mWindow, &filter,
    DEVICE_NOTIFY_WINDOW_HANDLE );
  if ( !mNotifications )
    throw std::exception( "Couldn't register for device notifications" );
}

void Window::unregisterNotifications()
{
  wprintf_s( L"Window::unregisterNotifications()\r\n" );
  UnregisterDeviceNotification( mNotifications );
}

void Window::pump()
{
  MSG msg;
  int ret;
  while ( ( ret = GetMessage( &msg, NULL, 0, 0 ) ) != 0 )
  {
    if ( ret == -1 )
      break;
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
}

LRESULT CALLBACK Window::wndProc( HWND window, UINT message,
WPARAM wParam, LPARAM lParam )
{
  PDEV_BROADCAST_DEVICEINTERFACE_W broadcast;

  if ( message == WM_CREATE )
  {
    LPCREATESTRUCTW pcs = (LPCREATESTRUCTW)lParam;
    Window* me = (Window*)pcs->lpCreateParams;
    SetWindowLongPtrW( window, GWLP_USERDATA, PtrToUlong( me ) );
    return 1;
  }

  LONG_PTR ptr = static_cast<LONG_PTR>( GetWindowLongPtrW( window, GWLP_USERDATA ) );
  Window* me = reinterpret_cast<Window*>( ptr );

  if ( !me )
    return DefWindowProcW( window, message, wParam, lParam );

  switch ( message )
  {
    case WM_DEVICECHANGE:
      broadcast = (PDEV_BROADCAST_DEVICEINTERFACE_W)lParam;
      if ( broadcast && broadcast->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
      {
        if ( wParam == DBT_DEVICEARRIVAL ) {
          wprintf_s( L"Device plugged: %s\r\n", broadcast->dbcc_name );
        } else if ( wParam == DBT_DEVICEREMOVECOMPLETE ) {
          wprintf_s( L"Device unplugged: %s\r\n", broadcast->dbcc_name );
        }
      }
      return TRUE;
    break;
    case WM_CLOSE:
      // don't react to outside close requests
      return 0;
    break;
    case WM_DESTROY:
      me->unregisterNotifications();
      return 0;
    break;
    default:
      return DefWindowProcW( window, message, wParam, lParam );
    break;
  }
}

Window::Window(): mInstance( 0 ), mClass( 0 ), mWindow( 0 ),
mNotifications( 0 )
{
  wprintf_s( L"Window::Window()\r\n" );

  mInstance = GetModuleHandleW( nullptr );

  WNDCLASSEXW wx = { 0 };
  wx.cbSize = sizeof( WNDCLASSEXW );
  wx.lpfnWndProc = wndProc;
  wx.hInstance = mInstance;
  wx.lpszClassName = L"USBDETCLS";

  mClass = RegisterClassExW( &wx );
  if ( !mClass )
    throw std::exception( "window class registration failed" );

  mWindow = CreateWindowExW(
    0, (LPCWSTR)mClass, nullptr, 0, 0, 0, 0, 0, 0, 0, mInstance, this );
  if ( !mWindow )
    throw std::exception( "window creation failed" );
}

Window::~Window()
{
  wprintf_s( L"Window::~Window()\r\n" );
  if ( mWindow )
    DestroyWindow( mWindow );
  if ( mClass )
    UnregisterClassW( (LPCWSTR)mClass, mInstance );
}

int wmain( int argc, wchar_t** argv, wchar_t** env )
{
  try {
    wprintf_s( L"Creating\r\n" );
    Window* wnd = new Window();
    wnd->registerNotifications();
    wprintf_s( L"Pumping\r\n" );
    wnd->pump();
    wprintf_s( L"Dying\r\n" );
    delete wnd;
  } catch ( std::exception& e ) {
    wprintf_s( L"Exception %S\r\n", e.what() );
  } catch ( ... ) {
    wprintf_s( L"Unknown exception\r\n" );
  }

  return EXIT_SUCCESS;
}