#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DeviceInstance::DeviceInstance( SystemPtr system, DevicePtr device ):
  system_( system ), device_( device )
  {
    assert( device_ );
  }

  const DevicePtr DeviceInstance::getDevice() const
  {
    return device_;
  }

  DeviceInstance::~DeviceInstance()
  {
  }

}