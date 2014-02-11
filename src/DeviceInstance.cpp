#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DeviceInstance::DeviceInstance( System* system, Device* device ):
  mSystem( system ), mDevice( device )
  {
    assert( mDevice );
  }

  const Device* DeviceInstance::getDevice() const
  {
    return mDevice;
  }

  DeviceInstance::~DeviceInstance()
  {
    //
  }

}