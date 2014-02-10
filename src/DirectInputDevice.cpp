#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DirectInputDevice::DirectInputDevice( System* system, DeviceID id,
  LPCDIDEVICEINSTANCEW instance ):
  Device( system, id, Device_Controller ),
  mProductID( instance->guidProduct ),
  mInstanceID( instance->guidInstance )
  {
    unsigned long deviceType = GET_DIDEVICE_TYPE( instance->dwDevType );

    switch ( deviceType )
    {
      case DI8DEVTYPE_MOUSE:
        mType = Device_Mouse;
      break;
      case DI8DEVTYPE_KEYBOARD:
        mType = Device_Keyboard;
      break;
    }

    // Auto-generate a name an type-specific index
    initAfterTyped();

    // Only replace auto-generated name if fetched one isn't empty
    String tmpName = util::cleanupName( instance->tszInstanceName );
    if ( !tmpName.empty() )
      mName = tmpName;
  }

  const Device::Handler DirectInputDevice::getHandler() const
  {
    return Device::Handler_DirectInput;
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