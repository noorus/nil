#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  const map<int, Controller::Type> c_xinputControllerTypeMap = {
    { XINPUT_DEVSUBTYPE_UNKNOWN, Controller::Controller_Unknown },
    { XINPUT_DEVSUBTYPE_GAMEPAD, Controller::Controller_Gamepad },
    { XINPUT_DEVSUBTYPE_WHEEL, Controller::Controller_Driving },
    { XINPUT_DEVSUBTYPE_ARCADE_STICK, Controller::Controller_Joystick },
    { XINPUT_DEVSUBTYPE_FLIGHT_STICK, Controller::Controller_Flight },
    { XINPUT_DEVSUBTYPE_DANCE_PAD, Controller::Controller_DancePad },
    { XINPUT_DEVSUBTYPE_GUITAR, Controller::Controller_Guitar },
    { XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, Controller::Controller_Guitar },
    { XINPUT_DEVSUBTYPE_GUITAR_BASS, Controller::Controller_Bass },
    { XINPUT_DEVSUBTYPE_DRUM_KIT, Controller::Controller_Drumkit },
    { XINPUT_DEVSUBTYPE_ARCADE_PAD, Controller::Controller_ArcadePad }
  };

  XInputController::XInputController( XInputDevicePtr device ):
  Controller( device->getSystem()->ptr(), device )
  {
    type_ = c_xinputControllerTypeMap.at( device->getCapabilities().SubType );

    state_.povs.resize( 1 );
    state_.buttons.resize( 10 );
    state_.axes.resize( 6 );
  }

  Real XInputController::filterThumbAxis( int val, int deadzone )
  {
    if ( val < 0 )
    {
      if ( val > -deadzone )
        return NIL_REAL_ZERO;
      val += deadzone;
      auto ret = static_cast<Real>( val ) / static_cast<Real>( 32767 - deadzone );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    if ( val > 0 )
    {
      if ( val < deadzone )
        return NIL_REAL_ZERO;
      val -= deadzone;
      auto ret = static_cast<Real>( val ) / static_cast<Real>( 32767 - deadzone );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    return NIL_REAL_ZERO;
  }

  Real XInputController::filterTrigger( int val )
  {
    if ( val < XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
      return NIL_REAL_ZERO;
    val -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    auto ret = static_cast<Real>( val ) / static_cast<Real>( 255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
    return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
  }

  void XInputController::update()
  {
    auto xDevice = dynamic_pointer_cast<XInputDevice>( device_ );

    DWORD ret = system_->getXInput()->funcs_.pfnXInputGetState( xDevice->getXInputID(), &xinputState_ );
    if ( ret == ERROR_DEVICE_NOT_CONNECTED )
    {
      xDevice->flagDisconnected();
      return;
    }

    if ( ret != ERROR_SUCCESS )
      NIL_EXCEPT( "XInputGetState failed" );

    if ( xinputState_.dwPacketNumber == lastPacket_ )
      return;

    lastPacket_ = xinputState_.dwPacketNumber;

    ControllerState lastState = state_;

    // Buttons - skip 0x400 & 0x800 as they are undefined in the API
    for ( size_t i = 0; i < 6; i++ )
      state_.buttons[i].pushed = ( ( xinputState_.Gamepad.wButtons & ( 1 << ( i + 4 ) ) ) != 0 );
    for ( size_t i = 6; i < 10; i++ )
      state_.buttons[i].pushed = ( ( xinputState_.Gamepad.wButtons & ( 1 << ( i + 6 ) ) ) != 0 );

    // Axes
    state_.axes[0].absolute = filterThumbAxis( xinputState_.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
    state_.axes[1].absolute = filterThumbAxis( xinputState_.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
    state_.axes[2].absolute = filterThumbAxis( xinputState_.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
    state_.axes[3].absolute = filterThumbAxis( xinputState_.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
    state_.axes[4].absolute = filterTrigger( xinputState_.Gamepad.bLeftTrigger );
    state_.axes[5].absolute = filterTrigger( xinputState_.Gamepad.bRightTrigger );

    // POV
    POVDirection& xPov = state_.povs[0].direction;
    xPov = POV::Centered;
    if ( xinputState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
      xPov |= POV::North;
    else if ( xinputState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
      xPov |= POV::South;
    if ( xinputState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
      xPov |= POV::West;
    else if ( xinputState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
      xPov |= POV::East;

    fireChanges( lastState );
  }

  XInputController::~XInputController()
  {
  }

}

#endif