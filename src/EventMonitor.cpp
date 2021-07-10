#include "nilPnP.h"
#include "nilUtil.h"

namespace nil {

  const wchar_t* cEventMonitorClass = L"NIL_MONITOR";

  EventMonitor::EventMonitor( HINSTANCE instance, const Cooperation coop ):
  mInstance( instance ), mClass( 0 ), mWindow( 0 ), mNotifications( 0 ),
  mInputBuffer( nullptr ), mCooperation( coop ),
  mInputBufferSize( 10240 ) // 10KB default
  {
    WNDCLASSEXW wx   = { 0 };
    wx.cbSize        = sizeof( WNDCLASSEXW );
    wx.lpfnWndProc   = wndProc;
    wx.hInstance     = mInstance;
    wx.lpszClassName = cEventMonitorClass;

    mClass = RegisterClassExW( &wx );
    if ( !mClass )
      NIL_EXCEPT_WINAPI( "Window class registration failed" );

    mWindow = CreateWindowExW(
      0, (LPCWSTR)mClass, nullptr, 0, 0, 0, 0, 0, 0, 0, mInstance, this ); //-V542
    if ( !mWindow )
      NIL_EXCEPT_WINAPI( "Window creation failed" );

    mInputBuffer = malloc( (size_t)mInputBufferSize );
    if ( !mInputBuffer )
      NIL_EXCEPT( "Couldn't allocate input read buffer" );

    registerNotifications();
  }

  void EventMonitor::registerPnPListener( PnPListener* listener )
  {
    mPnPListeners.push_back( listener );
  }

  void EventMonitor::unregisterPnPListener( PnPListener* listener )
  {
    mPnPListeners.remove( listener );
  }

  void EventMonitor::registerRawListener( RawListener* listener )
  {
    mRawListeners.push_back( listener );
  }

  void EventMonitor::unregisterRawListener( RawListener* listener )
  {
    mRawListeners.remove( listener );
  }

  void EventMonitor::handleInterfaceArrival( const GUID& deviceClass,
  const wideString& devicePath )
  {
    for ( auto listener : mPnPListeners )
      listener->onPnPPlug( deviceClass, devicePath );
  }

  void EventMonitor::handleInterfaceRemoval( const GUID& deviceClass,
  const wideString& devicePath )
  {
    for ( auto listener : mPnPListeners )
      listener->onPnPUnplug( deviceClass, devicePath );
  }

  void EventMonitor::handleRawArrival( HANDLE handle )
  {
    for ( auto listener : mRawListeners )
      listener->onRawArrival( handle );
  }

