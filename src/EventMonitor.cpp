#include "nilPnP.h"
#include "nilUtil.h"

namespace nil {

  const wchar_t* cEventMonitorClass = L"NIL_MONITOR";

  EventMonitor::EventMonitor( HINSTANCE instance ):
  mInstance( instance ), mClass( 0 ), mWindow( 0 ), mNotifications( 0 ),
  mInputBuffer( nullptr ),
  mInputBufferSize( 10240 ) // 10KB default
  {
    WNDCLASSEXW wx   = { 0 };
    wx.cbSize        = sizeof( WNDCLASSEXW );
    wx.lpfnWndProc   = wndProc;
    wx.hInstance     = mInstance;
    wx.lpszClassName = cEventMonitorClass;

    mClass = RegisterClassExW( &wx );
    if ( !mClass )
      NIL_EXCEPT_WINAPI( L"Window class registration failed" );

    mWindow = CreateWindowExW(
      0, (LPCWSTR)mClass, nullptr, 0, 0, 0, 0, 0, 0, 0, mInstance, this );
    if ( !mWindow )
      NIL_EXCEPT_WINAPI( L"Window creation failed" );

    mInputBuffer = malloc( mInputBufferSize );
    if ( !mInputBuffer )
      NIL_EXCEPT( L"Couldn't allocate input read buffer" );

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
  const String& devicePath )
  {
    for ( auto listener : mPnPListeners )
      listener->onPnPPlug( deviceClass, devicePath );
  }

  void EventMonitor::handleInterfaceRemoval( const GUID& deviceClass,
  const String& devicePath )
  {
    for ( auto listener : mPnPListeners )
      listener->onPnPUnplug( deviceClass, devicePath );
  }

  void EventMonitor::handleRawArrival( HANDLE handle )
  {
    for ( auto listener : mRawListeners )
      listener->onRawArrival( handle );
  }

  void EventMonitor::handleRawInput( HRAWINPUT input )
  {
    unsigned int dataSize;

    // TODO:LOW We could probably get away with just one GetRawInputData call?
    if ( GetRawInputData( input, RID_INPUT, NULL, &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 )
      return;

    if ( !dataSize )
      return;

    // Resize our input buffer if packet size exceeds previous cap
    if ( dataSize > mInputBufferSize )
    {
      mInputBufferSize = dataSize;
      mInputBuffer = realloc( mInputBuffer, dataSize );
      if ( !mInputBuffer )
        NIL_EXCEPT( L"Couldn't reallocate input read buffer" );
    }

    if ( GetRawInputData( input, RID_INPUT, mInputBuffer, &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 )
      return;

    auto raw = (const RAWINPUT*)mInputBuffer;

    // Ping our listeners
    if ( raw->header.dwType == RIM_TYPEMOUSE )
    {
      for ( auto listener : mRawListeners )
        listener->onRawMouseInput( raw->header.hDevice, raw->data.mouse );
    }
    else if ( raw->header.dwType == RIM_TYPEKEYBOARD )
    {
      for ( auto listener : mRawListeners )
        listener->onRawKeyboardInput( raw->header.hDevice, raw->data.keyboard );
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
      NIL_EXCEPT_WINAPI( L"Couldn't register for device interface notifications" );

    RAWINPUTDEVICE rawDevices[2];

    rawDevices[0].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    rawDevices[0].hwndTarget = mWindow;
    rawDevices[0].usUsagePage = USBUsagePage_Desktop;
    rawDevices[0].usUsage = USBUsage_Mice;

    rawDevices[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    rawDevices[1].hwndTarget = mWindow;
    rawDevices[1].usUsagePage = USBUsagePage_Desktop;
    rawDevices[1].usUsage = USBUsage_Keyboards;

    if ( !RegisterRawInputDevices( rawDevices, 2, sizeof( RAWINPUTDEVICE ) ) )
      NIL_EXCEPT_WINAPI( L"Couldn't register for raw input notifications" );
  }

  LRESULT CALLBACK EventMonitor::wndProc( HWND window, UINT message,
  WPARAM wParam, LPARAM lParam )
  {
    PDEV_BROADCAST_DEVICEINTERFACE_W broadcast;

    if ( message == WM_CREATE )
    {
      auto createstruct = (LPCREATESTRUCTW)lParam;
      auto me = (EventMonitor*)createstruct->lpCreateParams;
      SetWindowLongPtrW( window, GWLP_USERDATA, PtrToUlong( me ) );
      return 1;
    }

    auto ptr = static_cast<LONG_PTR>( GetWindowLongPtrW( window, GWLP_USERDATA ) );
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
        if ( GET_RAWINPUT_CODE_WPARAM( wParam ) == RIM_INPUTSINK )
        {
          me->handleRawInput( (HRAWINPUT)lParam );
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
    rawDevices[0].usUsage = USBUsage_Mice;

    rawDevices[0].dwFlags = RIDEV_REMOVE;
    rawDevices[0].hwndTarget = 0;
    rawDevices[0].usUsagePage = USBUsagePage_Desktop;
    rawDevices[0].usUsage = USBUsage_Keyboards;

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
    if ( mInputBuffer )
      free( mInputBuffer );
    if ( mWindow )
      DestroyWindow( mWindow );
    if ( mClass )
      UnregisterClassW( (LPCWSTR)mClass, mInstance );
  }

}