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

  System::System( HINSTANCE instance, HWND window ):
  mWindow( window ), mInstance( instance ),
  mDirectInput( nullptr ), mMonitor( nullptr ), mIDPool( 0 )
  {
    // Make sure the window is a window
    if ( !IsWindow( mWindow ) )
      NIL_EXCEPT( L"Window handle is invalid" );

    // Get the HID device interface GUID
    // HidD_GetHidGuid( &g_HIDInterfaceGUID );

    // Create DirectInput instance
    auto hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8, (LPVOID*)&mDirectInput, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );

    // Initialize our Plug-n-Play monitor
    mMonitor = new PnPMonitor( mInstance, this );

    initializeDevices();
    refreshDevices();
  }

  DeviceID System::getNextID()
  {
    return mIDPool++;
  }

  void System::onPlug( const GUID& deviceClass, const wstring& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::onUnplug( const GUID& deviceClass, const wstring& devicePath )
  {
    // Refresh all currently connected devices,
    // since IDirectInput8::FindDevice doesn't do jack shit
    refreshDevices();
  }

  void System::initializeDevices()
  {
    for ( int i = 0; i < NIL_MAX_XINPUT_DEVICES; i++ )
    {
      mXInputIDs[i] = getNextID();
      auto device = new XInputDevice( mXInputIDs[i], i );
      mDevices.push_back( device );
    }
  }

  void System::refreshDevices()
  {
    for ( Device* device : mDevices )
      if ( device->getType() == Device::Device_DirectInput )
        device->setState( Device::State_Pending );

    HRESULT hr = mDirectInput->EnumDevices( DI8DEVCLASS_ALL,
      diEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not enumerate DirectInput devices" );

    for ( Device* device : mDevices )
      if ( device->getType() == Device::Device_DirectInput
      && device->getState() == Device::State_Pending )
        device->onUnplug();

    XINPUT_STATE state;
    for ( Device* device : mDevices )
    {
      if ( device->getType() == Device::Device_XInput )
      {
        auto xDevice = dynamic_cast<XInputDevice*>( device );
        auto status = XInputGetState( xDevice->getXInputID(), &state );
        if ( xDevice->getState() == Device::State_Connected
        && status == ERROR_DEVICE_NOT_CONNECTED )
          xDevice->onUnplug();
        else if ( xDevice->getState() == Device::State_Disconnected
        && status == ERROR_SUCCESS )
          xDevice->onPlug();
      }
    }
  }

  BOOL CALLBACK System::diEnumCallback( LPCDIDEVICEINSTANCE instance,
  LPVOID referer )
  {
    auto system = static_cast<System*>( referer );

    unsigned long deviceType = GET_DIDEVICE_TYPE( instance->dwDevType );

    if ( deviceType == DI8DEVTYPE_JOYSTICK
    || deviceType == DI8DEVTYPE_GAMEPAD
    || deviceType == DI8DEVTYPE_DRIVING
    || deviceType == DI8DEVTYPE_FLIGHT
    || deviceType == DI8DEVTYPE_1STPERSON )
    {
      for ( Device* device : system->mDevices )
      {
        if ( device->getType() != Device::Device_DirectInput )
          continue;

        auto diDevice = dynamic_cast<DirectInputDevice*>( device );

        if ( diDevice->getInstanceID() == instance->guidInstance
        && device->getState() == Device::State_Pending )
        {
          device->onPlug();
          return DIENUM_CONTINUE;
        }
      }

      Device* device = new DirectInputDevice(
        system->getNextID(),
        instance->guidProduct,
        instance->guidInstance );

      system->mDevices.push_back( device );
    }

    return DIENUM_CONTINUE;
  }

  void System::identifyXInputDevices()
  {
    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    HDEVINFO info = SetupDiGetClassDevsW( &g_HIDInterfaceGUID,
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

          unsigned long identifier = MAKELONG( attributes.VendorID, attributes.ProductID );

          for ( Device* device : mDevices )
          {
            if ( device->getType() == Device::Device_DirectInput )
              continue;
          }

          CloseHandle( handle );
        }
      }

      free( detailData );
    }

    SetupDiDestroyDeviceInfoList( info );
  }

  void System::update()
  {
    mMonitor->update();
  }

  System::~System()
  {
    for ( Device* device : mDevices )
      delete device;

    SAFE_DELETE( mMonitor );
    SAFE_RELEASE( mDirectInput );
  }

}