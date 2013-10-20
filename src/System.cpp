#include "nil.h"
#include "nilUtil.h"

namespace nil {

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

    mMonitor = new PnPMonitor( mInstance );

    enumerate();
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