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
    if ( mInstance )
      return;

    if ( getHandler() == Handler_XInput )
    {
      XInputDevice* xDevice = dynamic_cast<XInputDevice*>( this );
      if ( !xDevice )
        NIL_EXCEPT( L"Dynamic cast failed for XInputDevice" );
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
      if ( !diDevice )
        NIL_EXCEPT( L"Dynamic cast failed for DirectInputDevice" );
      if ( getType() == Device_Controller )
      {
        mInstance = new DirectInputController( diDevice );
      }
      else
        NIL_EXCEPT( L"Unsupported device type for DirectInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_RawInput )
    {
      RawInputDevice* rawDevice = dynamic_cast<RawInputDevice*>( this );
      if ( !rawDevice )
        NIL_EXCEPT( L"Dynamic cast failed for RawInputDevice" );
      if ( getType() == Device_Mouse )
      {
        mInstance = new RawInputMouse( rawDevice );
      }
      else if ( getType() == Device_Keyboard )
      {
        mInstance = new RawInputKeyboard( rawDevice );
      }
      else
        NIL_EXCEPT( L"Unsupported device type for RawInput; cannot instantiate device!" );
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

  const bool Device::isDisconnectFlagged() const
  {
    return mDisconnectFlagged;
  }

  void Device::onConnect()
  {
    mStatus = Status_Connected;
    wprintf_s( L"Connected: (%d) %s (%s %s)\r\n",
      getID(),
      getName().c_str(),
      getHandler() == Device::Handler_XInput ? L"XInput" : getHandler() == Device::Handler_DirectInput ? L"DirectInput" : L"RawInput",
      getType() == Device::Device_Mouse ? L"Mouse" : getType() == Device::Device_Keyboard ? L"Keyboard" : L"Controller" );
    create();
  }

  void Device::onDisconnect()
  {
    mStatus = Status_Disconnected;
    wprintf_s( L"Disconnected: %d\r\n", getID() );
    mDisconnectFlagged = false;
    destroy();
  }

  System* Device::getSystem() const
  {
    return mSystem;
  }

  const DeviceID Device::getID() const
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

  const String& Device::getName() const
  {
    return mName;
  }

  const Device::Type Device::getType() const
  {
    return mType;
  }

  void Device::setStatus( Status status )
  {
    if ( mStatus != Status_Connected && status == Status_Connected )
      create();
    mStatus = status;
  }

  const Device::Status Device::getStatus() const
  {
    return mStatus;
  }

  Device::~Device()
  {
    destroy();
  }

}