  void EventMonitor::handleRawInput( HRAWINPUT input, const bool sinked )
  {
    unsigned int dataSize = 0;

    // TODO:LOW We could probably get away with just one GetRawInputData call?
    if ( GetRawInputData( input, RID_INPUT, nullptr, &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 )
      return;

    if ( !dataSize )
      return;

    // Resize our input buffer if packet size exceeds previous cap
    if ( dataSize > mInputBufferSize )
    {
      mInputBufferSize = dataSize;
      mInputBuffer = ( mInputBuffer ? realloc( mInputBuffer, (size_t)dataSize ) : malloc( (size_t)dataSize ) );
      if ( !mInputBuffer )
        NIL_EXCEPT( "Couldn't reallocate input read buffer" );
    }

    if ( GetRawInputData( input, RID_INPUT, mInputBuffer, &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 )
      return;

    auto raw = (const RAWINPUT*)mInputBuffer;

    // Ping our listeners
    if ( raw->header.dwType == RIM_TYPEMOUSE )
    {
      for ( auto listener : mRawListeners )
        listener->onRawMouseInput( raw->header.hDevice, raw->data.mouse, sinked );
    }
    else if ( raw->header.dwType == RIM_TYPEKEYBOARD )
    {
      for ( auto listener : mRawListeners )
        listener->onRawKeyboardInput( raw->header.hDevice, raw->data.keyboard, sinked );
    }
  }

  void EventMonitor::handleRawRemoval( HANDLE handle )
  {
    for ( auto listener : mRawListeners )
      listener->onRawRemoval( handle );
  }

  void EventMonitor::registerNotifications()
  {
    DEV_BROADCAST_DEVICEINTERFACE filter;
    ZeroMemory( &filter, sizeof( filter ) );

    filter.dbcc_size       = sizeof( filter );
    filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    filter.dbcc_classguid  = g_HIDInterfaceGUID;

    mNotifications = RegisterDeviceNotificationW( mWindow, &filter,
      DEVICE_NOTIFY_WINDOW_HANDLE );
    if ( !mNotifications )
      NIL_EXCEPT_WINAPI( "Couldn't register for device interface notifications" );

    RAWINPUTDEVICE rawDevices[2];

    rawDevices[0].dwFlags =
      ( mCooperation == Cooperation_Background )
      ? RIDEV_DEVNOTIFY | RIDEV_INPUTSINK
      : RIDEV_DEVNOTIFY;
    rawDevices[0].hwndTarget = mWindow;
    rawDevices[0].usUsagePage = USBUsagePage_Desktop;
    rawDevices[0].usUsage = USBDesktopUsage_Mice;

    rawDevices[1].dwFlags =
      ( mCooperation == Cooperation_Background )
      ? RIDEV_DEVNOTIFY | RIDEV_INPUTSINK
      : RIDEV_DEVNOTIFY;
    rawDevices[1].hwndTarget = mWindow;
    rawDevices[1].usUsagePage = USBUsagePage_Desktop;
    rawDevices[1].usUsage = USBDesktopUsage_Keyboards;

    if ( !RegisterRawInputDevices( rawDevices, 2, sizeof( RAWINPUTDEVICE ) ) )
      NIL_EXCEPT_WINAPI( "Couldn't register for raw input notifications" );
  }

  LRESULT CALLBACK EventMonitor::wndProc( HWND window, UINT message,
  WPARAM wParam, LPARAM lParam )
  {
    PDEV_BROADCAST_DEVICEINTERFACE_W broadcast;

    if ( message == WM_CREATE )
    {
      auto createstruct = (LPCREATESTRUCTW)lParam;
      auto me = (EventMonitor*)createstruct->lpCreateParams;
      SetWindowLongPtrW( window, GWLP_USERDATA, (DWORD_PTR)me );
      return 1;
    }

    auto ptr = static_cast<DWORD_PTR>( GetWindowLongPtrW( window, GWLP_USERDATA ) );
    auto me = reinterpret_cast<EventMonitor*>( ptr );

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
            me->handleInterfaceArrival( broadcast->dbcc_classguid, broadcast->dbcc_name );
          }
          else if ( wParam == DBT_DEVICEREMOVECOMPLETE )
          {
            me->handleInterfaceRemoval( broadcast->dbcc_classguid, broadcast->dbcc_name );
          }
        }
        return TRUE;
      break;
      case WM_INPUT_DEVICE_CHANGE:
        if ( wParam == GIDC_ARRIVAL )
        {
          me->handleRawArrival( (HANDLE)lParam );
        }
        else if ( wParam == GIDC_REMOVAL )
        {
          me->handleRawRemoval( (HANDLE)lParam );
        }
        return 0;
      break;
      case WM_INPUT:
        if ( GET_RAWINPUT_CODE_WPARAM( wParam ) == RIM_INPUT )
        {
          me->handleRawInput( (HRAWINPUT)lParam, false );
        }
        else if ( GET_RAWINPUT_CODE_WPARAM( wParam ) == RIM_INPUTSINK )
        {
          me->handleRawInput( (HRAWINPUT)lParam, true );
        }
        return DefWindowProcW( window, message, wParam, lParam );
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

  void EventMonitor::unregisterNotifications()
  {
    RAWINPUTDEVICE rawDevices[2];

    rawDevices[0].dwFlags = RIDEV_REMOVE;
    rawDevices[0].hwndTarget = 0;
    rawDevices[0].usUsagePage = USBUsagePage_Desktop;
    rawDevices[0].usUsage = USBDesktopUsage_Mice;

    rawDevices[0].dwFlags = RIDEV_REMOVE;
    rawDevices[0].hwndTarget = 0;
    rawDevices[0].usUsagePage = USBUsagePage_Desktop;
    rawDevices[0].usUsage = USBDesktopUsage_Keyboards;

    RegisterRawInputDevices( rawDevices, 2, sizeof( RAWINPUTDEVICE ) );

    if ( mNotifications )
    {
      UnregisterDeviceNotification( mNotifications );
      mNotifications = 0;
    }
  }

  void EventMonitor::update()
  {
    MSG msg;
    // Run all queued messages
    while ( PeekMessageW( &msg, mWindow, 0, 0, PM_REMOVE ) > 0 )
    {
      DispatchMessageW( &msg );
    }
  }

  EventMonitor::~EventMonitor()
  {
    unregisterNotifications();
    free( mInputBuffer );
    if ( mWindow )
      DestroyWindow( mWindow );
    if ( mClass )
      UnregisterClassW( (LPCWSTR)mClass, mInstance ); //-V542
  }

}