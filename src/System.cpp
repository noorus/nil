#include "nil.h"
#include "nilUtil.h"
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>
#include <wbemidl.h>
#include <oleauto.h>
#include <setupapi.h>

extern "C" {
# include <hidsdi.h>
};

#pragma comment( lib, "hid.lib" )
#pragma comment( lib, "setupapi.lib" )

namespace nil {

  const long cMaxXInputDevices = 4;

  System::System( HINSTANCE instance, HWND window ): mWindow( window ),
  mInstance( instance ), mDirectInput( nullptr ), mMonitor( nullptr ),
  mIDPool( 0 ), mInitializing( true )
  {
    // Make sure the window is a window
    if ( !IsWindow( mWindow ) )
      NIL_EXCEPT( L"Window handle is invalid" );

    // Get the HID device interface GUID
    // HidD_GetHidGuid( &g_HIDInterfaceGUID );

    // Create DirectInput instance
    auto hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8W, (LPVOID*)&mDirectInput, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );

    // Initialize our Plug-n-Play monitor
    mMonitor = new PnPMonitor( mInstance, this );

    initializeDevices();
    refreshDevices();

    mInitializing = false;

    wprintf_s( L"Initial devices:\r\n" );
    for ( Device* device : mDevices )
      if ( device->getStatus() == Device::Status_Connected )
      {
        wprintf_s( L"Initial: (%d) %s (%s %s)\r\n",
        device->getID(),
        device->getName().c_str(),
        device->getHandler() == Device::Handler_XInput ? L"XInput" : L"DirectInput",
        device->getType() == Device::Device_Mouse ? L"Mouse" : device->getType() == Device::Device_Keyboard ? L"Keyboard" : L"Controller"
        );
        if ( device->getHandler() == Device::Handler_DirectInput )
        {
          DirectInputDevice* diDevice = dynamic_cast<DirectInputDevice*>( device );
          wprintf_s( L"  product: %s\r\n  instance: %s\r\n",
            guidToStr( diDevice->getProductID() ).c_str(),
            guidToStr( diDevice->getInstanceID() ).c_str()
          );
        }
      }
    wprintf_s( L"Running...\r\n" );
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
    mXInputIDs.resize( cMaxXInputDevices );
    for ( int i = 0; i < cMaxXInputDevices; i++ )
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
      diEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not enumerate DirectInput devices" );

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

  BOOL CALLBACK System::diEnumCallback( LPCDIDEVICEINSTANCEW instance,
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

    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    auto info = SetupDiGetClassDevsW( &g_HIDInterfaceGUID,
      NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( info == INVALID_HANDLE_VALUE )
      NIL_EXCEPT_WINAPI( L"SetupDiGetClassDevsW failed" );

    unsigned long xinputIndex = 0;

    for ( unsigned long i = 0; SetupDiEnumDeviceInterfaces(
    info, 0, &g_HIDInterfaceGUID, i, &interfaceData ); i++ )
    {
      unsigned long length = 0;
      SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, NULL, 0, &length, NULL );

      if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        NIL_EXCEPT_WINAPI( L"SetupDiGetDeviceInterfaceDetailW failed" );

      auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc( length );
      detailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA_W );

      if ( SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, detailData, length, NULL, NULL ) )
      {
        if ( wcsstr( detailData->DevicePath, L"ig_" )
        || wcsstr( detailData->DevicePath, L"IG_" ) )
        {
          auto handle = CreateFileW( detailData->DevicePath, 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

          if ( handle == INVALID_HANDLE_VALUE )
            NIL_EXCEPT_WINAPI( L"CreateFileW failed" );

          HIDD_ATTRIBUTES attributes = { sizeof( HIDD_ATTRIBUTES ) };

          if ( !HidD_GetAttributes( handle, &attributes ) )
            NIL_EXCEPT( L"HidD_GetAttributes failed" );

          unsigned long identifier = MAKELONG(
            attributes.VendorID, attributes.ProductID );

          mXInputDeviceIDs.push_back( identifier );

          CloseHandle( handle );
        }
      }

      free( detailData );
    }

    SetupDiDestroyDeviceInfoList( info );
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

    // TODO update devices!
  }

  System::~System()
  {
    for ( Device* device : mDevices )
      delete device;

    SAFE_DELETE( mMonitor );
    SAFE_RELEASE( mDirectInput );
  }

}