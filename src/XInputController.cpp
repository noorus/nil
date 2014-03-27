#include "nil.h"
#include "nilUtil.h"

namespace Nil {

#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
  const long cMaxXInputTypes = 11;
  static std::pair<int,Controller::Type> cXInputTypes[cMaxXInputTypes] = {
    std::make_pair( XINPUT_DEVSUBTYPE_UNKNOWN, Controller::Controller_Unknown ),
    std::make_pair( XINPUT_DEVSUBTYPE_GAMEPAD, Controller::Controller_Gamepad ),
    std::make_pair( XINPUT_DEVSUBTYPE_WHEEL, Controller::Controller_Driving ),
    std::make_pair( XINPUT_DEVSUBTYPE_ARCADE_STICK, Controller::Controller_Joystick ),
    std::make_pair( XINPUT_DEVSUBTYPE_FLIGHT_STICK, Controller::Controller_Flight ),
    std::make_pair( XINPUT_DEVSUBTYPE_DANCE_PAD, Controller::Controller_DancePad ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR, Controller::Controller_Guitar ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, Controller::Controller_Guitar ),
    std::make_pair( XINPUT_DEVSUBTYPE_GUITAR_BASS, Controller::Controller_Bass ),
    std::make_pair( XINPUT_DEVSUBTYPE_DRUM_KIT, Controller::Controller_Drumkit ),
    std::make_pair( XINPUT_DEVSUBTYPE_ARCADE_PAD, Controller::Controller_ArcadePad )
  };
#else
  const long cMaxXInputTypes = 1;
  static std::pair<int,Controller::Type> cXInputTypes[cMaxXInputTypes] = {
    std::make_pair( XINPUT_DEVSUBTYPE_GAMEPAD, Controller::Controller_Gamepad )
  };
#endif

  XInputController::XInputController( XInputDevice* device ):
  Controller( device->getSystem(), device ), mLastPacket( 0 )
  {
    for ( int i = 0; i < cMaxXInputTypes; i++ )
      if ( cXInputTypes[i].first == device->getCapabilities().SubType )
        mType = cXInputTypes[i].second;

    mState.mPOVs.resize( 1 );
    mState.mButtons.resize( 12 );
    mState.mAxes.resize( 6 );
  }

  Real XInputController::filterLeftThumbAxis( int val )
  {
    if ( val < 0 )
    {
      if ( val > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
        return NIL_REAL_ZERO;
      val += XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
      Real ret = (Real)val / (Real)( 32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    else if ( val > 0 )
    {
      if ( val < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE )
        return NIL_REAL_ZERO;
      val -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
      Real ret = (Real)val / (Real)( 32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    else
      return NIL_REAL_ZERO;
  }

  Real XInputController::filterRightThumbAxis( int val )
  {
    // There's a problem with the right thumbstick's axes often not reaching
    // their maximum. So we adjust the range upward slightly to make up for it.
    // It's OK to lose a tiny bit of low-range precision in order to do that.
    // However, we'll try to stay as neutral as possible, and leave further
    // filtering to the end-user application.
 
    static int adjustedDeadzone = (int)( (double)XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * 0.8 );

    if ( val < 0 )
    {
      if ( val > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE )
        return NIL_REAL_ZERO;
      val += adjustedDeadzone;
      Real ret = (Real)val / (Real)( 32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    else if ( val > 0 )
    {
      if ( val < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE )
        return NIL_REAL_ZERO;
      val -= adjustedDeadzone;
      Real ret = (Real)val / (Real)( 32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    else
      return NIL_REAL_ZERO;
  }

  Real XInputController::filterTrigger( int val )
  {
    if ( val < XINPUT_GAMEPAD_TRIGGER_THRESHOLD )
      return NIL_REAL_ZERO;
    val -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    Real ret = (Real)val / (Real)( 255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
    return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
  }

  void XInputController::update()
  {
    XInputDevice* xDevice = dynamic_cast<XInputDevice*>( mDevice );

    DWORD ret = XInputGetState( xDevice->getXInputID(), &mXInputState );
    if ( ret == ERROR_DEVICE_NOT_CONNECTED )
    {
      xDevice->flagDisconnected();
      return;
    }
    else if ( ret != ERROR_SUCCESS )
      NIL_EXCEPT( L"XInputGetState failed!" );

    if ( mXInputState.dwPacketNumber == mLastPacket )
      return;

    mLastPacket = mXInputState.dwPacketNumber;

    ControllerState lastState = mState;

    // Buttons
    for ( size_t i = 0; i < mState.mButtons.size(); i++ )
      mState.mButtons[i].pushed = ( ( mXInputState.Gamepad.wButtons & ( 1 << ( i + 4 ) ) ) != 0 );

    // Axes
    mState.mAxes[0].absolute = filterLeftThumbAxis( mXInputState.Gamepad.sThumbLX );
    mState.mAxes[1].absolute = filterLeftThumbAxis( mXInputState.Gamepad.sThumbLY );
    mState.mAxes[2].absolute = filterRightThumbAxis( mXInputState.Gamepad.sThumbRX );
    mState.mAxes[3].absolute = filterRightThumbAxis( mXInputState.Gamepad.sThumbRY );
    mState.mAxes[4].absolute = filterTrigger( mXInputState.Gamepad.bLeftTrigger );
    mState.mAxes[5].absolute = filterTrigger( mXInputState.Gamepad.bRightTrigger );

    // POV
    POVDirection& xPov = mState.mPOVs[0].direction;
    xPov = POV::Centered;
    if ( mXInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP )
      xPov |= POV::North;
    else if ( mXInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN )
      xPov |= POV::South;
    if ( mXInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT )
      xPov |= POV::West;
    else if ( mXInputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT )
      xPov |= POV::East;

    fireChanges( lastState );
  }

  XInputController::~XInputController()
  {
    //
  }

}