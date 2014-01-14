#include "nil.h"
#include "nilUtil.h"

namespace nil {

  XInputDevice::XInputDevice( DeviceID id, int xinputID ):
  Device( id, Device_Controller ), mXInputID( xinputID )
  {
  }

  const Device::Handler XInputDevice::getHandler()
  {
    return Device::Handler_XInput;
  }

  const int XInputDevice::getXInputID()
  {
    return mXInputID;
  }

}