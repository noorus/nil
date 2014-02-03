#include "nil.h"
#include "nilUtil.h"
#include "nilLogitech.h"

namespace nil {

  System::System( HINSTANCE instance, HWND window ): mWindow( window ),
  mInstance( instance ), mDirectInput( nullptr ), mMonitor( nullptr ),
  mIDPool( 0 ), mInitializing( true ), mHIDManager( nullptr ),
  mLogitechGKeys( nullptr )
  {
    // Validate the passes window handle
    if ( !IsWindow( mWindow ) )
      NIL_EXCEPT( L"Window handle is invalid" );

    mLogitechGKeys = new Logitech::GKeySDK();

    // Create DirectInput instance
    auto hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8W, (LPVOID*)&mDirectInput, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );

    // Initialize our Plug-n-Play monitor
    mMonitor = new PnPMonitor( mInstance, this );

    // Initialize our HID manager
    mHIDManager = new HIDManager();

    // Register the HID manager and ourselves as PnP event listeners
    mMonitor->registerPnPListener( mHIDManager );
    mMonitor->registerPnPListener( this );

    // Register ourselves as a raw event listener
    mMonitor->registerRawListener( this );
    
    // Fetch initial devices
    initializeDevices();
    refreshDevices();

    // Update the monitor once, to receive initial Raw devices
    mMonitor->update();

