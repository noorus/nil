#include "nil.h"
#include "nilLogitech.h"

namespace Nil {

  ExternalModule::ExternalModule(): mModule( NULL ), mInitialized( false )
  {
  }

  bool ExternalModule::isInitialized() const
  {
    return mInitialized;
  }

}