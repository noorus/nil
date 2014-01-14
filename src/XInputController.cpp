#include "nil.h"
#include "nilUtil.h"

namespace nil {

  XInputController::XInputController( XInputDevice* device ):
  Controller( device->getSystem(), device )
  {
    //
  }

  XInputController::~XInputController()
  {
    //
  }

}