#include "nil.h"
#include "nilUtil.h"
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>
#include <wbemidl.h>
#include <oleauto.h>

namespace nil {

  System::System( HINSTANCE instance, HWND window ): mWindow( window ),
  mInstance( instance ), mDirectInput( nullptr ), mMonitor( nullptr ),
  mIDPool( 0 ), mInitializing( true ), mHIDManager( nullptr )
  {
    // Make sure the window is a window
    if ( !IsWindow( mWindow ) )
      NIL_EXCEPT( L"Window handle is invalid" );

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
    mMonitor->registerListener( mHIDManager );
    mMonitor->registerListener( this );
    
    // Fetch initial devices
    initializeDevices();
    refreshDevices();

    mInitializing = false;
  }

  DeviceID System::getNextID()
  {
    return mIDPool++;
  }

  void System::onPlug( const GUID& deviceClass, const String& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onUnplug( const GUID& deviceClass, const String& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
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

    auto hr = mDirectInput->EnumDevices( DI8DEVCLASS_ALL,
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
        auto xDevice = dynamic_cast<XInputDevice*>( device );
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
    auto system = static_cast<System*>( referer );

    for ( uint32_t identifier : system->mXInputDeviceIDs )
      if ( instance->guidProduct.Data1 == identifier )
        return DIENUM_CONTINUE;

    for ( Device* device : system->mDevices )
    {
      if ( device->getHandler() != Device::Handler_DirectInput )
        continue;

      auto diDevice = dynamic_cast<DirectInputDevice*>( device );

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
    for ( auto device : mHIDManager->getDevices() )
      if ( device->isXInput() )
        mXInputDeviceIDs.push_back( device->getIdentifier() );
  }

  const bool System::isInitializing()
  {
    return mInitializing;
  }

  void System::update()
  {
    // Run PnP events if there are any
    mMonitor->update();

    // First make sure that we disconnect failed devices
    for ( Device* device : mDevices )
      if ( device->isDisconnectFlagged() )
        device->onDisconnect();
      else
        device->update();
  }

  System::~System()
  {
    for ( Device* device : mDevices )
      delete device;

    SAFE_DELETE( mHIDManager );
    SAFE_DELETE( mMonitor );
    SAFE_RELEASE( mDirectInput );
  }

}