#include "nil.h"
#include "nilUtil.h"

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
  Controller( device->getSystem(), device )
  {
    for ( int i = 0; i < cMaxXInputTypes; i++ ) {
      if ( cXInputTypes[i].first == device->getCapabilities().SubType )
        mType = cXInputTypes[i].second;
    }
  }

  void XInputController::update()
  {
    XInputDevice* xDevice = dynamic_cast<XInputDevice*>( mDevice );

    DWORD ret = XInputGetState( xDevice->getXInputID(), &mState );
    if ( ret == ERROR_DEVICE_NOT_CONNECTED )
    {
      xDevice->flagDisconnected();
      return;
    }
    else if ( ret != ERROR_SUCCESS )
      NIL_EXCEPT( L"XInputGetState failed!" );
  }

  XInputController::~XInputController()
  {
    //
  }

}