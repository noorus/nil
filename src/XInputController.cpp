#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

namespace nil {

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
    mState.mButtons.resize( 10 );
    mState.mAxes.resize( 6 );
  }

  Real XInputController::filterThumbAxis( int val, int deadzone )
  {
    if ( val < 0 )
    {
      if ( val > -deadzone )
        return NIL_REAL_ZERO;
      val += deadzone;
      Real ret = (Real)val / (Real)( 32767 - deadzone );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    else if ( val > 0 )
    {
      if ( val < deadzone )
        return NIL_REAL_ZERO;
      val -= deadzone;
      Real ret = (Real)val / (Real)( 32767 - deadzone );
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

    DWORD ret = mSystem->getXInput()->mFunctions.pfnXInputGetState( xDevice->getXInputID(), &mXInputState );
    if ( ret == ERROR_DEVICE_NOT_CONNECTED )
    {
      xDevice->flagDisconnected();
      return;
    }
    else if ( ret != ERROR_SUCCESS )
      NIL_EXCEPT( "XInputGetState failed" );

    if ( mXInputState.dwPacketNumber == mLastPacket )
      return;

    mLastPacket = mXInputState.dwPacketNumber;

    ControllerState lastState = mState;

    // Buttons - skip 0x400 & 0x800 as they are undefined in the API
    for ( size_t i = 0; i < 6; i++ )
      mState.mButtons[i].pushed = ( ( mXInputState.Gamepad.wButtons & ( 1 << ( i + 4 ) ) ) != 0 );
    for ( size_t i = 6; i < 10; i++ )
      mState.mButtons[i].pushed = ( ( mXInputState.Gamepad.wButtons & ( 1 << ( i + 6 ) ) ) != 0 );

    // Axes
    mState.mAxes[0].absolute = filterThumbAxis( mXInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
    mState.mAxes[1].absolute = filterThumbAxis( mXInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
    mState.mAxes[2].absolute = filterThumbAxis( mXInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
    mState.mAxes[3].absolute = filterThumbAxis( mXInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
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