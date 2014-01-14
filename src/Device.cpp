#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( DeviceID id ): mID( id ), mState( State_Pending )
  {
  }

  void Device::onPlug()
  {
    mState = State_Connected;
  }

  void Device::onUnplug()
  {
    mState = State_Disconnected;
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