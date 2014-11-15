#include "nil.h"
#include "nilUtil.h"

#define NIL_FIELD_OFFSET(type, field) ((LONG_PTR)&(((type*)0)->field))

#define NIL_DIJ2OFS_BUTTON(n)  (NIL_FIELD_OFFSET(DIJOYSTATE2, rgbButtons)+(n))
#define NIL_DIJ2OFS_POV(n)     (NIL_FIELD_OFFSET(DIJOYSTATE2, rgdwPOV)+(n)*sizeof(DWORD))
#define NIL_DIJ2OFS_SLIDER0(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglSlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER1(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglVSlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER2(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglASlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER3(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglFSlider)+(n)*sizeof(LONG))

namespace Nil {

  const unsigned long cJoystickEvents = 64;

  DirectInputController::DirectInputController( DirectInputDevice* device,
  const Cooperation coop ):
  Controller( device->getSystem(), device ), mDIDevice( nullptr ),
  mAxisEnum( 0 ), mCooperation( coop )
  {
    HRESULT hr = device->getSystem()->mDirectInput->CreateDevice(
      device->getInstanceID(), &mDIDevice, NULL );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not create DirectInput8 device" );

    hr = mDIDevice->SetDataFormat( &c_dfDIJoystick2 );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not set DirectInput8 device data format" );

    // Force feedback, to be implemented later, requires exclusive access.
    hr = mDIDevice->SetCooperativeLevel( device->getSystem()->mWindow,
      ( mCooperation == Cooperation_Background )
      ? DISCL_BACKGROUND | DISCL_EXCLUSIVE
      : DISCL_FOREGROUND | DISCL_EXCLUSIVE );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not set DirectInput8 device cooperation level" );

    DIPROPDWORD bufsize;
    bufsize.diph.dwSize       = sizeof( DIPROPDWORD );
    bufsize.diph.dwHeaderSize = sizeof( DIPROPHEADER );
    bufsize.diph.dwObj        = 0;
    bufsize.diph.dwHow        = DIPH_DEVICE;
    bufsize.dwData            = cJoystickEvents;

    hr = mDIDevice->SetProperty( DIPROP_BUFFERSIZE, &bufsize.diph );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not set DirectInput8 device buffer size" );

    mDICapabilities.dwSize = sizeof( DIDEVCAPS );
    hr = mDIDevice->GetCapabilities( &mDICapabilities );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not get DirectInput8 device capabilities" );

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

    mState.mPOVs.resize( (size_t)mDICapabilities.dwPOVs );
    mState.mButtons.resize( (size_t)mDICapabilities.dwButtons );

    mAxisEnum = 0;
    mSliderEnum = 0;

    mDIDevice->EnumObjects( diComponentsEnumCallback, this, DIDFT_AXIS );

    mState.mAxes.resize( mAxisEnum );

    // TODO This is totally untested. I haven't found a single controller
    // that actually reports sliders, so this code _could_ crash and burn.
    // Let me know if it does. :)
    if ( mSliderEnum > 0 )
    {
      mSliderEnum /= 2;
      mState.mSliders.resize( mSliderEnum );
    }
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
      NIL_EXCEPT_DINPUT( hr, "Could not set axis range property on DirectInput8 device" );

    return DIENUM_CONTINUE;
  }

  Real DirectInputController::filterAxis( int val )
  {
    if ( val < 0 )
    {
      Real ret = (Real)val / (Real)( 32767 );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    else if ( val > 0 )
    {
      Real ret = (Real)val / (Real)( 32767 );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    else
      return NIL_REAL_ZERO;
  }

  void DirectInputController::update()
  {
    DIDEVICEOBJECTDATA buffers[cJoystickEvents];
    unsigned long entries = cJoystickEvents;

    ControllerState lastState = mState;

    bool done = false;

    while ( !done )
    {
      HRESULT hr = mDIDevice->Poll();
      if ( hr == DI_OK )
        hr = mDIDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), buffers, &entries, 0 );

      if ( hr != DI_OK )
      {
        hr = mDIDevice->Acquire();
        while ( hr == DIERR_INPUTLOST )
          hr = mDIDevice->Acquire();
        hr = mDIDevice->Poll();
        if ( FAILED( hr ) )
          return;
        hr = mDIDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), buffers, &entries, 0 );
        if ( FAILED( hr ) )
          return;
      }

      if ( entries < cJoystickEvents )
        done = true;

      for ( unsigned long i = 0; i < entries; i++ )
      {
        if ( (size_t)buffers[i].dwOfs >= NIL_DIJ2OFS_POV( 0 )
        && (size_t)buffers[i].dwOfs < NIL_DIJ2OFS_POV( mState.mPOVs.size() ) )
        {
          size_t pov = (size_t)buffers[i].dwOfs - NIL_DIJ2OFS_POV( 0 );
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
        else if ( (size_t)buffers[i].dwOfs >= NIL_DIJ2OFS_BUTTON( 0 )
        && (size_t)buffers[i].dwOfs < NIL_DIJ2OFS_BUTTON( mState.mButtons.size() ) )
        {
          size_t button = (size_t)buffers[i].dwOfs - NIL_DIJ2OFS_BUTTON( 0 );
          mState.mButtons[button].pushed = ( buffers[i].dwData & 0x80 ? true : false );
        }
        else if ( (uint16_t)( buffers[i].uAppData >> 16 ) == 0x6E69 )
        {
          size_t axis = (size_t)( 0x0000FFFF & buffers[i].uAppData );
          mState.mAxes[axis].absolute = filterAxis( buffers[i].dwData );
        }
        else
        {
          switch ( buffers[i].dwOfs )
          {
            case NIL_DIJ2OFS_SLIDER0( 0 ):
              mState.mSliders[0].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER0( 1 ):
              mState.mSliders[0].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER1( 0 ):
              mState.mSliders[1].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER1( 1 ):
              mState.mSliders[1].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER2( 0 ):
              mState.mSliders[2].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER2( 1 ):
              mState.mSliders[2].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER3( 0 ):
              mState.mSliders[3].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER3( 1 ):
              mState.mSliders[3].absolute.y = filterAxis( buffers[i].dwData );
            break;
          }
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