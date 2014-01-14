#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( DeviceID id ): mID( id ), mState( State_Disconnected )
  {
  }

  void Device::onPlug()
  {
    mState = State_Connected;
    wprintf_s( L"Connected: Device %d (%s)\r\n", mID,
      getType() == Device_XInput ? L"XInput" : L"DirectInput" );
  }

  void Device::onUnplug()
  {
    mState = State_Disconnected;
    wprintf_s( L"Disconnected: Device %d (%s)\r\n", mID,
      getType() == Device_XInput ? L"XInput" : L"DirectInput" );
  }

  const DeviceID Device::getID()
  {
    return mID;
  }

  void Device::setState( State state )
  {
    mState = state;
  }

  const Device::State Device::getState()
  {
    return mState;
  }

}