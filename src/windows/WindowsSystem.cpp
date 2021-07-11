#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilLogitech.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  void System::Internals::store()
  {
    swapMouseButtons = ( GetSystemMetrics( SM_SWAPBUTTON ) != 0 );

    storedStickyKeys.cbSize = sizeof( storedStickyKeys );
    SystemParametersInfoW( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &storedStickyKeys, 0 );

    storedToggleKeys.cbSize = sizeof( storedToggleKeys );
    SystemParametersInfoW( SPI_GETTOGGLEKEYS, sizeof( TOGGLEKEYS ), &storedToggleKeys, 0 );

    storedFilterKeys.cbSize = sizeof( storedFilterKeys );
    SystemParametersInfoW( SPI_GETFILTERKEYS, sizeof( FILTERKEYS ), &storedFilterKeys, 0 );
  }

  void System::Internals::disableHotKeyHarassment()
  {
    // Don't touch stickykeys/togglekeys/filterkeys if they're being used,
    // but if they aren't, make sure Windows doesn't harass the user about
    // maybe enabling them.

    auto stickyKeys = storedStickyKeys;
    if ( ( stickyKeys.dwFlags & SKF_STICKYKEYSON ) == 0 )
    {
      stickyKeys.dwFlags &= ~SKF_HOTKEYACTIVE;
      stickyKeys.dwFlags &= ~SKF_CONFIRMHOTKEY;
      SystemParametersInfoW( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &stickyKeys, 0 );
    }

    auto toggleKeys = storedToggleKeys;
    if ( ( toggleKeys.dwFlags & TKF_TOGGLEKEYSON ) == 0 )
    {
      toggleKeys.dwFlags &= ~TKF_HOTKEYACTIVE;
      toggleKeys.dwFlags &= ~TKF_CONFIRMHOTKEY;
      SystemParametersInfoW( SPI_SETTOGGLEKEYS, sizeof( TOGGLEKEYS ), &toggleKeys, 0 );
    }

    auto filterKeys = storedFilterKeys;
    if ( ( filterKeys.dwFlags & FKF_FILTERKEYSON ) == 0 )
    {
      filterKeys.dwFlags &= ~FKF_HOTKEYACTIVE;
      filterKeys.dwFlags &= ~FKF_CONFIRMHOTKEY;
      SystemParametersInfoW( SPI_SETFILTERKEYS, sizeof( FILTERKEYS ), &filterKeys, 0 );
    }
  }

  void System::Internals::restore()
  {
    SystemParametersInfoW( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &storedStickyKeys, 0 );
    SystemParametersInfoW( SPI_SETTOGGLEKEYS, sizeof( TOGGLEKEYS ), &storedToggleKeys, 0 );
    SystemParametersInfoW( SPI_SETFILTERKEYS, sizeof( FILTERKEYS ), &storedFilterKeys, 0 );
  }

  System::System( HINSTANCE instance, HWND window, const Cooperation coop,
  SystemListener* listener ): coop_( coop ),
  window_( window ), instance_( instance ), dinput_( nullptr ),
  idPool_( 0 ), isInitializing_( true ),
  listener_( listener ), mouseIdPool_( 0 ), keyboardIdPool_( 0 ),
  controllerIdPool_( 0 )
  {
    assert( listener_ );

    // Validate the passes window handle
    if ( !IsWindow( window_ ) )
      NIL_EXCEPT( "Window handle is invalid" );

    // Store accessibility feature states, and tell Windows not to be annoying
    internals_.store();
    internals_.disableHotKeyHarassment();

    // Init Logitech SDKs where available
    logitechGkeys_ = make_unique<logitech::GKeySDK>();
    logitechLeds_ = make_unique<logitech::LedSDK>();

    // Init XInput subsystem
    xinput_ = make_unique<XInput>();
    if ( xinput_->initialize() != ExternalModule::Initialization_OK )
      NIL_EXCEPT( "Loading XInput failed" );

    // Create DirectInput instance
    auto hr = DirectInput8Create( instance_, DIRECTINPUT_VERSION,
      IID_IDirectInput8W, (LPVOID*)&dinput_, nullptr );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not instantiate DirectInput 8" );

    // Initialize our event monitor
    eventMonitor_ = make_unique<windows::EventMonitor>( instance_, coop_ );

    // Initialize our HID manager
    hidManager_ = make_unique<windows::HIDManager>();

    // Register the HID manager and ourselves as PnP event listeners
    eventMonitor_->registerPnPListener( hidManager_.get() );
    eventMonitor_->registerPnPListener( this );

    // Register ourselves as a raw event listener
    eventMonitor_->registerRawListener( this );

    // Fetch initial devices
    initializeDevices();
    refreshDevices();

    // Update the monitor once, to receive initial Raw devices
    eventMonitor_->update();

    isInitializing_ = false;
  }

  DeviceID System::getNextID()
  {
    return idPool_++;
  }

  bool System::getDefaultMouseButtonSwapping()
  {
    return internals_.swapMouseButtons;
  }

  void System::onPnPPlug( const GUID& deviceClass, const wideString& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onPnPUnplug( const GUID& deviceClass, const wideString& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onRawArrival( HANDLE handle )
  {
    UINT pathLength = 0;

    if ( GetRawInputDeviceInfoW( handle, RIDI_DEVICENAME, nullptr, &pathLength ) )
      NIL_EXCEPT_WINAPI( "GetRawInputDeviceInfoW failed" );

    wideString rawPath( pathLength, '\0' );

    GetRawInputDeviceInfoW( handle, RIDI_DEVICENAME, &rawPath[0], &pathLength );
    rawPath.resize( rawPath.length() - 1 );

    for ( auto device : devices_ )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = static_cast<RawInputDevice*>( device );

      if ( _wcsicmp( rawDevice->getRawPath().c_str(), rawPath.c_str() ) == 0 )
      {
        deviceConnect( rawDevice );
        return;
      }
    }

    auto device = new RawInputDevice( this, getNextID(), handle, rawPath );

    if ( isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      deviceConnect( device );

    devices_.push_back( device );
  }

  void System::onRawMouseInput( HANDLE handle,
  const RAWMOUSE& input, const bool sinked )
  {
    if ( isInitializing_ || !handle )
      return;

    auto it = mouseMap_.find( handle );
    if ( it != mouseMap_.end() )
      it->second->onRawInput( input );
  }

  void System::onRawKeyboardInput( HANDLE handle,
  const RAWKEYBOARD& input, const bool sinked )
  {
    if ( isInitializing_ || !handle )
      return;

    auto it = keyboardMap_.find( handle );
    if ( it != keyboardMap_.end() )
      it->second->onRawInput( input );
  }

  void System::onRawRemoval( HANDLE handle )
  {
    for ( auto device : devices_ )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = static_cast<RawInputDevice*>( device );

      if ( rawDevice->getRawHandle() == handle )
      {
        deviceDisconnect( rawDevice );
        return;
      }
    }
  }

  void System::mapMouse( HANDLE handle, RawInputMouse* mouse )
  {
    mouseMap_[handle] = mouse;
  }

  void System::unmapMouse( HANDLE handle )
  {
    mouseMap_.erase( handle );
  }

  void System::mapKeyboard( HANDLE handle, RawInputKeyboard* keyboard )
  {
    keyboardMap_[handle] = keyboard;
  }

  void System::unmapKeyboard( HANDLE handle )
  {
    keyboardMap_.erase( handle );
  }

  void System::initializeDevices()
  {
    xinputIds_.resize( XUSER_MAX_COUNT );
    for ( int i = 0; i < XUSER_MAX_COUNT; i++ )
    {
      xinputIds_[i] = getNextID();
      auto device = new XInputDevice( this, xinputIds_[i], i );
      devices_.push_back( device );
    }
  }

  void System::refreshDevices()
  {
    identifyXInputDevices();

    for ( Device* device : devices_ )
      if ( device->getHandler() == Device::Handler_DirectInput )
      {
        device->saveStatus();
        device->setStatus( Device::Status_Pending );
      }

    auto hr = dinput_->EnumDevices( DI8DEVCLASS_GAMECTRL,
      diDeviceEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not enumerate DirectInput devices!" );

    for ( Device* device : devices_ )
      if ( device->getHandler() == Device::Handler_DirectInput
        && device->getSavedStatus() == Device::Status_Connected
        && device->getStatus() == Device::Status_Pending )
        deviceDisconnect( device );

    XINPUT_STATE state;
    for ( Device* device : devices_ )
    {
      if ( device->getHandler() == Device::Handler_XInput )
      {
        auto xDevice = static_cast<XInputDevice*>( device );
        auto status = xinput_->funcs_.pfnXInputGetState( xDevice->getXInputID(), &state );
        if ( status == ERROR_DEVICE_NOT_CONNECTED )
        {
          if ( xDevice->getStatus() == Device::Status_Connected )
            deviceDisconnect( xDevice );
          else if ( xDevice->getStatus() == Device::Status_Pending )
            xDevice->setStatus( Device::Status_Disconnected );
        }
        else if ( status == ERROR_SUCCESS )
        {
          if ( xDevice->getStatus() == Device::Status_Disconnected )
            deviceConnect( xDevice );
          else if ( xDevice->getStatus() == Device::Status_Pending )
            xDevice->setStatus( Device::Status_Connected );
        }
        else
          NIL_EXCEPT( "XInputGetState failed" );
      }
    }
  }

  BOOL CALLBACK System::diDeviceEnumCallback( LPCDIDEVICEINSTANCEW instance,
  LPVOID referer )
  {
    auto system = reinterpret_cast<System*>( referer );

    for ( auto identifier : system->xinputDeviceIds_ )
      if ( instance->guidProduct.Data1 == identifier )
        return DIENUM_CONTINUE;

    for ( auto device : system->devices_ )
    {
      if ( device->getHandler() != Device::Handler_DirectInput )
        continue;

      auto diDevice = static_cast<DirectInputDevice*>( device );

      if ( diDevice->getInstanceID() == instance->guidInstance )
      {
        if ( device->getSavedStatus() == Device::Status_Disconnected )
          system->deviceConnect( device );
        else
          device->setStatus( Device::Status_Connected );

        return DIENUM_CONTINUE;
      }
    }

    Device* device = new DirectInputDevice( system, system->getNextID(), instance );

    if ( system->isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      system->deviceConnect( device );

    system->devices_.push_back( device );

    return DIENUM_CONTINUE;
  }

  void System::deviceConnect( Device* device )
  {
    device->onConnect();
    listener_->onDeviceConnected( device );
  }

  void System::deviceDisconnect( Device* device )
  {
    device->onDisconnect();
    listener_->onDeviceDisconnected( device );
  }

  void System::mouseEnabled( Device* device, Mouse* instance )
  {
    listener_->onMouseEnabled( device, instance );
  }

  void System::mouseDisabled( Device* device, Mouse* instance )
  {
    listener_->onMouseDisabled( device, instance );
  }

  void System::keyboardEnabled( Device* device, Keyboard* instance )
  {
    listener_->onKeyboardEnabled( device, instance );
  }

  void System::keyboardDisabled( Device* device, Keyboard* instance )
  {
    listener_->onKeyboardDisabled( device, instance );
  }

  void System::controllerEnabled( Device* device, Controller* instance )
  {
    listener_->onControllerEnabled( device, instance );
  }

  void System::controllerDisabled( Device* device, Controller* instance )
  {
    listener_->onControllerDisabled( device, instance );
  }

  void System::identifyXInputDevices()
  {
    xinputDeviceIds_.clear();
    for ( auto hidRecord : hidManager_->getRecords() )
      if ( hidRecord->isXInput() )
        xinputDeviceIds_.push_back( hidRecord->getIdentifier() );
  }

  DeviceList& System::getDevices()
  {
    return devices_;
  }

  const bool System::isInitializing() const
  {
    return isInitializing_;
  }

  int System::getNextMouseIndex()
  {
    return ++mouseIdPool_;
  }

  int System::getNextKeyboardIndex()
  {
    return ++keyboardIdPool_;
  }

  int System::getNextControllerIndex()
  {
    return ++controllerIdPool_;
  }

  void System::update()
  {
    // Run PnP & raw events if there are any
    eventMonitor_->update();

    // Make sure that we disconnect failed devices,
    // and update the rest
    for ( Device* device : devices_ )
      if ( device->isDisconnectFlagged() )
        deviceDisconnect( device );
      else
        device->update();

    // Run queued G-key events if using the SDK
    if ( logitechGkeys_->isInitialized() )
      logitechGkeys_->update();
  }

  logitech::GKeySDK* System::getLogitechGKeys()
  {
    return logitechGkeys_.get();
  }

  logitech::LedSDK* System::getLogitechLEDs()
  {
    return logitechLeds_.get();
  }

  XInput* System::getXInput()
  {
    return xinput_.get();
  }

  System::~System()
  {
    for ( Device* device : devices_ )
    {
      device->disable();
      delete device;
    }

    if ( dinput_ )
      dinput_->Release();

    // Restore accessiblity features
    internals_.restore();
  }

}

#endif