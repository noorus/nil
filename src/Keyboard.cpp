#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Keyboard::Keyboard( SystemPtr system, DevicePtr device ):
  DeviceInstance( system, device )
  {
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
  }

}