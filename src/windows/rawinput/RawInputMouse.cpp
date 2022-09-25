#include "nilConfig.h"

#include "nil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

# define NIL_RAW_TEST_MOUSE_BUTTON_DOWN(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  state_.buttons[x].pushed = true; \
  for ( auto& listener : listeners_ ) \
    listener->onMouseButtonPressed( this, state_, x ); \
}

# define NIL_RAW_TEST_MOUSE_BUTTON_UP(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  state_.buttons[x].pushed = false; \
  for ( auto& listener : listeners_ ) \
    listener->onMouseButtonReleased( this, state_, x ); \
}

  RawInputMouse::RawInputMouse( RawInputDevicePtr rawDevice, const bool swapButtons ):
  Mouse( rawDevice->getSystem()->ptr(), rawDevice, swapButtons )
  {
    rawDevice->getSystem()->mapMouse( rawDevice->getRawHandle(), this );

    // All this raw device info is usually worthless, at least when using
    // the default Windows mouse driver. The number of buttons will be "16",
    // sample rate will be 0 and it will not indicate horizontal scrolling
    // even when horizontal scrolling is available.

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
      Vector2i newPosition( input.lLastX, input.lLastY );
      state_.movement.relative = newPosition - lastPosition_;
      lastPosition_ = newPosition;
    }
    else
    {
      state_.movement.relative.x = input.lLastX;
      state_.movement.relative.y = input.lLastY;
    }

    if ( state_.movement.relative.x != 0
      || state_.movement.relative.y != 0 )
    {
      for ( auto& listener : listeners_ )
        listener->onMouseMoved( this, state_ );
    }

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

    if ( input.usButtonFlags & RI_MOUSE_WHEEL )
    {
      state_.wheel.relative = (short)input.usButtonData;
      for ( auto& listener : listeners_ )
        listener->onMouseWheelMoved( this, state_ );
    }
  }

  void RawInputMouse::update()
  {
    // Nothing to update, since we process events as they come
  }

  RawInputMouse::~RawInputMouse()
  {
    auto rawDevice = dynamic_pointer_cast<RawInputDevice>( device_ );
    rawDevice->getSystem()->unmapMouse( rawDevice->getRawHandle() );
  }

}

#endif