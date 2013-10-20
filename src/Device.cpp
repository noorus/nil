#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( GUID productID, GUID deviceID ):
  mProductID( productID ), mDeviceID( deviceID )
  {
    wprintf_s( L"Device:\r\n  ProductID: %s\r\n  DeviceID: %s\r\n",
      guidToStr( mProductID ).c_str(),
      guidToStr( mDeviceID ).c_str()
    );
  }

  const GUID Device::getProductID()
  {
    return mProductID;
  }

  const GUID Device::getDeviceID()
  {
    return mDeviceID;
  }

  Device::~Device()
  {
    wprintf_s( L"~Device:\r\n  ProductID: %s\r\n  DeviceID: %s\r\n",
      guidToStr( mProductID ).c_str(),
      guidToStr( mDeviceID ).c_str()
    );
  }

}