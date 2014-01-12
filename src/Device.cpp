#include "nil.h"
#include "nilUtil.h"

namespace nil {

  DeviceEntry::DeviceEntry( Type type, GUID productID, GUID deviceID ):
  mType( type ), mProductID( productID ), mDeviceID( deviceID ),
  mState( State_Disconnected )
  {
    wprintf_s( L"DeviceEntry:\r\n  ProductID: %s\r\n  DeviceID: %s\r\n",
      guidToStr( mProductID ).c_str(),
      guidToStr( mDeviceID ).c_str() );
  }

  void DeviceEntry::setState( State state )
  {
    mState = state;
  }

  void DeviceEntry::makeXInput( int index )
  {
    mType = Device_XInput;
    mXInput.deviceID = index;
    wprintf_s( L"XInput:\r\n  DeviceID: %s\r\n  XInputID: %d\r\n",
      guidToStr( mDeviceID ).c_str(),
      mXInput.deviceID );
  }

  const DeviceEntry::Type DeviceEntry::getType()
  {
    return mType;
  }

  const DeviceEntry::State DeviceEntry::getState()
  {
    return mState;
  }

  const GUID DeviceEntry::getProductID()
  {
    return mProductID;
  }

  const GUID DeviceEntry::getDeviceID()
  {
    return mDeviceID;
  }

  DeviceEntry::~DeviceEntry()
  {
    wprintf_s( L"~DeviceEntry:\r\n  ProductID: %s\r\n  DeviceID: %s\r\n",
      guidToStr( mProductID ).c_str(),
      guidToStr( mDeviceID ).c_str() );
  }

}