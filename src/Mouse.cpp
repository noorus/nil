#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  // MouseState class

  MouseState::MouseState()
  {
    reset();
  }

  void MouseState::reset()
  {
    // We do NOT reset buttons!

    mWheel.relative = 0;

    mMovement.relative.x = 0;
    mMovement.relative.y = 0;
  }

  // Mouse class

  Mouse::Mouse( System* system, Device* device ):
  DeviceInstance( system, device )
  {
  }

  void Mouse::addListener( MouseListener* listener )
  {
    mListeners.push_back( listener );
  }

  void Mouse::removeListener( MouseListener* listener )
  {
    mListeners.remove( listener );
  }

  const MouseState& Mouse::getState() const
  {
    return mState;
  }

  Mouse::~Mouse()
  {
    //
  }

}