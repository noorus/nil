#include "nil.h"
#include "nilWindows.h"

namespace nil {

  ExternalModule::ExternalModule(): module_( NULL ), isInitialized_( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return isInitialized_;
  }

}