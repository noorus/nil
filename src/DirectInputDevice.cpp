#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DirectInputDevice::DirectInputDevice( DeviceID id,
  const GUID& productID, const GUID& instanceID ):
  Device( id ), mProductID( productID ), mInstanceID( instanceID )
  {
    wprintf_s( L"Created: Device %d (%s)\r\n", mID,
      getType() == Device_XInput ? L"XInput" : L"DirectInput" );
  }

  const Device::Type DirectInputDevice::getType()
  {
    return Device::Device_DirectInput;
  }

  const GUID DirectInputDevice::getProductID()
  {
    return mProductID;
  }

  const GUID DirectInputDevice::getInstanceID()
  {
    return mInstanceID;
  }

}