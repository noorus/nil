#include "nil.h"
#include "nilLogitech.h"

namespace nil {

  ExternalModule::ExternalModule(): mModule( NULL ), mInitialized( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return mInitialized;
  }

}