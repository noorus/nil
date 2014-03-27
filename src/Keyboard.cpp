#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  Keyboard::Keyboard( System* system, Device* device ):
  DeviceInstance( system, device )
  {
    //
  }

  void Keyboard::addListener( KeyboardListener* listener )
  {
    mListeners.push_back( listener );
  }

  void Keyboard::removeListener( KeyboardListener* listener )
  {
    mListeners.remove( listener );
  }

  Keyboard::~Keyboard()
  {
    //
  }

}