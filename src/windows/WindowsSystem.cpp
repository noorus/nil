#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

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

  void System::Internals::disableHotkeyHelpers()
  {
    // Don't touch stickykeys/togglekeys/filterkeys if they're being used,
    // but if they aren't, make sure Windows doesn't harass the user about
    // maybe enabling them while we're running.

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

  System::System( HINSTANCE instance, HWND window, const Cooperation coop, SystemListener* listener ): 
  instance_( instance ), window_( window ), listener_( listener ), coop_( coop )
  {
    assert( listener_ );
  }

  //! Factory function. Use this to construct your instance.
  //! \param  instance  Handle of the host instance.
  //! \param  window    Handle of the host window.
  //! \param  coop      Cooperation mode.
  //! \param  listener  Listener for system events.
  SystemPtr System::create( HINSTANCE instance, HWND window, const Cooperation coop, SystemListener* listener )
  {
    return SystemPtr( new System( instance, window, coop, listener ) );
  }

  void System::initialize()
  {
    if ( !initializing_ )
      return;

    // Validate the passed window handle
    if ( !IsWindow( window_ ) )
      NIL_EXCEPT( "Window handle is invalid" );

    // Store accessibility feature states, and tell Windows not to be annoying
    internals_.store();
    internals_.disableHotkeyHelpers();

    // Init XInput subsystem
    xinput_ = make_unique<XInput>();
    if ( xinput_->initialize() != ExternalModule::Initialization_OK )
      NIL_EXCEPT( "Loading XInput failed" );

    // Create DirectInput instance
    auto hr = DirectInput8Create( instance_, DIRECTINPUT_VERSION, IID_IDirectInput8W, (LPVOID*)&dinput_, nullptr );
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

    initializing_ = false;
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
    UNREFERENCED_PARAMETER( deviceClass );
    UNREFERENCED_PARAMETER( devicePath );

    // IDirectInput8::FindDevice does nothing, so just force a full refresh
    refreshDevices();
  }

  void System::onPnPUnplug( const GUID& deviceClass, const wideString& devicePath )
  {
    UNREFERENCED_PARAMETER( deviceClass );
    UNREFERENCED_PARAMETER( devicePath );

    // IDirectInput8::FindDevice does nothing, so just force a full refresh
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

    auto hidRecord = hidManager_->getRecordByPath( rawPath );

    for ( auto& device : devices_ )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = dynamic_pointer_cast<RawInputDevice>( device );

      if ( util::compareDevicePaths( rawDevice->getRawPath(), rawPath ) )
      {
        deviceConnect( rawDevice );
        return;
      }
    }

    auto device = make_shared<RawInputDevice>( ptr(), getNextID(), handle, rawPath, hidRecord )->ptr();

    if ( isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      deviceConnect( device );

    devices_.push_back( device );
  }

  void System::onRawMouseInput( HANDLE handle,
  const RAWMOUSE& input, const bool sinked )
  {
    UNREFERENCED_PARAMETER( sinked );

    if ( initializing_ || !handle )
      return;

    auto it = mouseMap_.find( handle );
    if ( it != mouseMap_.end() )
      it->second->onRawInput( input );
  }

  void System::onRawKeyboardInput( HANDLE handle,
  const RAWKEYBOARD& input, const bool sinked )
  {
    UNREFERENCED_PARAMETER( sinked );

    if ( initializing_ || !handle )
      return;

    auto it = keyboardMap_.find( handle );
    if ( it != keyboardMap_.end() )
      it->second->onRawInput( input );
  }

  void System::onRawHIDInput( HANDLE handle, const RAWHID& input, const bool sinked )
  {
    UNREFERENCED_PARAMETER( sinked );

    if ( initializing_ || !handle )
      return;

    auto it = controllerMap_.find( handle );
    if ( it != controllerMap_.end() )
      it->second->onRawInput( input );
  }

  void System::onRawRemoval( HANDLE handle )
  {
    for ( auto& device : devices_ )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = dynamic_pointer_cast<RawInputDevice>( device );

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

  void System::mapController( HANDLE handle, RawInputController* controller )
  {
    controllerMap_[handle] = controller;
  }

  void System::unmapController( HANDLE handle )
  {
    controllerMap_.erase( handle );
  }

  void System::initializeDevices()
  {
    xinputIds_.resize( XUSER_MAX_COUNT );
    for ( int i = 0; i < XUSER_MAX_COUNT; i++ )
    {
      xinputIds_[i] = getNextID();
      auto device = make_shared<XInputDevice>( ptr(), xinputIds_[i], i )->ptr();
      devices_.push_back( device );
    }
  }

  void System::refreshDevices()
  {
    // Gather devices that will be ignored in the DI enumerator callback.
    // In practice this means XInput and specific direct HID controllers.
    identifySpecialHandlingDevices();

    // DirectInput

    for ( auto& device : devices_ )
      if ( device->getHandler() == Device::Handler_DirectInput )
      {
        device->saveStatus();
        device->setStatus( Device::Status_Pending );
      }

    auto hr = dinput_->EnumDevices( DI8DEVCLASS_GAMECTRL,
      diDeviceEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not enumerate DirectInput devices" );

    for ( auto& device : devices_ )
      if ( device->getHandler() == Device::Handler_DirectInput
        && device->getSavedStatus() == Device::Status_Connected
        && device->getStatus() == Device::Status_Pending )
        deviceDisconnect( device );

    // XInput

    XINPUT_STATE state;
    for ( auto& device : devices_ )
    {
      if ( device->getHandler() == Device::Handler_XInput )
      {
        auto xDevice = dynamic_pointer_cast<XInputDevice>( device );
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

    for ( auto& identifier : system->specialHandlingDeviceIDs_ )
      if ( instance->guidProduct.Data1 == identifier )
        return DIENUM_CONTINUE;

    for ( auto& device : system->devices_ )
    {
      if ( device->getHandler() != Device::Handler_DirectInput )
        continue;

      auto diDevice = static_cast<DirectInputDevice*>( device.get() );

      if ( diDevice->getInstanceID() == instance->guidInstance )
      {
        if ( device->getSavedStatus() == Device::Status_Disconnected )
          system->deviceConnect( device );
        else
          device->setStatus( Device::Status_Connected );

        return DIENUM_CONTINUE;
      }
    }

    auto device = make_shared<DirectInputDevice>( system->ptr(), system->getNextID(), instance )->ptr();

    if ( system->isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      system->deviceConnect( device );

    system->devices_.push_back( device );

    return DIENUM_CONTINUE;
  }

  void System::deviceConnect( DevicePtr device )
  {
    device->onConnect();
    listener_->onDeviceConnected( device.get() );
  }

  void System::deviceDisconnect( DevicePtr device )
  {
    device->onDisconnect();
    listener_->onDeviceDisconnected( device.get() );
  }

  void System::mouseEnabled( DevicePtr device, MousePtr instance )
  {
    listener_->onMouseEnabled( device.get(), instance.get() );
  }

  void System::mouseDisabled( DevicePtr device, MousePtr instance )
  {
    listener_->onMouseDisabled( device.get(), instance.get() );
  }

  void System::keyboardEnabled( DevicePtr device, KeyboardPtr instance )
  {
    listener_->onKeyboardEnabled( device.get(), instance.get() );
  }

  void System::keyboardDisabled( DevicePtr device, KeyboardPtr instance )
  {
    listener_->onKeyboardDisabled( device.get(), instance.get() );
  }

  void System::controllerEnabled( DevicePtr device, ControllerPtr instance )
  {
    listener_->onControllerEnabled( device.get(), instance.get() );
  }

  void System::controllerDisabled( DevicePtr device, ControllerPtr instance )
  {
    listener_->onControllerDisabled( device.get(), instance.get() );
  }

  void System::identifySpecialHandlingDevices()
  {
    specialHandlingDeviceIDs_.clear();
    for ( auto& hidRecord : hidManager_->getRecords() )
      if ( hidRecord->isXInput() )
        specialHandlingDeviceIDs_.insert( hidRecord->getIdentifier() );
  }

  DeviceList& System::getDevices()
  {
    return devices_;
  }

  bool System::isInitializing() const
  {
    return initializing_;
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
    for ( auto& device : devices_ )
      if ( device->isDisconnectFlagged() )
        deviceDisconnect( device );
      else
        device->update();
  }

  XInput* System::getXInput()
  {
    return xinput_.get();
  }

  System::~System()
  {
    for ( auto& device : devices_ )
      device->disable();

    SAFE_RELEASE( dinput_ );

    // Restore accessiblity features
    internals_.restore();
  }

}

#endif