#include "nil.h"
#include "nilWindows.h"

namespace nil {

  ExternalModule::ExternalModule(): mModule( NULL ), mInitialized( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return mInitialized;
  }

}