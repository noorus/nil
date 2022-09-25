#include "nilConfig.h"

#include "nil.h"
#include "nilWindows.h"

namespace nil {

#ifdef NIL_PLATFORM_WINDOWS

  ExternalModule::ExternalModule(): module_( nullptr ), isInitialized_( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return isInitialized_;
  }

#endif

}