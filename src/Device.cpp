#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( System* system, DeviceID id, Type type ): mSystem( system ),
  mID( id ), mType( type ), mStatus( Status_Pending ),
  mSavedStatus( Status_Pending ), mInstance( nullptr ),
  mDisconnectFlagged( false )
  {
    // Get our type-specific index
    switch ( mType )
    {
      case Device_Mouse:
        mTypedIndex = mSystem->getNextMouseIndex();
      break;
      case Device_Keyboard:
        mTypedIndex = mSystem->getNextKeyboardIndex();
      break;
      case Device_Controller:
        mTypedIndex = mSystem->getNextControllerIndex();
      break;
    }

    // Autogenerate a device name, which can be overridden later
    mName = util::generateName( mType, mTypedIndex );
  }

  void Device::enable()
  {
    if ( mInstance || mStatus != Status_Connected )
      return;

    if ( getHandler() == Handler_XInput )
    {
      XInputDevice* xDevice = dynamic_cast<XInputDevice*>( this );
      if ( !xDevice )
        NIL_EXCEPT( "Dynamic cast failed for XInputDevice" );
      if ( getType() == Device_Controller )
      {
        mInstance = new XInputController( xDevice );
        mSystem->controllerEnabled( this, (Controller*)mInstance );
      }
      else
        NIL_EXCEPT( "Unsupport device type for XInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_DirectInput )
    {
      DirectInputDevice* diDevice = dynamic_cast<DirectInputDevice*>( this );
      if ( !diDevice )
        NIL_EXCEPT( "Dynamic cast failed for DirectInputDevice" );
      if ( getType() == Device_Controller )
      {
        mInstance = new DirectInputController( diDevice, mSystem->mCooperation );
        mSystem->controllerEnabled( this, (Controller*)mInstance );
      }
      else
        NIL_EXCEPT( "Unsupported device type for DirectInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_RawInput )
    {
      RawInputDevice* rawDevice = dynamic_cast<RawInputDevice*>( this );
      if ( !rawDevice )
        NIL_EXCEPT( "Dynamic cast failed for RawInputDevice" );
      if ( getType() == Device_Mouse )
      {
        mInstance = new RawInputMouse( rawDevice, mSystem->getDefaultMouseButtonSwapping() );
        mSystem->mouseEnabled( this, (Mouse*)mInstance );
      }
      else if ( getType() == Device_Keyboard )
      {
        mInstance = new RawInputKeyboard( rawDevice );
        mSystem->keyboardEnabled( this, (Keyboard*)mInstance );
      }
      else
        NIL_EXCEPT( "Unsupported device type for RawInput; cannot instantiate device!" );
    }
    else
      NIL_EXCEPT( "Unsupported device handler; Cannot instantiate device!" );
  }

  DeviceInstance* Device::getInstance()
  {
    return mInstance;
  }

  void Device::update()
  {
    if ( mInstance )
      mInstance->update();
  }

  void Device::disable()
  {
    if ( !mInstance )
      return;

    switch ( getType() )
    {
      case Device_Controller:
        mSystem->controllerDisabled( this, (Controller*)mInstance );
      break;
      case Device_Mouse:
        mSystem->mouseDisabled( this, (Mouse*)mInstance );
      break;
      case Device_Keyboard:
        mSystem->keyboardDisabled( this, (Keyboard*)mInstance );
      break;
      default:
        NIL_EXCEPT( "Unimplemented device type" );
      break;
    }

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
  }

  void Device::onDisconnect()
  {
    disable();
    mStatus = Status_Disconnected;
    mDisconnectFlagged = false;
  }

  System* Device::getSystem()
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

  const utf8String& Device::getName() const
  {
    return mName;
  }

  const Device::Type Device::getType() const
  {
    return mType;
  }

  void Device::setStatus( Status status )
  {
    mStatus = status;
  }

  const Device::Status Device::getStatus() const
  {
    return mStatus;
  }

  Device::~Device()
  {
  }

}