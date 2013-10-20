#include "nil.h"
#include "nilUtil.h"

namespace nil {

  wstring guidToStr( GUID guid )
  {
    OLECHAR* bstrGuid;
    StringFromCLSID( guid, &bstrGuid );
    wstring str = bstrGuid;
    ::CoTaskMemFree( bstrGuid );
    return str;
  }

  Device::Device( GUID productID, GUID deviceID ):
  mProductID( productID ), mDeviceID( deviceID )
  {
    wprintf_s( L"Device: %s\r\n", guidToStr( mDeviceID ).c_str() );
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
    wprintf_s( L"~Device: %s\r\n", guidToStr( mDeviceID ).c_str() );
  }

}