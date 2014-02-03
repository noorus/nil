#include "nil.h"
#include "nilUtil.h"

namespace nil {

  class DummyMouseListener: public MouseListener {
  public:
    virtual void onMouseMoved( Mouse* mouse, const MouseState& state )
    {
      //
    }
    virtual void onMouseButtonPressed( Mouse* mouse, const MouseState& state, size_t button )
    {
      wprintf_s( L"Mouse button pressed: %d (%s)\r\n", button, mouse->getDevice()->getName().c_str() );
    }
    virtual void onMouseButtonReleased( Mouse* mouse, const MouseState& state, size_t button )
    {
      wprintf_s( L"Mouse button released: %d (%s)\r\n", button, mouse->getDevice()->getName().c_str() );
    }
    virtual void onMouseWheelMoved( Mouse* mouse, const MouseState& state )
    {
      wprintf_s( L"Mouse wheel moved: %d (%s)\r\n", state.mWheel.relative, mouse->getDevice()->getName().c_str() );
    }
  };

  DummyMouseListener gDummyMouseListener;

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
    mListeners.push_back( &gDummyMouseListener );
  }

  Mouse::~Mouse()
  {
    //
  }

}