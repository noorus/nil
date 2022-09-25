#include "nilConfig.h"

#include "nil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  ExternalModule::ExternalModule(): module_( nullptr ), initialized_( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return initialized_;
  }

}

#endif