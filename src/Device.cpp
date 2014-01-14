#include "nil.h"
#include "nilUtil.h"

namespace nil {

  // Device class

  Device::Device( DeviceID id, Type type ): mID( id ), mType( type ),
  mStatus( Status_Pending ), mSavedStatus( Status_Pending )
  {
  }

  void Device::onConnect()
  {
    mStatus = Status_Connected;
    wprintf_s( L"Connected: (%d) %s (%s %s)\r\n",
      getID(),
      getName().c_str(),
      getHandler() == Device::Handler_XInput ? L"XInput" : L"DirectInput",
      getType() == Device::Device_Mouse ? L"Mouse" : getType() == Device::Device_Keyboard ? L"Keyboard" : L"Controller" );
  }

  void Device::onDisconnect()
  {
    mStatus = Status_Disconnected;
    wprintf_s( L"Disconnected: (%d) %s (%s %s)\r\n",
      getID(),
      getName().c_str(),
      getHandler() == Device::Handler_XInput ? L"XInput" : L"DirectInput",
      getType() == Device::Device_Mouse ? L"Mouse" : getType() == Device::Device_Keyboard ? L"Keyboard" : L"Controller" );
  }

  const DeviceID Device::getID()
  {
    return mID;
  }

  void Device::saveStatus()
  {
    mSavedStatus = mStatus;
  }

  const Device::Status Device::getSavedStatus()
  {
    return mSavedStatus;
  }

  const String& Device::getName()
  {
    return mName;
  }

  const Device::Type Device::getType()
  {
    return mType;
  }

  void Device::setStatus( Status status )
  {
    mStatus = status;
  }

  const Device::Status Device::getStatus()
  {
    return mStatus;
  }

  // DeviceInstance class

  DeviceInstance::DeviceInstance( System* system, Device* device ):
  mSystem( system ), mDevice( device )
  {
    //
  }

  DeviceInstance::~DeviceInstance()
  {
    //
  }

  // Controller class

  const Controller::Type Controller::getType()
  {
    return mType;
  }

}