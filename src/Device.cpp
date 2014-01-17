#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( System* system, DeviceID id, Type type ): mSystem( system ),
  mID( id ), mType( type ), mStatus( Status_Pending ),
  mSavedStatus( Status_Pending ), mInstance( nullptr ),
  mDisconnectFlagged( false )
  {
  }

  void Device::create()
  {
    if ( getHandler() == Handler_XInput )
    {
      XInputDevice* xDevice = dynamic_cast<XInputDevice*>( this );
      if ( getType() == Device_Controller )
      {
        mInstance = new XInputController( xDevice );
      }
      else
        NIL_EXCEPT( L"Unsupport device type for XInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_DirectInput )
    {
      DirectInputDevice* diDevice = dynamic_cast<DirectInputDevice*>( this );
      if ( getType() == Device_Mouse )
      {
        mInstance = new DirectInputMouse( diDevice );
      }
      else if ( getType() == Device_Keyboard )
      {
        mInstance = new DirectInputKeyboard( diDevice );
      }
      else if ( getType() == Device_Controller )
      {
        mInstance = new DirectInputController( diDevice );
      }
      else
        NIL_EXCEPT( L"Unsupported device type for DirectInput; Cannot instantiate device!" );
    }
    else
      NIL_EXCEPT( L"Unsupported device handler; Cannot instantiate device!" );
  }

  void Device::update()
  {
    if ( mInstance )
      mInstance->update();
  }

  void Device::destroy()
  {
    SAFE_DELETE( mInstance );
  }

  void Device::flagDisconnected()
  {
    mDisconnectFlagged = true;
  }

  const bool Device::isDisconnectFlagged()
  {
    return mDisconnectFlagged;
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
    mDisconnectFlagged = false;
    destroy();
  }

  System* Device::getSystem()
  {
    return mSystem;
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

  Device::~Device()
  {
    destroy();
  }

}