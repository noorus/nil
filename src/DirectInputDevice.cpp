#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  Device::Type resolveDIDeviceType( unsigned long type )
  {
    type = GET_DIDEVICE_TYPE( type );

    switch ( type )
    {
      case DI8DEVTYPE_MOUSE:
        return Device::Device_Mouse;
      break;
      case DI8DEVTYPE_KEYBOARD:
        return Device::Device_Keyboard;
      break;
      default:
        return Device::Device_Controller;
      break;
    }
  }

  DirectInputDevice::DirectInputDevice( System* system, DeviceID id,
  LPCDIDEVICEINSTANCEW instance ):
  Device( system, id, resolveDIDeviceType( instance->dwDevType ) ),
  mProductID( instance->guidProduct ),
  mInstanceID( instance->guidInstance )
  {
    // Only replace auto-generated name if fetched one isn't empty
    String tmpName = util::cleanupName( instance->tszInstanceName );
    if ( !tmpName.empty() )
      mName = tmpName;
  }

  const Device::Handler DirectInputDevice::getHandler() const
  {
    return Device::Handler_DirectInput;
  }

  const DeviceID DirectInputDevice::getStaticID() const
  {
    // Static ID for DirectInput devices:
    // 4 bits of handler ID, 28 bits of unique id (hashed instance GUID)

    DeviceID id = util::fnv_32a_buf(
      (void*)&mInstanceID, sizeof( GUID ), FNV1_32A_INIT );

    return ( ( id >> 4 ) | ( ( Handler_DirectInput + 1 ) << 28 ) );
  }

  const GUID DirectInputDevice::getProductID() const
  {
    return mProductID;
  }

  const GUID DirectInputDevice::getInstanceID() const
  {
    return mInstanceID;
  }

}