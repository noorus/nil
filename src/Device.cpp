#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( System* system, DeviceID id, Type type ): system_( system ),
  id_( id ), type_( type ), status_( Status_Pending ),
  savedStatus_( Status_Pending ), instance_( nullptr ),
  disconnectFlag_( false )
  {
    // Get our type-specific index
    switch ( type_ )
    {
      case Device_Mouse:
        typedIndex_ = system_->getNextMouseIndex();
      break;
      case Device_Keyboard:
        typedIndex_ = system_->getNextKeyboardIndex();
      break;
      case Device_Controller:
        typedIndex_ = system_->getNextControllerIndex();
      break;
    }

    // Autogenerate a device name, which can be overridden later
    name_ = util::generateName( type_, typedIndex_ );
  }

  void Device::enable()
  {
    if ( instance_ || status_ != Status_Connected )
      return;

    if ( getHandler() == Handler_XInput )
    {
      XInputDevice* xDevice = dynamic_cast<XInputDevice*>( this );
      if ( !xDevice )
        NIL_EXCEPT( "Dynamic cast failed for XInputDevice" );
      if ( getType() == Device_Controller )
      {
        instance_ = new XInputController( xDevice );
        system_->controllerEnabled( this, (Controller*)instance_ );
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
        instance_ = new DirectInputController( diDevice, system_->coop_ );
        system_->controllerEnabled( this, (Controller*)instance_ );
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
        instance_ = new RawInputMouse( rawDevice, system_->getDefaultMouseButtonSwapping() );
        system_->mouseEnabled( this, (Mouse*)instance_ );
      }
      else if ( getType() == Device_Keyboard )
      {
        instance_ = new RawInputKeyboard( rawDevice );
        system_->keyboardEnabled( this, (Keyboard*)instance_ );
      }
      else
        NIL_EXCEPT( "Unsupported device type for RawInput; cannot instantiate device!" );
    }
    else
      NIL_EXCEPT( "Unsupported device handler; Cannot instantiate device!" );
  }

  DeviceInstance* Device::getInstance()
  {
    return instance_;
  }

  void Device::update()
  {
    if ( instance_ )
      instance_->update();
  }

  void Device::disable()
  {
    if ( !instance_ )
      return;

    switch ( getType() )
    {
      case Device_Controller:
        system_->controllerDisabled( this, (Controller*)instance_ );
      break;
      case Device_Mouse:
        system_->mouseDisabled( this, (Mouse*)instance_ );
      break;
      case Device_Keyboard:
        system_->keyboardDisabled( this, (Keyboard*)instance_ );
      break;
      default:
        NIL_EXCEPT( "Unimplemented device type" );
      break;
    }

    SAFE_DELETE( instance_ );
  }

  void Device::flagDisconnected()
  {
    disconnectFlag_ = true;
  }

  const bool Device::isDisconnectFlagged() const
  {
    return disconnectFlag_;
  }

  void Device::onConnect()
  {
    status_ = Status_Connected;
  }

  void Device::onDisconnect()
  {
    disable();
    status_ = Status_Disconnected;
    disconnectFlag_ = false;
  }

  System* Device::getSystem()
  {
    return system_;
  }

  const DeviceID Device::getID() const
  {
    return id_;
  }

  void Device::saveStatus()
  {
    savedStatus_ = status_;
  }

  const Device::Status Device::getSavedStatus()
  {
    return savedStatus_;
  }

  const utf8String& Device::getName() const
  {
    return name_;
  }

  const Device::Type Device::getType() const
  {
    return type_;
  }

  void Device::setStatus( Status status )
  {
    status_ = status;
  }

  const Device::Status Device::getStatus() const
  {
    return status_;
  }

  Device::~Device()
  {
  }

}