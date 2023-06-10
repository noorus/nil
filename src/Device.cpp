#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Device::Device( SystemPtr system, DeviceID id, Type type ):
  type_( type ), system_( system ), id_( id )
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
      auto xDevice = dynamic_pointer_cast<XInputDevice>( ptr() );
      if ( !xDevice )
        NIL_EXCEPT( "Dynamic cast failed for XInputDevice" );
      if ( getType() == Device_Controller )
      {
        instance_ = make_shared<XInputController>( xDevice )->ptr();
        system_->controllerEnabled( ptr(), dynamic_pointer_cast<Controller>( instance_->ptr() ) );
      }
      else
        NIL_EXCEPT( "Unsupport device type for XInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_DirectInput )
    {
      auto diDevice = dynamic_pointer_cast<DirectInputDevice>( ptr() );
      if ( !diDevice )
        NIL_EXCEPT( "Dynamic cast failed for DirectInputDevice" );
      if ( getType() == Device_Controller )
      {
        instance_ = make_shared<DirectInputController>( diDevice, system_->coop_ )->ptr();
        system_->controllerEnabled( ptr(), dynamic_pointer_cast<Controller>( instance_->ptr() ) );
      }
      else
        NIL_EXCEPT( "Unsupported device type for DirectInput; Cannot instantiate device!" );
    }
    else if ( getHandler() == Handler_RawInput )
    {
      auto rawDevice = dynamic_pointer_cast<RawInputDevice>( ptr() );
      if ( !rawDevice )
        NIL_EXCEPT( "Dynamic cast failed for RawInputDevice" );
      if ( getType() == Device_Mouse )
      {
        instance_ = make_shared<RawInputMouse>( rawDevice, system_->getDefaultMouseButtonSwapping() )->ptr();
        system_->mouseEnabled( ptr(), dynamic_pointer_cast<Mouse>( instance_->ptr() ) );
      }
      else if ( getType() == Device_Keyboard )
      {
        instance_ = make_shared<RawInputKeyboard>( rawDevice )->ptr();
        system_->keyboardEnabled( ptr(), dynamic_pointer_cast<Keyboard>( instance_->ptr() ) );
      }
      else if ( getType() == Device_Controller )
      {
        instance_ = make_shared<RawInputController>( rawDevice )->ptr();
        system_->controllerEnabled( ptr(), dynamic_pointer_cast<Controller>( instance_->ptr() ) );
      }
      else
        NIL_EXCEPT( "Unsupported device type for RawInput; cannot instantiate device!" );
    }
    else
      NIL_EXCEPT( "Unsupported device handler; Cannot instantiate device!" );
  }

  DeviceInstance* Device::getInstance()
  {
    return instance_.get();
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
        system_->controllerDisabled( ptr(), dynamic_pointer_cast<Controller>( instance_->ptr() ) );
      break;
      case Device_Mouse:
        system_->mouseDisabled( ptr(), dynamic_pointer_cast<Mouse>( instance_->ptr() ) );
      break;
      case Device_Keyboard:
        system_->keyboardDisabled( ptr(), dynamic_pointer_cast<Keyboard>( instance_->ptr() ) );
      break;
      default:
        NIL_EXCEPT( "Unimplemented device type" );
      break;
    }

    instance_.reset();
  }

  void Device::flagDisconnected()
  {
    disconnectFlag_ = true;
  }

  bool Device::isDisconnectFlagged() const
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
    return system_.get();
  }

  DeviceID Device::getID() const
  {
    return id_;
  }

  void Device::saveStatus()
  {
    savedStatus_ = status_;
  }

  Device::Status Device::getSavedStatus()
  {
    return savedStatus_;
  }

  const utf8String& Device::getName() const
  {
    return name_;
  }

  Device::Type Device::getType() const
  {
    return type_;
  }

  void Device::setStatus( Status status )
  {
    status_ = status;
  }

  Device::Status Device::getStatus() const
  {
    return status_;
  }

}