#include "nilConfig.h"

#include "nil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  RawInputController::RawInputController( RawInputDevicePtr device )
      : Controller( device->getSystem()->ptr(), device )
  {
    device->getSystem()->mapController( device->getRawHandle(), this );

    devType_ = device->getHIDReccord()->knownDeviceType();
    connType_ = device->getHIDReccord()->connectionType();

    if ( devType_ == KnownDevice_DualSense )
      setupDualSense();
  }

  void RawInputController::setupDualSense()
  {
    state_.povs.resize( 1 );
    state_.axes.resize( 6 );
    state_.buttons.resize( 19 );
  }

  inline Real filterThumbAxis( int val, int deadzone )
  {
    if ( val < 0 )
    {
      if ( val > -deadzone )
        return NIL_REAL_ZERO;
      val += deadzone;
      auto ret = static_cast<Real>( val ) / static_cast<Real>( 128 - deadzone );
      return ( ret < NIL_REAL_MINUSONE ? NIL_REAL_MINUSONE : ret );
    }
    if ( val > 0 )
    {
      if ( val < deadzone )
        return NIL_REAL_ZERO;
      val -= deadzone;
      auto ret = static_cast<Real>( val ) / static_cast<Real>( 127 - deadzone );
      return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
    }
    return NIL_REAL_ZERO;
  }

  inline Real filterTrigger( int val, int deadzone )
  {
    if ( val < deadzone )
      return NIL_REAL_ZERO;
    val -= deadzone;
    auto ret = static_cast<Real>( val ) / static_cast<Real>( 255 - deadzone );
    return ( ret > NIL_REAL_ONE ? NIL_REAL_ONE : ret );
  }

  void RawInputController::handleDualSense( const uint8_t* buf )
  {
    if ( connType_ == HIDConnection_Bluetooth && buf[0] != 0x31 )
      return;

    int offset = ( connType_ == HIDConnection_Bluetooth ? 1 : 0 );

    state_.axes[0].absolute = filterThumbAxis( buf[1 + offset] - 128, 5 );
    state_.axes[1].absolute = filterThumbAxis( buf[2 + offset] - 128, 5 );
    state_.axes[2].absolute = filterThumbAxis( buf[3 + offset] - 128, 5 );
    state_.axes[3].absolute = filterThumbAxis( buf[4 + offset] - 128, 5 );
    state_.axes[4].absolute = filterTrigger( buf[5 + offset], 30 );
    state_.axes[5].absolute = filterTrigger( buf[6 + offset], 30 );

    auto tmp = buf[8 + offset];
    state_.buttons[0].pushed = ( tmp & ( 1 << 7 ) ) != 0; // Triangle
    state_.buttons[1].pushed = ( tmp & ( 1 << 6 ) ) != 0; // Circle
    state_.buttons[2].pushed = ( tmp & ( 1 << 5 ) ) != 0; // Cross
    state_.buttons[3].pushed = ( tmp & ( 1 << 4 ) ) != 0; // Square

    POVDirection& pov = state_.povs[0].direction;
    pov = POV::Centered;
    switch ( tmp & 0x0F )
    {
      case 0x0: pov = POV::North; break;
      case 0x4: pov = POV::South; break;
      case 0x6: pov = POV::West; break;
      case 0x2: pov = POV::East; break;
      case 0x5: pov = POV::SouthWest; break;
      case 0x7: pov = POV::NorthWest; break;
      case 0x1: pov = POV::NorthEast; break;
      case 0x3: pov = POV::SouthEast; break;
    }

    tmp = buf[9 + offset];
    state_.buttons[4].pushed = ( tmp & ( 1 << 7 ) ) != 0; // Right stick
    state_.buttons[5].pushed = ( tmp & ( 1 << 6 ) ) != 0; // Left stick
    state_.buttons[6].pushed = ( tmp & ( 1 << 5 ) ) != 0; // Options
    state_.buttons[7].pushed = ( tmp & ( 1 << 4 ) ) != 0; // Create/Share
    state_.buttons[8].pushed = ( tmp & ( 1 << 3 ) ) != 0; // R2
    state_.buttons[9].pushed = ( tmp & ( 1 << 2 ) ) != 0; // L2
    state_.buttons[10].pushed = ( tmp & ( 1 << 1 ) ) != 0; // R1
    state_.buttons[11].pushed = ( tmp & ( 1 << 0 ) ) != 0; // L1

    tmp = buf[10 + offset];
    state_.buttons[12].pushed = ( tmp & ( 1 << 0 ) ) != 0; // PS logo
    state_.buttons[13].pushed = ( tmp & 0x02 ) != 0; // Touchpad
    state_.buttons[14].pushed = ( tmp & ( 1 << 2 ) ) != 0; // Mic button
    state_.buttons[15].pushed = ( tmp & ( 1 << 4 ) ) != 0;
    state_.buttons[16].pushed = ( tmp & ( 1 << 5 ) ) != 0;
    state_.buttons[17].pushed = ( tmp & ( 1 << 6 ) ) != 0;
    state_.buttons[18].pushed = ( tmp & ( 1 << 7 ) ) != 0;
  }

  void RawInputController::onRawInput( const RAWHID& input )
  {
    ControllerState lastState = state_;

    auto buf = &input.bRawData[0];
    if ( devType_ == KnownDevice_DualSense )
      handleDualSense( buf );

    fireChanges( lastState );
  }

  void RawInputController::update()
  {
    // Nothing to update, since we process events as they come
  }

  RawInputController::~RawInputController()
  {
    auto rawDevice = dynamic_pointer_cast<RawInputDevice>( device_ );
    rawDevice->getSystem()->unmapController( rawDevice->getRawHandle() );
  }

}

#endif