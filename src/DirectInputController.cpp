#include "nil.h"
#include "nilUtil.h"

#define DIJ2OFS_BUTTON(n)  (FIELD_OFFSET(DIJOYSTATE2, rgbButtons)+(n))
#define DIJ2OFS_POV(n)     (FIELD_OFFSET(DIJOYSTATE2, rgdwPOV)+(n)*sizeof(DWORD))
#define DIJ2OFS_SLIDER0(n) (FIELD_OFFSET(DIJOYSTATE2, rglSlider)+(n)*sizeof(LONG))
#define DIJ2OFS_SLIDER1(n) (FIELD_OFFSET(DIJOYSTATE2, rglVSlider)+(n)*sizeof(LONG))
#define DIJ2OFS_SLIDER2(n) (FIELD_OFFSET(DIJOYSTATE2, rglASlider)+(n)*sizeof(LONG))
#define DIJ2OFS_SLIDER3(n) (FIELD_OFFSET(DIJOYSTATE2, rglFSlider)+(n)*sizeof(LONG))

namespace nil {

  const unsigned long cJoystickEvents = 64;

  DirectInputController::DirectInputController( DirectInputDevice* device ):
  Controller( device->getSystem(), device ), mDIDevice( nullptr ), mAxisEnum( 0 )
  {
    HRESULT hr = device->getSystem()->mDirectInput->CreateDevice(
      device->getInstanceID(), &mDIDevice, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not create DirectInput8 device" );

    hr = mDIDevice->SetDataFormat( &c_dfDIJoystick2 );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not set DirectInput8 device data format" );

    hr = mDIDevice->SetCooperativeLevel( device->getSystem()->mWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not set DirectInput8 device cooperation level" );

    DIPROPDWORD bufsize;
    bufsize.diph.dwSize       = sizeof( DIPROPDWORD );
    bufsize.diph.dwHeaderSize = sizeof( DIPROPHEADER );
    bufsize.diph.dwObj        = 0;
    bufsize.diph.dwHow        = DIPH_DEVICE;
    bufsize.dwData            = cJoystickEvents;

    hr = mDIDevice->SetProperty( DIPROP_BUFFERSIZE, &bufsize.diph );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not set DirectInput8 device buffer size" );

    mDICapabilities.dwSize = sizeof( DIDEVCAPS );
    hr = mDIDevice->GetCapabilities( &mDICapabilities );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not get DirectInput8 device capabilities" );

    // Identify a more specific controller type, if available
    switch ( mDICapabilities.dwDevType )
    {
      case DI8DEVTYPE_JOYSTICK:
        mType = Controller::Controller_Joystick;
      break;
      case DI8DEVTYPE_GAMEPAD:
        mType = Controller::Controller_Gamepad;
      break;
      case DI8DEVTYPE_1STPERSON:
        mType = Controller::Controller_Firstperson;
      break;
      case DI8DEVTYPE_DRIVING:
        mType = Controller::Controller_Driving;
      break;
      case DI8DEVTYPE_FLIGHT:
        mType = Controller::Controller_Flight;
      break;
    }

    mState.mPOVs.resize( mDICapabilities.dwPOVs );
    mState.mButtons.resize( mDICapabilities.dwButtons );

    mAxisEnum = 0;
    mSliderEnum = 0;

    mDIDevice->EnumObjects( diComponentsEnumCallback, this, DIDFT_AXIS );

    mState.mAxes.resize( mAxisEnum );
    mState.mSliders.resize( mSliderEnum );
  }

  BOOL CALLBACK DirectInputController::diComponentsEnumCallback(
  LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer )
  {
    auto controller = static_cast<DirectInputController*>( referer );

    if ( component->guidType == GUID_Slider )
    {
      controller->mSliderEnum++;
    }
    else
    {
      DIPROPPOINTER prop;
      prop.diph.dwSize       = sizeof( DIPROPPOINTER );
      prop.diph.dwHeaderSize = sizeof( DIPROPHEADER );
      prop.diph.dwHow        = DIPH_BYID;
      prop.diph.dwObj        = component->dwType;
      prop.uData             = 0x6E690000 | controller->mAxisEnum;

      HRESULT hr = controller->mDIDevice->SetProperty( DIPROP_APPDATA, &prop.diph );
      if ( FAILED( hr ) )
        return DIENUM_CONTINUE;

      controller->mAxisEnum++;
    }

    DIPROPRANGE range;
    range.diph.dwSize       = sizeof( DIPROPRANGE );
    range.diph.dwHeaderSize = sizeof( DIPROPHEADER );
    range.diph.dwHow        = DIPH_BYID;
    range.diph.dwObj        = component->dwType;
    range.lMin              = -32768;
    range.lMax              = 32767;

    HRESULT hr = controller->mDIDevice->SetProperty( DIPROP_RANGE, &range.diph );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, L"Could not set axis range property on DirectInput8 device" );

    return DIENUM_CONTINUE;
  }

  Real DirectInputController::filterAxis( int val )
  {
    if ( val < 0 )
    {
      Real ret = (Real)val / (Real)( 32767 );
      return ( ret < -1.0f ? -1.0f : ret );
    }
    else if ( val > 0 )
    {
      Real ret = (Real)val / (Real)( 32767 );
      return ( ret > 1.0f ? 1.0f : ret );
    }
    else
      return 0.0f;
  }

  void DirectInputController::update()
  {
    DIDEVICEOBJECTDATA buffers[cJoystickEvents];
    unsigned long entries = cJoystickEvents;

    ControllerState lastState = mState;

    bool done = false;

    while ( !done )
    {
      HRESULT hr = mDIDevice->Acquire();
      while ( hr == DIERR_INPUTLOST )
        hr = mDIDevice->Acquire();

      hr = mDIDevice->Poll();
      if ( FAILED( hr ) )
        return; // NIL_EXCEPT_DINPUT( hr, L"Could not poll DirectInput8 device" );

      hr = mDIDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), buffers, &entries, 0 );
      if ( FAILED( hr ) )
        return; // NIL_EXCEPT_DINPUT( hr, L"Could not get DirectInput8 device data" );

      if ( entries < cJoystickEvents )
        done = true;

      for ( unsigned long i = 0; i < entries; i++ )
      {
        if ( buffers[i].dwOfs >= DIJ2OFS_POV( 0 )
        && buffers[i].dwOfs < DIJ2OFS_POV( mState.mPOVs.size() ) )
        {
          int pov = buffers[i].dwOfs - DIJ2OFS_POV( 0 );
          if ( LOWORD( buffers[i].dwData ) == 0xFFFF )
            mState.mPOVs[pov].direction = POV::Centered;
          else
          {
            switch ( buffers[i].dwData )
            {
              case 0: mState.mPOVs[pov].direction = POV::North; break;
              case 4500: mState.mPOVs[pov].direction = POV::NorthEast; break;
              case 9000: mState.mPOVs[pov].direction = POV::East; break;
              case 13500: mState.mPOVs[pov].direction = POV::SouthEast; break;
              case 18000: mState.mPOVs[pov].direction = POV::South; break;
              case 22500: mState.mPOVs[pov].direction = POV::SouthWest; break;
              case 27000: mState.mPOVs[pov].direction = POV::West; break;
              case 31500: mState.mPOVs[pov].direction = POV::NorthWest; break;
            }
          }
        }
        else if ( buffers[i].dwOfs >= DIJ2OFS_BUTTON( 0 )
        && buffers[i].dwOfs < DIJ2OFS_BUTTON( mState.mButtons.size() ) )
        {
          int button = buffers[i].dwOfs - DIJ2OFS_BUTTON( 0 );
          mState.mButtons[button].pushed = ( buffers[i].dwData & 0x80 ? true : false );
        }
        else if ( (uint16_t)( buffers[i].uAppData >> 16 ) == 0x6E69 )
        {
          int axis = (int)( 0x0000FFFF & buffers[i].uAppData );
          mState.mAxes[axis].absolute = filterAxis( buffers[i].dwData );
        }
      }
    }

    fireChanges( lastState );
  }

  DirectInputController::~DirectInputController()
  {
    if ( mDIDevice )
      mDIDevice->Unacquire();
    SAFE_RELEASE( mDIDevice );
  }

}