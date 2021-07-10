#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Keyboard::Keyboard( System* system, Device* device ):
  DeviceInstance( system, device )
  {
    //
  }

  void Keyboard::addListener( KeyboardListener* listener )
  {
    listeners_.push_back( listener );
  }

  void Keyboard::removeListener( KeyboardListener* listener )
  {
    listeners_.remove( listener );
  }

  Keyboard::~Keyboard()
  {
    //
  }

}