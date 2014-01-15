#include "nil.h"
#include "nilUtil.h"

namespace nil {

  const String cXInputDefaultName = L"XInput Controller";

#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
  const long cMaxXInputModels = 11;
  static std::pair<int,String> cXInputModels[cMaxXInputModels] = {
    std::make_pair( XINPUT_DEVSUBTYPE_UNKNOWN, cXInputDefaultName ),
    std::make_pair( XINPUT_DEVSUBTYPE_GAMEPAD, L"XBOX 360 Gamepad" ),
    std::make_pair( XINPUT_DEVSUBTYPE_WHEEL, L"XBOX 360 Racing Wheel" ),
    std::make_pair( XINPUT_DEVSUBTYPE_ARCADE_STICK, L"XBOX 360 Arcade Stick" ),
    std::make_pair( XINPUT_DEVSUBTYPE_FLIGHT_STICK, L"XBOX 360 Flight Stick" ),
    std::make_pair( XINPUT_DEVSUBTYPE_DANCE_PAD, L"XBOX 360 Dance Pad" ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR, L"XBOX 360 Guitar" ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, L"XBOX 360 Alternate Guitar" ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR_BASS, L"XBOX 360 Bass Guitar" ),
    std::make_pair( XINPUT_DEVSUBTYPE_DRUM_KIT, L"XBOX 360 Drum Kit" ),
    std::make_pair( XINPUT_DEVSUBTYPE_ARCADE_PAD, L"XBOX 360 Arcade Pad" )
  };
#else
  const long cMaxXInputModels = 1;
  static std::pair<int,String> cXInputModels[cMaxXInputModels] = {
    std::make_pair( XINPUT_DEVSUBTYPE_GAMEPAD, L"XBOX 360 Gamepad" )
  };
#endif

  XInputDevice::XInputDevice( System* system, DeviceID id, int xinputID ):
  Device( system, id, Device_Controller ), mXInputID( xinputID ),
  mIdentified( false )
  {
    memset( &mCapabilities, NULL, sizeof( XINPUT_CAPABILITIES ) );
    mName = cXInputDefaultName;
  }

  void XInputDevice::identify()
  {
    if ( XInputGetCapabilities( mXInputID, 0, &mCapabilities ) != ERROR_SUCCESS )
      return;

    for ( int i = 0; i < cMaxXInputModels; i++ ) {
      if ( cXInputModels[i].first == mCapabilities.SubType )
        mName = cXInputModels[i].second;
    }

    mIdentified = true;
  }

  void XInputDevice::onConnect()
  {
    if ( !mIdentified )
      identify();

    Device::onConnect();
  }

  void XInputDevice::onDisconnect()
  {
    Device::onDisconnect();
  }

  void XInputDevice::setStatus( Status status )
  {
    if ( status == Status_Connected && !mIdentified )
      identify();
    Device::setStatus( status );
  }

  const Device::Handler XInputDevice::getHandler()
  {
    return Device::Handler_XInput;
  }

  const int XInputDevice::getXInputID()
  {
    return mXInputID;
  }

  const XINPUT_CAPABILITIES& XInputDevice::getCapabilities()
  {
    return mCapabilities;
  }

}