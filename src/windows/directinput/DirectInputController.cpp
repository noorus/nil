#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

#define NIL_FIELD_OFFSET(type, field) ((LONG_PTR)&(((type*)0)->field))

#define NIL_DIJ2OFS_BUTTON(n)  (NIL_FIELD_OFFSET(DIJOYSTATE2, rgbButtons)+(n))
#define NIL_DIJ2OFS_POV(n)     (NIL_FIELD_OFFSET(DIJOYSTATE2, rgdwPOV)+(n)*sizeof(DWORD))
#define NIL_DIJ2OFS_SLIDER0(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglSlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER1(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglVSlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER2(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglASlider)+(n)*sizeof(LONG))
#define NIL_DIJ2OFS_SLIDER3(n) (NIL_FIELD_OFFSET(DIJOYSTATE2, rglFSlider)+(n)*sizeof(LONG))

namespace nil {

  const unsigned long cJoystickEvents = 64;

  DirectInputController::DirectInputController( DirectInputDevice* device,
  const Cooperation coop ):
  Controller( device->getSystem(), device ), diDevice_( nullptr ),
  axisEnum_( 0 ), coop_( coop )
  {
    HRESULT hr = device->getSystem()->dinput_->CreateDevice(
      device->getInstanceID(), &diDevice_, nullptr );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not create DirectInput8 device" );

    hr = diDevice_->SetDataFormat( &c_dfDIJoystick2 );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not set DirectInput8 device data format" );

    // Force feedback, to be implemented later, requires exclusive access.
    hr = diDevice_->SetCooperativeLevel( device->getSystem()->window_,
      ( coop_ == Cooperation::Background )
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

    hr = diDevice_->SetProperty( DIPROP_BUFFERSIZE, &bufsize.diph );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not set DirectInput8 device buffer size" );

    diCaps_.dwSize = sizeof( DIDEVCAPS );
    hr = diDevice_->GetCapabilities( &diCaps_ );
    if ( FAILED( hr ) )
      NIL_EXCEPT_DINPUT( hr, "Could not get DirectInput8 device capabilities" );

    // Identify a more specific controller type, if available
    switch ( diCaps_.dwDevType )
    {
      case DI8DEVTYPE_JOYSTICK:
        type_ = Controller::Controller_Joystick;
      break;
      case DI8DEVTYPE_GAMEPAD:
        type_ = Controller::Controller_Gamepad;
      break;
      case DI8DEVTYPE_1STPERSON:
        type_ = Controller::Controller_Firstperson;
      break;
      case DI8DEVTYPE_DRIVING:
        type_ = Controller::Controller_Driving;
      break;
      case DI8DEVTYPE_FLIGHT:
        type_ = Controller::Controller_Flight;
      break;
    }

    state_.povs.resize( (size_t)diCaps_.dwPOVs );
    state_.buttons.resize( (size_t)diCaps_.dwButtons );

    axisEnum_ = 0;
    sliderEnum_ = 0;

    diDevice_->EnumObjects( diComponentsEnumCallback, this, DIDFT_AXIS );

    state_.axes.resize( axisEnum_ );

    // TODO This is totally untested. I haven't found a single controller
    // that actually reports sliders, so this code _could_ crash and burn.
    // Let me know if it does. :)
    if ( sliderEnum_ > 0 )
    {
      sliderEnum_ /= 2;
      state_.sliders.resize( sliderEnum_ );
    }
  }

  BOOL CALLBACK DirectInputController::diComponentsEnumCallback(
  LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer )
  {
    auto controller = static_cast<DirectInputController*>( referer );

    if ( component->guidType == GUID_Slider )
    {
      controller->sliderEnum_++;
    }
    else
    {
      DIPROPPOINTER prop;
      prop.diph.dwSize       = sizeof( DIPROPPOINTER );
      prop.diph.dwHeaderSize = sizeof( DIPROPHEADER );
      prop.diph.dwHow        = DIPH_BYID;
      prop.diph.dwObj        = component->dwType;
      prop.uData             = 0x6E690000 | controller->axisEnum_;

      HRESULT hr = controller->diDevice_->SetProperty( DIPROP_APPDATA, &prop.diph );
      if ( FAILED( hr ) )
        return DIENUM_CONTINUE;

      controller->axisEnum_++;
    }

    DIPROPRANGE range;
    range.diph.dwSize       = sizeof( DIPROPRANGE );
    range.diph.dwHeaderSize = sizeof( DIPROPHEADER );
    range.diph.dwHow        = DIPH_BYID;
    range.diph.dwObj        = component->dwType;
    range.lMin              = -32768;
    range.lMax              = 32767;

    HRESULT hr = controller->diDevice_->SetProperty( DIPROP_RANGE, &range.diph );
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
    if ( val > 0 )
    {
      Real ret = (Real)val / (Real)( 32767 );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    return NIL_REAL_ZERO;
  }

  void DirectInputController::update()
  {
    DIDEVICEOBJECTDATA buffers[cJoystickEvents];
    unsigned long entries = cJoystickEvents;

    ControllerState lastState = state_;

    bool done = false;

    while ( !done )
    {
      HRESULT hr = diDevice_->Poll();
      if ( hr == DI_OK )
        hr = diDevice_->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), buffers, &entries, 0 );

      if ( hr != DI_OK )
      {
        hr = diDevice_->Acquire();
        while ( hr == DIERR_INPUTLOST )
          hr = diDevice_->Acquire();
        hr = diDevice_->Poll();
        if ( FAILED( hr ) )
          return;
        hr = diDevice_->GetDeviceData( sizeof( DIDEVICEOBJECTDATA ), buffers, &entries, 0 );
        if ( FAILED( hr ) )
          return;
      }

      if ( entries < cJoystickEvents )
        done = true;

      for ( unsigned long i = 0; i < entries; i++ )
      {
        if ( (size_t)buffers[i].dwOfs >= NIL_DIJ2OFS_POV( 0 )
        && (size_t)buffers[i].dwOfs < NIL_DIJ2OFS_POV( state_.povs.size() ) )
        {
          size_t pov = (size_t)buffers[i].dwOfs - NIL_DIJ2OFS_POV( 0 );
          if ( LOWORD( buffers[i].dwData ) == 0xFFFF )
            state_.povs[pov].direction = POV::Centered;
          else
          {
            switch ( buffers[i].dwData )
            {
              case 0: state_.povs[pov].direction = POV::North; break;
              case 4500: state_.povs[pov].direction = POV::NorthEast; break;
              case 9000: state_.povs[pov].direction = POV::East; break;
              case 13500: state_.povs[pov].direction = POV::SouthEast; break;
              case 18000: state_.povs[pov].direction = POV::South; break;
              case 22500: state_.povs[pov].direction = POV::SouthWest; break;
              case 27000: state_.povs[pov].direction = POV::West; break;
              case 31500: state_.povs[pov].direction = POV::NorthWest; break;
            }
          }
        }
        else if ( (size_t)buffers[i].dwOfs >= NIL_DIJ2OFS_BUTTON( 0 )
        && (size_t)buffers[i].dwOfs < NIL_DIJ2OFS_BUTTON( state_.buttons.size() ) )
        {
          auto button = static_cast<size_t>( buffers[i].dwOfs - NIL_DIJ2OFS_BUTTON( 0 ) );
          state_.buttons[button].pushed = ( buffers[i].dwData & 0x80 ? true : false );
        }
        else if ( (uint16_t)( buffers[i].uAppData >> 16 ) == 0x6E69 )
        {
          auto axis = static_cast<size_t>( 0x0000FFFF & buffers[i].uAppData );
          state_.axes[axis].absolute = filterAxis( buffers[i].dwData );
        }
        else
        {
          switch ( buffers[i].dwOfs )
          {
            case NIL_DIJ2OFS_SLIDER0( 0 ):
              state_.sliders[0].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER0( 1 ):
              state_.sliders[0].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER1( 0 ):
              state_.sliders[1].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER1( 1 ):
              state_.sliders[1].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER2( 0 ):
              state_.sliders[2].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER2( 1 ):
              state_.sliders[2].absolute.y = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER3( 0 ):
              state_.sliders[3].absolute.x = filterAxis( buffers[i].dwData );
            break;
            case NIL_DIJ2OFS_SLIDER3( 1 ):
              state_.sliders[3].absolute.y = filterAxis( buffers[i].dwData );
            break;
          }
        }
      }
    }

    fireChanges( lastState );
  }

  DirectInputController::~DirectInputController()
  {
    if ( diDevice_ )
    {
      diDevice_->Unacquire();
      diDevice_->Release();
    }
  }

}

#endif