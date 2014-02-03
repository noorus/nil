#include "nil.h"
#include "nilUtil.h"

namespace nil {

  class DummyListener: public KeyboardListener {
  public:
    virtual void onKeyPressed( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key pressed: 0x%X\r\n", keycode );
    }
    virtual void onKeyRepeat( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key repeat: 0x%X\r\n", keycode );
    }
    virtual void onKeyReleased( Keyboard* keyboard, const VirtualKeyCode keycode )
    {
      wprintf_s( L"Key released: 0x%X\r\n", keycode );
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