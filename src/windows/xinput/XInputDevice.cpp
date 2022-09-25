#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  const utf8String c_xinputDefaultName = "XInput Controller";

  const map<int, utf8String> c_xinputModelNameMap = {
    { XINPUT_DEVSUBTYPE_UNKNOWN, c_xinputDefaultName },
    { XINPUT_DEVSUBTYPE_GAMEPAD, "XBOX 360 Gamepad" },
    { XINPUT_DEVSUBTYPE_WHEEL, "XBOX 360 Racing Wheel" },
    { XINPUT_DEVSUBTYPE_ARCADE_STICK, "XBOX 360 Arcade Stick" },
    { XINPUT_DEVSUBTYPE_FLIGHT_STICK, "XBOX 360 Flight Stick" },
    { XINPUT_DEVSUBTYPE_DANCE_PAD, "XBOX 360 Dance Pad" },
    { XINPUT_DEVSUBTYPE_GUITAR, "XBOX 360 Guitar" },
    { XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, "XBOX 360 Alternate Guitar" },
    { XINPUT_DEVSUBTYPE_GUITAR_BASS, "XBOX 360 Bass Guitar" },
    { XINPUT_DEVSUBTYPE_DRUM_KIT, "XBOX 360 Drum Kit" },
    { XINPUT_DEVSUBTYPE_ARCADE_PAD, "XBOX 360 Arcade Pad" }
  };

  XInputDevice::XInputDevice( SystemPtr system, DeviceID id, int xinputID ):
  Device( system, id, Device_Controller ), xinputId_( xinputID )
  {
    memset( &caps_, NULL, sizeof( XINPUT_CAPABILITIES ) );
    name_ = c_xinputDefaultName;
  }

  void XInputDevice::identify()
  {
    if ( system_->getXInput()->funcs_.pfnXInputGetCapabilities( xinputId_, 0, &caps_ ) != ERROR_SUCCESS )
      return;

    auto it = c_xinputModelNameMap.find( caps_.SubType );
    if ( it != c_xinputModelNameMap.end() )
      name_ = it->second;

    identified_ = true;
  }

  DeviceID XInputDevice::getStaticID() const
  {
    // Static ID for XInput devices:
    // 4 bits of handler ID, 28 bits of XInput controller ID (1-4)

    DeviceID id = ( xinputId_ | ( ( Handler_XInput + 1 ) << 28 ) );
    return id;
  }

  void XInputDevice::onConnect()
  {
    if ( !identified_ )
      identify();

    Device::onConnect();
  }

  void XInputDevice::onDisconnect()
  {
    Device::onDisconnect();
  }

  void XInputDevice::setStatus( Status status )
  {
    if ( status == Status_Connected && !identified_ )
      identify();

    Device::setStatus( status );
  }

  Device::Handler XInputDevice::getHandler() const
  {
    return Device::Handler_XInput;
  }

  int XInputDevice::getXInputID() const
  {
    return xinputId_;
  }

  const XINPUT_CAPABILITIES& XInputDevice::getCapabilities() const
  {
    return caps_;
  }

}

#endif