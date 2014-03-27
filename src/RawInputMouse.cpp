#include "nil.h"

namespace Nil {

  // Yeah, uh, don't ask.

# define NIL_RAW_TEST_MOUSE_BUTTON_DOWN(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  mState.mButtons[x].pushed = true; \
  for ( auto listener : mListeners ) \
    listener->onMouseButtonPressed( this, mState, x ); \
}

# define NIL_RAW_TEST_MOUSE_BUTTON_UP(flag,x) if ( input.usButtonFlags & flag ) \
  { \
  mState.mButtons[x].pushed = false; \
  for ( auto listener : mListeners ) \
    listener->onMouseButtonReleased( this, mState, x ); \
}

  RawInputMouse::RawInputMouse( RawInputDevice* rawDevice ):
  Mouse( rawDevice->getSystem(), rawDevice )
  {
    rawDevice->getSystem()->mapMouse( rawDevice->getRawHandle(), this );

    // All this raw device info is usually worthless, at least when using
    // the default Windows mouse driver. The number of buttons will be "16",
    // sample rate will be 0 and it will not indicate horizontal scrolling
    // even when the feature exists.
    // So don't actually trust this information for anything.
    // (We will use it for sizing the button vector, however)

    mState.mButtons.resize( rawDevice->getRawInfo()->mouse.dwNumberOfButtons );

    mSampleRate = rawDevice->getRawInfo()->mouse.dwSampleRate;
    mHorizontalWheel = rawDevice->getRawInfo()->mouse.fHasHorizontalWheel ? true : false;
  }

  void RawInputMouse::onRawInput( const RAWMOUSE& input )
  {
    // Reset everything but the buttons
    mState.reset();

    if ( input.usFlags & MOUSE_MOVE_ABSOLUTE )
    {
      // Calculate the relative change ourselves, if something actually
      // does pass absolute coordinates to us
      Vector2i newPosition( input.lLastX, input.lLastY );
      mState.mMovement.relative = newPosition - mLastPosition;
      mLastPosition = newPosition;
    }
    else
    {
      // Apparently even the default Windows 7 mouse driver doesn't
      // bother setting MOUSE_MOVE_RELATIVE, so we don't check for that.
      // But relative movement is still the default in all observed cases.
      mState.mMovement.relative.x = input.lLastX;
      mState.mMovement.relative.y = input.lLastY;
    }

    if ( mState.mMovement.relative.x != 0
      || mState.mMovement.relative.y != 0 )
    {
      for ( auto listener : mListeners )
        listener->onMouseMoved( this, mState );
    }

    // This is butt-ugly, but at least it's semantically correct.
    // Also, the raw input API does not support more than 5 buttons.

    if ( mState.mButtons.size() > 0 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_1_DOWN, 0 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_1_UP, 0 );
    }

    if ( mState.mButtons.size() > 1 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_2_DOWN, 1 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_2_UP, 1 );
    }

    if ( mState.mButtons.size() > 2 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_3_DOWN, 2 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_3_UP, 2 );
    }

    if ( mState.mButtons.size() > 3 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_4_DOWN, 3 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_4_UP, 3 );
    }

    if ( mState.mButtons.size() > 4 )
    {
      NIL_RAW_TEST_MOUSE_BUTTON_DOWN( RI_MOUSE_BUTTON_5_DOWN, 4 );
      NIL_RAW_TEST_MOUSE_BUTTON_UP( RI_MOUSE_BUTTON_5_UP, 4 );
    }

    // At least the wheel delta is simple and reliable
    if ( input.usButtonFlags & RI_MOUSE_WHEEL )
    {
      mState.mWheel.relative = (short)input.usButtonData;
      for ( auto listener : mListeners )
        listener->onMouseWheelMoved( this, mState );
    }
  }

  void RawInputMouse::update()
  {
    // Nothing to update, since we process events as they come
  }

  RawInputMouse::~RawInputMouse()
  {
    auto rawDevice = static_cast<RawInputDevice*>( mDevice );

    rawDevice->getSystem()->unmapMouse( rawDevice->getRawHandle() );
  }

}