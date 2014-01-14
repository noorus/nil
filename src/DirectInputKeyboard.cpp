#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DirectInputKeyboard::DirectInputKeyboard( DirectInputDevice* device ):
  Keyboard( device->getSystem(), device )
  {
    //
  }

  DirectInputKeyboard::~DirectInputKeyboard()
  {
    //
  }

}