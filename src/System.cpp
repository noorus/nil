#include "nil.h"
#include "nilUtil.h"
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>
#include <hidclass.h>

namespace nil {

  const unsigned long cDetailDataBufferSize = 4096;

  System::System( HWND window ): mDirectInput( nullptr ),
  mWindow( window ), mInstance( 0 ), mMonitor( nullptr )
  {
    if ( !IsWindow( mWindow ) )
      EXCEPT( L"Passed window handle is invalid" );

    mInstance = GetModuleHandleW( nullptr );

    HRESULT hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8, (LPVOID*)&mDirectInput, NULL );
    if ( FAILED( hr ) )
      EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );

    mMonitor = new PnPMonitor( mInstance, this );

    enumerate();
  }

  bool System::resolveDevice( const wstring& devicePath )
  {
    SP_DEVINFO_DATA deviceData = { sizeof( SP_DEVINFO_DATA ) };
    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    HDEVINFO info = SetupDiGetClassDevsW( &nil::g_HIDInterfaceGUID,
      NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES );

    if ( info == INVALID_HANDLE_VALUE )
      EXCEPT_WINAPI( L"SetupDiGetClassDevsW failed" );

    auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc( cDetailDataBufferSize );

    for ( unsigned long i = 0; SetupDiEnumDeviceInfo( info, i, &deviceData ); i++ )
    {
      WCHAR instance[512] = { 0 };
      SetupDiGetDeviceInstanceIdW( info, &deviceData, instance, 512, NULL );

      if ( !SetupDiEnumDeviceInterfaces( info, 0, &nil::g_HIDInterfaceGUID, i, &interfaceData ) )
        continue;

      memset( detailData, 0, cDetailDataBufferSize );
      detailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA_W );

      if ( SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, detailData, cDetailDataBufferSize, NULL, NULL ) )
      {
        if ( boost::iequals( detailData->DevicePath, devicePath ) )
        {
          wprintf_s( L"  instance: %s\r\n", instance );
          free( detailData );
          SetupDiDestroyDeviceInfoList( info );
          return true;
        }
      }
    }
    free( detailData );
    SetupDiDestroyDeviceInfoList( info );
    return false;
  }

  void System::onPlug( const GUID& deviceClass, const wstring& devicePath )
  {
    wprintf_s( L"Plugged:\r\n  Class: %s\r\n  Path: %s\r\n", guidToStr( deviceClass ).c_str(), devicePath.c_str() );
    resolveDevice( devicePath );
  }

  void System::onUnplug( const GUID& deviceClass, const wstring& devicePath )
  {
    wprintf_s( L"Unplugged:\r\n  Class: %s\r\n  Path: %s\r\n", guidToStr( deviceClass ).c_str(), devicePath.c_str() );
    resolveDevice( devicePath );
  }

  void System::enumerate()
  {
    mDevices.clear();

    HRESULT hr = mDirectInput->EnumDevices( DI8DEVCLASS_ALL,
      diEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      EXCEPT_DINPUT( hr, L"Could not enumerate DirectInput devices" );
  }

  BOOL CALLBACK System::diEnumCallback( LPCDIDEVICEINSTANCE instance,
  LPVOID referer )
  {
    System* system = static_cast<System*>( referer );

    unsigned long deviceType = GET_DIDEVICE_TYPE( instance->dwDevType );

    if ( deviceType == DI8DEVTYPE_JOYSTICK
    || deviceType == DI8DEVTYPE_GAMEPAD
    || deviceType == DI8DEVTYPE_DRIVING
    || deviceType == DI8DEVTYPE_FLIGHT
    || deviceType == DI8DEVTYPE_1STPERSON )
    {
      Device* device = new Device( instance->guidProduct, instance->guidInstance );
      system->mDevices.push_back( device );
    }

    return DIENUM_CONTINUE;
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