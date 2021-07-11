#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DeviceInstance::DeviceInstance( System* system, Device* device ):
  system_( system ), device_( device )
  {
    assert( device_ );
  }

  const Device* DeviceInstance::getDevice() const
  {
    return device_;
  }

  DeviceInstance::~DeviceInstance()
  {
    //
  }

}