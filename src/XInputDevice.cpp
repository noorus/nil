#include "nil.h"
#include "nilUtil.h"

namespace nil {

  XInputDevice::XInputDevice( DeviceID id, int xinputID ): Device( id ),
  mXInputID( xinputID )
  {
    wprintf_s( L"Created: Device %d (%s)\r\n", mID,
      getHandler() == Handler_XInput ? L"XInput" : L"DirectInput" );
  }

  const Device::Handler XInputDevice::getHandler()
  {
    return Device::Handler_XInput;
  }

  const Device::Type XInputDevice::getType()
  {
    return Device::Device_Controller;
  }

  const int XInputDevice::getXInputID()
  {
    return mXInputID;
  }

}