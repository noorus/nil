#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DirectInputMouse::DirectInputMouse( DirectInputDevice* device ):
  Mouse( device->getSystem(), device )
  {
    //
  }

  void DirectInputMouse::update()
  {
    //
  }

  DirectInputMouse::~DirectInputMouse()
  {
    //
  }

}