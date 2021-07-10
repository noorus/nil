#include "nil.h"
#include "nilWindows.h"

namespace nil {

# define NIL_RAW_TEST_MOUSE_BUTTON_DOWN(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  state_.buttons[x].pushed = true; \
  for ( auto listener : listeners_ ) \
    listener->onMouseButtonPressed( this, state_, x ); \
}

# define NIL_RAW_TEST_MOUSE_BUTTON_UP(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  state_.buttons[x].pushed = false; \
  for ( auto listener : listeners_ ) \
    listener->onMouseButtonReleased( this, state_, x ); \
}

  RawInputMouse::RawInputMouse( RawInputDevice* rawDevice, const bool swapButtons ):
  Mouse( rawDevice->getSystem(), rawDevice, swapButtons )
  {
    rawDevice->getSystem()->mapMouse( rawDevice->getRawHandle(), this );

    // All this raw device info is usually worthless, at least when using
    // the default Windows mouse driver. The number of buttons will be "16",
    // sample rate will be 0 and it will not indicate horizontal scrolling
    // even when horizontal scrolling is available.
    // So don't actually trust this information for anything.
    // (We will use it for sizing the button vector, however)

    const RID_DEVICE_INFO* rawInfo = rawDevice->getRawInfo();

    state_.buttons.resize( (size_t)rawInfo->mouse.dwNumberOfButtons );

    sampleRate_ = rawInfo->mouse.dwSampleRate;
    hasHorizontalWheel_ = rawInfo->mouse.fHasHorizontalWheel ? true : false;
  }

  void RawInputMouse::onRawInput( const RAWMOUSE& input )
  {
    // Reset everything but the buttons
    state_.reset();

    if ( input.usFlags & MOUSE_MOVE_ABSOLUTE )
    {
      // Calculate the relative change ourselves, if something actually
      // does pass absolute coordinates to us
      Vector2i newPosition( input.lLastX, input.lLastY );
      state_.movement.relative = newPosition - lastPosition_;
      lastPosition_ = newPosition;
    }
    else
    {
      // Apparently even the default Windows 7 mouse driver doesn't
      // bother setting MOUSE_MOVE_RELATIVE, so we don't check for that.
      // But relative movement is still the default in all observed cases.
      state_.movement.relative.x = input.lLastX;
      state_.movement.relative.y = input.lLastY;
    }

    if ( state_.movement.relative.x != 0
      || state_.movement.relative.y != 0 )
    {
      for ( auto listener : listeners_ )
        listener->onMouseMoved( this, state_ );
    }

    // This is butt-ugly, but at least it's semantically correct.
    // Also, the raw input API does not support more than 5 buttons.

    if ( !state_.buttons.empty() )
    {
      int btn = ( swapButtons_ ? 1 : 0 );
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_1_DOWN, btn );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_1_UP, btn );
    }

    if ( state_.buttons.size() > 1 )
    {
      int btn = ( swapButtons_ ? 0 : 1 );
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_2_DOWN, btn );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_2_UP, btn );
    }

    if ( state_.buttons.size() > 2 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_3_DOWN, 2 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_3_UP, 2 );
    }

    if ( state_.buttons.size() > 3 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_4_DOWN, 3 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_4_UP, 3 );
    }

    if ( state_.buttons.size() > 4 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_5_DOWN, 4 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_5_UP, 4 );
    }

    // At least the wheel delta is simple and reliable
    if ( input.usButtonFlags & RI_MOUSE_WHEEL )
    {
      state_.wheel.relative = (short)input.usButtonData;
      for ( auto listener : listeners_ )
        listener->onMouseWheelMoved( this, state_ );
    }
  }

  void RawInputMouse::update()
  {
    // Nothing to update, since we process events as they come
  }

  RawInputMouse::~RawInputMouse()
  {
    auto rawDevice = static_cast<RawInputDevice*>( device_ );

    rawDevice->getSystem()->unmapMouse( rawDevice->getRawHandle() );
  }

}