    mInitializing = false;
  }

  DeviceID System::getNextID()
  {
    return mIDPool++;
  }

  void System::onPnPPlug( const GUID& deviceClass, const String& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onPnPUnplug( const GUID& deviceClass, const String& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onRawArrival( HANDLE handle )
  {
    UINT pathLength = 0;

    if ( GetRawInputDeviceInfoW( handle, RIDI_DEVICENAME, NULL, &pathLength ) )
      NIL_EXCEPT_WINAPI( L"GetRawInputDeviceInfoW failed" );

    String rawPath( pathLength, '\0' );

    GetRawInputDeviceInfoW( handle, RIDI_DEVICENAME, &rawPath[0], &pathLength );
    rawPath.resize( rawPath.length() - 1 );

    for ( auto device : mDevices )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = static_cast<RawInputDevice*>( device );

      if ( !_wcsicmp( rawDevice->getRawPath().c_str(), rawPath.c_str() ) )
      {
        rawDevice->onConnect();
        return;
      }
    }

    auto device = new RawInputDevice( this, getNextID(), handle, rawPath );

    if ( isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      device->onConnect();

    mDevices.push_back( device );
  }

  void System::onRawMouseInput( HANDLE handle, const RAWMOUSE& input )
  {
    if ( mInitializing || !handle )
      return;

    auto it = mMouseMapping.find( handle );
    if ( it != mMouseMapping.end() )
      it->second->onRawInput( input );
  }

  void System::onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input )
  {
    if ( mInitializing || !handle )
      return;

    auto it = mKeyboardMapping.find( handle );
    if ( it != mKeyboardMapping.end() )
      it->second->onRawInput( input );
  }

  void System::onRawRemoval( HANDLE handle )
  {
    for ( auto device : mDevices )
    {
      if ( device->getHandler() != Device::Handler_RawInput )
        continue;

      auto rawDevice = static_cast<RawInputDevice*>( device );

      if ( rawDevice->getRawHandle() == handle )
      {
        rawDevice->onDisconnect();
        return;
      }
    }
  }

  void System::mapMouse( HANDLE handle, RawInputMouse* mouse )
  {
    mMouseMapping[handle] = mouse;
  }

  void System::unmapMouse( HANDLE handle )
  {
    mMouseMapping.erase( handle );
  }

  void System::mapKeyboard( HANDLE handle, RawInputKeyboard* keyboard )
  {
    mKeyboardMapping[handle] = keyboard;
  }

  void System::unmapKeyboard( HANDLE handle )
  {
    mKeyboardMapping.erase( handle );
  }

  void System::initializeDevices()
  {
    mXInputIDs.resize( XUSER_MAX_COUNT );
    for ( int i = 0; i < XUSER_MAX_COUNT; i++ )
    {
      mXInputIDs[i] = getNextID();
      auto device = new XInputDevice( this, mXInputIDs[i], i );
      mDevices.push_back( device );
    }
  }

  void System::refreshDevices()
  {
    identifyXInputDevices();

    for ( Device* device : mDevices )
      if ( device->getHandler() == Device::Handler_DirectInput )
      {
        device->saveStatus();
        device->setStatus( Device::Status_Pending );
      }

    auto hr = mDirectInput->EnumDevices( DI8DEVCLASS_GAMECTRL,
      diDeviceEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not enumerate DirectInput devices!" );

    for ( Device* device : mDevices )
      if ( device->getHandler() == Device::Handler_DirectInput
        && device->getSavedStatus() == Device::Status_Connected
        && device->getStatus() == Device::Status_Pending )
        device->onDisconnect();

    XINPUT_STATE state;
    for ( Device* device : mDevices )
    {
      if ( device->getHandler() == Device::Handler_XInput )
      {
        auto xDevice = static_cast<XInputDevice*>( device );
        auto status = XInputGetState( xDevice->getXInputID(), &state );
        if ( status == ERROR_DEVICE_NOT_CONNECTED )
        {
          if ( xDevice->getStatus() == Device::Status_Connected )
            xDevice->onDisconnect();
          else if ( xDevice->getStatus() == Device::Status_Pending )
            xDevice->setStatus( Device::Status_Disconnected );
        }
        else if ( status == ERROR_SUCCESS )
        {
          if ( xDevice->getStatus() == Device::Status_Disconnected )
            xDevice->onConnect();
          else if ( xDevice->getStatus() == Device::Status_Pending )
            xDevice->setStatus( Device::Status_Connected );
        }
        else
          NIL_EXCEPT( L"XInputGetState failed" );
      }
    }
  }

  BOOL CALLBACK System::diDeviceEnumCallback( LPCDIDEVICEINSTANCEW instance,
  LPVOID referer )
  {
    auto system = reinterpret_cast<System*>( referer );

    for ( auto identifier : system->mXInputDeviceIDs )
      if ( instance->guidProduct.Data1 == identifier )
        return DIENUM_CONTINUE;

    for ( auto device : system->mDevices )
    {
      if ( device->getHandler() != Device::Handler_DirectInput )
        continue;

      auto diDevice = static_cast<DirectInputDevice*>( device );

      if ( diDevice->getInstanceID() == instance->guidInstance )
      {
        if ( device->getSavedStatus() == Device::Status_Disconnected )
          device->onConnect();
        else
          device->setStatus( Device::Status_Connected );

        return DIENUM_CONTINUE;
      }
    }

    Device* device = new DirectInputDevice( system, system->getNextID(), instance );

    if ( system->isInitializing() )
      device->setStatus( Device::Status_Connected );
    else
      device->onConnect();

    system->mDevices.push_back( device );

    return DIENUM_CONTINUE;
  }

  void System::identifyXInputDevices()
  {
    mXInputDeviceIDs.clear();
    for ( auto hidRecord : mHIDManager->getRecords() )
      if ( hidRecord->isXInput() )
        mXInputDeviceIDs.push_back( hidRecord->getIdentifier() );
  }

  const bool System::isInitializing()
  {
    return mInitializing;
  }

  void System::update()
  {
    // Run PnP & raw events if there are any
    mMonitor->update();

    // First make sure that we disconnect failed devices
    for ( Device* device : mDevices )
      if ( device->isDisconnectFlagged() )
        device->onDisconnect();
      else
        device->update();

    mLogitechGKeys->update();
  }

  System::~System()
  {
    for ( Device* device : mDevices )
      delete device;

    SAFE_DELETE( mHIDManager );
    SAFE_DELETE( mMonitor );
    SAFE_RELEASE( mDirectInput );
    SAFE_DELETE( mLogitechGKeys );
  }

}