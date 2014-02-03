#include "nil.h"
#include "nilUtil.h"

namespace nil {

  class DummyListener: public KeyboardListener {
  public:
    virtual void onKeyPressed( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key pressed: 0x%X (%s)\r\n", keycode, keyboard->getDevice()->getName().c_str() );
    }
    virtual void onKeyRepeat( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key repeat: 0x%X (%s)\r\n", keycode, keyboard->getDevice()->getName().c_str() );
    }
    virtual void onKeyReleased( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key released: 0x%X (%s)\r\n", keycode, keyboard->getDevice()->getName().c_str() );
    }
  };

  DummyListener gDummyListener;

  Keyboard::Keyboard( System* system, Device* device ):
  DeviceInstance( system, device )
  {
    mListeners.push_back( &gDummyListener );
  }

  Keyboard::~Keyboard()
  {
    //
  }

}