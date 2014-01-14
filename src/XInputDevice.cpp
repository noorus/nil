#include "nil.h"
#include "nilUtil.h"

namespace nil {

  XInputDevice::XInputDevice( DeviceID id, int xinputID ): Device( id ),
  mXInputID( xinputID )
  {
    wprintf_s( L"Created: Device %d (%s)\r\n", mID,
      getType() == Device_XInput ? L"XInput" : L"DirectInput" );
  }

  const Device::Type XInputDevice::getType()
  {
    return Device::Device_XInput;
  }

  const int XInputDevice::getXInputID()
  {
    return mXInputID;
  }

}