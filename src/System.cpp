#include "nil.h"
#include "nilUtil.h"
#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>
#include <hidclass.h>
#include <wbemidl.h>
#include <oleauto.h>

namespace nil {

  const unsigned long cDetailDataBufferSize = 4096;
  const int cMaxXInputDevices = 4;
  const int cCOMQueryTimeout = 5000;

  System::System( HINSTANCE instance, HWND window ):
  mWindow( window ), mInstance( instance ),
  mDirectInput( nullptr ), mMonitor( nullptr ), mInitializedCOM( false )
  {
    // Make sure the window is a window
    if ( !IsWindow( mWindow ) )
      NIL_EXCEPT( L"Window handle is invalid" );

    // Initialize COM, in case it isn't already
    auto hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY );
    if ( FAILED( hr ) )
      NIL_EXCEPT( L"COM initialization failed" );
    mInitializedCOM = true;

    // Create DirectInput instance
    hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8, (LPVOID*)&mDirectInput, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );

    // Initialize our Plug-n-Play monitor
    mMonitor = new PnPMonitor( mInstance, this );

    // Enumerate currently connected devices
    refreshDevices();
  }

  bool System::resolveDevice( const wstring& devicePath )
  {
    SP_DEVINFO_DATA deviceData = { sizeof( SP_DEVINFO_DATA ) };
    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    HDEVINFO info = SetupDiGetClassDevsW( &nil::g_HIDInterfaceGUID,
      NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES );

    if ( info == INVALID_HANDLE_VALUE )
      NIL_EXCEPT_WINAPI( L"SetupDiGetClassDevsW failed" );

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
    GUID resolved = { 0 };
    // Wtf? I think FindDevice is totally borked, keeps returning DIERR_DEVICENOTREG...
    HRESULT hr = mDirectInput->FindDevice( deviceClass, devicePath.c_str(), &resolved );
    wprintf_s( L"mDirectInput->FindDevice() returned 0x%X\r\n", hr );
    wprintf_s( L"Resolved is %s\r\n", guidToStr( resolved ).c_str() );
  }

  void System::onUnplug( const GUID& deviceClass, const wstring& devicePath )
  {
    wprintf_s( L"Unplugged:\r\n  Class: %s\r\n  Path: %s\r\n", guidToStr( deviceClass ).c_str(), devicePath.c_str() );
  }

  void System::refreshDevices()
  {
    mEntries.clear();

    HRESULT hr = mDirectInput->EnumDevices( DI8DEVCLASS_ALL,
      diEnumCallback, this, DIEDFL_ATTACHEDONLY );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not enumerate DirectInput devices" );

    for ( int i = 0; i < cMaxXInputDevices; i++ )
    {
      XINPUT_STATE state;
      if ( XInputGetState( i, &state ) == ERROR_SUCCESS )
      {
        // At least one XInput device is connected; Identify them
        identifyXInputDevices();
        break;
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
      DeviceEntry* device = new DeviceEntry(
        DeviceEntry::Device_DirectInput,
        instance->guidProduct,
        instance->guidInstance );

      device->setState( DeviceEntry::State_Current );

      system->mEntries.push_back( device );
    }

    return DIENUM_CONTINUE;
  }

  void System::identifyXInputDevices()
  {
    COMString nameSpace( L"\\\\.\\root\\cimv2" );
    COMString className( L"Win32_PNPEntity" );
    COMString classValueDeviceID( L"DeviceID" );

    IWbemLocator* locator;
    auto hr = CoCreateInstance( __uuidof( WbemLocator ), NULL,
      CLSCTX_INPROC_SERVER, __uuidof( IWbemLocator ),
      (LPVOID*)&locator );
    if ( FAILED( hr ) || !locator )
      NIL_EXCEPT( L"CoCreateInstance failed" );

    IWbemServices* services;
    hr = locator->ConnectServer( nameSpace, NULL, NULL, 0L, 0L,
      NULL, NULL, &services );
    if ( FAILED( hr ) || !services )
      NIL_EXCEPT( L"ConnectServer failed" );

    hr = CoSetProxyBlanket( services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
      NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
      EOAC_NONE );
    if ( FAILED( hr ) )
      NIL_EXCEPT( L"CoSetProxyBlanket failed" );

    IEnumWbemClassObject* enumerator;
    hr = services->CreateInstanceEnum( className, 0, NULL, &enumerator );
    if ( FAILED( hr ) || !enumerator )
      NIL_EXCEPT( L"CreateInstanceEnum failed" );

    unsigned long xinputIndex = 0;
    IWbemClassObject* devices[8] = { nullptr };

    while ( true )
    {
      unsigned long count = 0;

      hr = enumerator->Next( cCOMQueryTimeout, 8, devices, &count );
      if ( FAILED( hr ) || count == 0 )
        break;

      for ( unsigned long i = 0; i < count; i++ )
      {
        VARIANT var;
        hr = devices[i]->Get( classValueDeviceID, 0L, &var, NULL, NULL );
        if ( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
        {
          if ( wcsstr( var.bstrVal, L"IG_" ) )
          {
            unsigned long vid;
            auto vidptr = wcsstr( var.bstrVal, L"VID_" );
            if ( !vidptr || swscanf_s( vidptr, L"VID_%4X", &vid ) != 1 )
              vid = 0;

            unsigned long pid;
            auto pidptr = wcsstr( var.bstrVal, L"PID_" );
            if ( !pidptr || swscanf_s( pidptr, L"PID_%4X", &pid ) != 1 )
              pid = 0;

            unsigned long identifier = MAKELONG( vid, pid );

            for ( DeviceEntry* entry : mEntries )
            {
              if ( entry->getType() == DeviceEntry::Device_XInput )
                continue;
              if ( entry->getProductID().Data1 == identifier )
              {
                entry->makeXInput( xinputIndex );
                xinputIndex++;
              }
            }
          }
        }

        SAFE_RELEASE( devices[i] );
      }
    }
    
    SAFE_RELEASE( enumerator );
    SAFE_RELEASE( services );
    SAFE_RELEASE( locator );
  }

  void System::update()
  {
    mMonitor->update();
  }

  System::~System()
  {
    for ( DeviceEntry* entry : mEntries )
      delete entry;

    SAFE_DELETE( mMonitor );
    SAFE_RELEASE( mDirectInput );

    if ( mInitializedCOM )
      CoUninitialize();
  }

}