#include "nilConfig.h"

#include "nil.h"
#include "nilWindowsPNP.h"
#include "nilUtil.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  namespace windows {

    const wchar_t* cEventMonitorClass = L"NIL_MONITOR";

    EventMonitor::EventMonitor( HINSTANCE instance, const Cooperation coop ):
    instance_( instance ), coop_( coop )
    {
      WNDCLASSEXW wx = {
        .cbSize = sizeof( WNDCLASSEXW ),
        .style = 0,
        .lpfnWndProc = wndProc,
        .hInstance = instance_,
        .lpszClassName = cEventMonitorClass };

      class_ = RegisterClassExW( &wx );
      if ( !class_ )
        NIL_EXCEPT_WINAPI( "Window class registration failed" );

      window_ = CreateWindowExW(
        0, reinterpret_cast<LPCWSTR>( class_ ), nullptr, 0, 0, 0, 0, 0, 0, 0, instance_, this );
      if ( !window_ )
        NIL_EXCEPT_WINAPI( "Window creation failed" );

      // Initial read buffer size, should avoid reallocations
      inputBuffer_.resize( 1024 * 10, 0 );

      registerNotifications();
    }

    void EventMonitor::registerPnPListener( PnPListenerPtr listener )
    {
      pnpListeners_.push_back( listener );
    }

    void EventMonitor::unregisterPnPListener( PnPListenerPtr listener )
    {
      pnpListeners_.remove( listener );
    }

    void EventMonitor::registerRawListener( RawListenerPtr listener )
    {
      rawListeners_.push_back( listener );
    }

    void EventMonitor::unregisterRawListener( RawListenerPtr listener )
    {
      rawListeners_.remove( listener );
    }

    void EventMonitor::handleInterfaceArrival( const GUID& deviceClass,
    const wideString& devicePath )
    {
      for ( auto& listener : pnpListeners_ )
        listener->onPnPPlug( deviceClass, devicePath );
    }

    void EventMonitor::handleInterfaceRemoval( const GUID& deviceClass,
    const wideString& devicePath )
    {
      for ( auto& listener : pnpListeners_ )
        listener->onPnPUnplug( deviceClass, devicePath );
    }

    void EventMonitor::handleRawArrival( HANDLE handle )
    {
      for ( auto& listener : rawListeners_ )
        listener->onRawArrival( handle );
    }

    void EventMonitor::handleRawInput( HRAWINPUT input, const bool sinked )
    {
      unsigned int dataSize = 0;

      if ( GetRawInputData( input, RID_INPUT, nullptr, &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 || !dataSize )
        return;

      // Resize our input buffer if packet size exceeds previous cap
      if ( dataSize > inputBuffer_.size() )
        inputBuffer_.resize( dataSize, 0 );

      if ( GetRawInputData( input, RID_INPUT, inputBuffer_.data(), &dataSize, sizeof( RAWINPUTHEADER ) ) == (UINT)-1 )
        return;

      auto raw = reinterpret_cast<const RAWINPUT*>( inputBuffer_.data() );

      // Ping our listeners
      if ( raw->header.dwType == RIM_TYPEMOUSE )
      {
        for ( auto& listener : rawListeners_ )
          listener->onRawMouseInput( raw->header.hDevice, raw->data.mouse, sinked );
      }
      else if ( raw->header.dwType == RIM_TYPEKEYBOARD )
      {
        for ( auto& listener : rawListeners_ )
          listener->onRawKeyboardInput( raw->header.hDevice, raw->data.keyboard, sinked );
      }
      else if ( raw->header.dwType == RIM_TYPEHID )
      {
        for ( auto& listener : rawListeners_ )
          listener->onRawHIDInput( raw->header.hDevice, raw->data.hid, sinked );
      }
    }

    void EventMonitor::handleRawRemoval( HANDLE handle )
    {
      for ( auto& listener : rawListeners_ )
        listener->onRawRemoval( handle );
    }

    const set<uint16_t> c_wantedUsages = {
      USBDesktopUsage_Mice,
      USBDesktopUsage_Joysticks,
      USBDesktopUsage_Gamepads,
      USBDesktopUsage_Keyboards,
      USBDesktopUsage_MultiAxes };

    void EventMonitor::registerNotifications()
    {
      DEV_BROADCAST_DEVICEINTERFACE filter;
      ZeroMemory( &filter, sizeof( filter ) );

      filter.dbcc_size = sizeof( filter );
      filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      filter.dbcc_classguid = g_HIDInterfaceGUID;

      notifications_ = RegisterDeviceNotificationW( window_, &filter,
        DEVICE_NOTIFY_WINDOW_HANDLE );
      if ( !notifications_ )
        NIL_EXCEPT_WINAPI( "Couldn't register for device interface notifications" );

      uint32_t flags = ( coop_ == Cooperation::Background )
        ? ( RIDEV_DEVNOTIFY | RIDEV_INPUTSINK )
        : ( RIDEV_DEVNOTIFY );

      int i = 0;
      vector<RAWINPUTDEVICE> rawDevices( c_wantedUsages.size() );
      for ( auto& usage : c_wantedUsages )
      {
        bool isMiceKbd = ( usage == USBDesktopUsage_Mice || usage == USBDesktopUsage_Keyboards );
        rawDevices.data()[i].dwFlags = ( flags | ( isMiceKbd ? RIDEV_NOLEGACY : 0 ) );
        rawDevices.data()[i].hwndTarget = window_;
        rawDevices.data()[i].usUsagePage = USBUsagePage_Desktop;
        rawDevices.data()[i].usUsage = usage;
        i++;
      }

      if ( !RegisterRawInputDevices( rawDevices.data(), static_cast<UINT>( rawDevices.size() ), sizeof( RAWINPUTDEVICE ) ) )
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
      int i = 0;
      vector<RAWINPUTDEVICE> rawDevices( c_wantedUsages.size() );
      for ( auto& usage : c_wantedUsages )
      {
        rawDevices.data()[i].dwFlags = RIDEV_REMOVE;
        rawDevices.data()[i].hwndTarget = nullptr;
        rawDevices.data()[i].usUsagePage = USBUsagePage_Desktop;
        rawDevices.data()[i].usUsage = usage;
        i++;
      }

      RegisterRawInputDevices( rawDevices.data(), static_cast<UINT>( rawDevices.size() ), sizeof( RAWINPUTDEVICE ) );

      if ( notifications_ )
      {
        UnregisterDeviceNotification( notifications_ );
        notifications_ = 0;
      }
    }

    void EventMonitor::update()
    {
      MSG msg;
      while ( PeekMessageW( &msg, window_, 0, 0, PM_REMOVE ) > 0 )
        DispatchMessageW( &msg );
    }

    EventMonitor::~EventMonitor()
    {
      unregisterNotifications();
      if ( window_ )
        DestroyWindow( window_ );
      if ( class_ )
        UnregisterClassW( reinterpret_cast<LPCWSTR>( class_ ), instance_ );
    }

  }

}

#endif