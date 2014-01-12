#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( Type type, GUID productID, GUID deviceID ):
  mType( type ), mProductID( productID ), mDeviceID( deviceID ),
  mState( State_Disconnected )
  {
    wprintf_s( L"Device:\r\n  ProductID: %s\r\n  DeviceID: %s\r\n",
      guidToStr( mProductID ).c_str(),
      guidToStr( mDeviceID ).c_str() );
  }

  void Device::setState( State state )
  {
    mState = state;
  }

  void Device::makeXInput( int index )
  {
    mType = Device_XInput;
    mXInput.deviceID = index;
    wprintf_s( L"XInput:\r\n  DeviceID: %s\r\n  XInputID: %d\r\n",
      guidToStr( mDeviceID ).c_str(),
      mXInput.deviceID );
  }

  const Device::Type Device::getType()
  {
    return mType;
  }

  const Device::State Device::getState()
  {
    return mState;
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
      guidToStr( mDeviceID ).c_str() );
  }

}