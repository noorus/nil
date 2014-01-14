#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DirectInputController::DirectInputController( DirectInputDevice* device ):
  Controller( device->getSystem(), device )
  {
    //
  }

  DirectInputController::~DirectInputController()
  {
    //
  }

}