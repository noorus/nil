#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  // MouseState class

  MouseState::MouseState()
  {
    reset();
  }

  void MouseState::reset()
  {
    // We do NOT reset buttons!

    wheel.relative = 0;

    movement.relative.x = 0;
    movement.relative.y = 0;
  }

  // Mouse class

  Mouse::Mouse( System* system, Device* device, const bool swapButtons ):
  DeviceInstance( system, device ), swapButtons_( swapButtons )
  {
  }

  void Mouse::addListener( MouseListener* listener )
  {
    listeners_.push_back( listener );
  }

  void Mouse::removeListener( MouseListener* listener )
  {
    listeners_.remove( listener );
  }

  const MouseState& Mouse::getState() const
  {
    return state_;
  }

  Mouse::~Mouse()
  {
    //
  }

}