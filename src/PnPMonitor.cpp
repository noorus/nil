#include "nil.h"
#include "nilUtil.h"

const wchar_t* cPnPMonitorClass = L"NILPNP";

namespace nil {

  PnPMonitor::PnPMonitor( HINSTANCE instance, PnPListener* listener ):
  mInstance( instance ), mClass( 0 ), mWindow( 0 ), mNotifications( 0 )
  {
    WNDCLASSEXW wx   = { 0 };
    wx.cbSize        = sizeof( WNDCLASSEXW );
    wx.lpfnWndProc   = wndProc;
    wx.hInstance     = mInstance;
    wx.lpszClassName = cPnPMonitorClass;

    mClass = RegisterClassExW( &wx );
    if ( !mClass )
      NIL_EXCEPT_WINAPI( L"Window class registration failed" );

    mWindow = CreateWindowExW(
      0, (LPCWSTR)mClass, nullptr, 0, 0, 0, 0, 0, 0, 0, mInstance, this );
    if ( !mWindow )
      NIL_EXCEPT_WINAPI( L"Window creation failed" );

    registerNotifications();
  }

  void PnPMonitor::registerListener( PnPListener* listener )
  {
    mListeners.push_back( listener );
  }

  void PnPMonitor::unregisterListener( PnPListener* listener )
  {
    mListeners.remove( listener );
  }

  void PnPMonitor::handleArrival( const GUID& deviceClass,
  const String& devicePath )
  {
    for ( auto listener : mListeners )
      listener->onPlug( deviceClass, devicePath );
  }

  void PnPMonitor::handleRemoval( const GUID& deviceClass,
  const String& devicePath )
  {
    for ( auto listener : mListeners )
      listener->onUnplug( deviceClass, devicePath );
  }

  void PnPMonitor::registerNotifications()
  {
    DEV_BROADCAST_DEVICEINTERFACE filter;
    ZeroMemory( &filter, sizeof( filter ) );

    filter.dbcc_size       = sizeof( filter );
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid  = g_HIDInterfaceGUID;

    mNotifications = RegisterDeviceNotification( mWindow, &filter,
      DEVICE_NOTIFY_WINDOW_HANDLE );
    if ( !mNotifications )
      NIL_EXCEPT_WINAPI( L"Couldn't register device notification handler" );
  }

  LRESULT CALLBACK PnPMonitor::wndProc( HWND window, UINT message,
  WPARAM wParam, LPARAM lParam )
  {
    PDEV_BROADCAST_DEVICEINTERFACE_W broadcast;

    if ( message == WM_CREATE )
    {
      auto createstruct = (LPCREATESTRUCTW)lParam;
      auto me = (PnPMonitor*)createstruct->lpCreateParams;
      SetWindowLongPtrW( window, GWLP_USERDATA, PtrToUlong( me ) );
      return 1;
    }

    auto ptr = static_cast<LONG_PTR>( GetWindowLongPtrW( window, GWLP_USERDATA ) );
    auto me = reinterpret_cast<PnPMonitor*>( ptr );

    if ( !me )
      return DefWindowProcW( window, message, wParam, lParam );

    switch ( message )
    {
      case WM_DEVICECHANGE:
        broadcast = (PDEV_BROADCAST_DEVICEINTERFACE_W)lParam;
        if ( broadcast && broadcast->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
        {
          if ( wParam == DBT_DEVICEARRIVAL )
          {
            me->handleArrival( broadcast->dbcc_classguid, broadcast->dbcc_name );
          }
          else if ( wParam == DBT_DEVICEREMOVECOMPLETE )
          {
            me->handleRemoval( broadcast->dbcc_classguid, broadcast->dbcc_name );
          }
        }
        return TRUE;
      break;
      case WM_CLOSE:
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

  void PnPMonitor::unregisterNotifications()
  {
    if ( mNotifications )
    {
      UnregisterDeviceNotification( mNotifications );
      mNotifications = 0;
    }
  }

  void PnPMonitor::update()
  {
    MSG msg;
    while ( PeekMessageW( &msg, mWindow, 0, 0, PM_REMOVE ) > 0 )
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
  }

  PnPMonitor::~PnPMonitor()
  {
    unregisterNotifications();
    if ( mWindow )
      DestroyWindow( mWindow );
    if ( mClass )
      UnregisterClassW( (LPCWSTR)mClass, mInstance );
  }

}