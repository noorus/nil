#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  // ControllerState class

  ControllerState::ControllerState()
  {
    reset();
  }

  void ControllerState::reset()
  {
    for ( auto& button : buttons )
      button.pushed = false;

    for ( auto& axis : axes )
      axis.absolute = NIL_REAL_ZERO;

    for ( auto& slider : sliders )
      slider.absolute = Vector2f::ZERO;

    for ( auto& pov : povs )
      pov.direction = POV::Centered;
  }

  // Controller class

  Controller::Controller( SystemPtr system, DevicePtr device ):
  DeviceInstance( system, device ), type_( Controller_Unknown )
  {
  }

  void Controller::fireChanges( const ControllerState& lastState )
  {
    // Notify all our listeners
    for ( auto& listener : listeners_ )
    {
      // Buttons
      for ( size_t i = 0; i < state_.buttons.size(); i++ )
        if ( !lastState.buttons[i].pushed && state_.buttons[i].pushed )
          listener->onControllerButtonPressed( this, state_, i );
        else if ( lastState.buttons[i].pushed && !state_.buttons[i].pushed )
          listener->onControllerButtonReleased( this, state_, i );

      // Axes
      for ( size_t i = 0; i < state_.axes.size(); i++ )
        if ( lastState.axes[i].absolute != state_.axes[i].absolute )
          listener->onControllerAxisMoved( this, state_, i );

      // Sliders
      for ( size_t i = 0; i < state_.sliders.size(); i++ )
        if ( lastState.sliders[i].absolute != state_.sliders[i].absolute )
          listener->onControllerSliderMoved( this, state_, i );

      // POVs
      for ( size_t i = 0; i < state_.povs.size(); i++ )
        if ( lastState.povs[i].direction != state_.povs[i].direction )
          listener->onControllerPOVMoved( this, state_, i );
    }
  }

  void Controller::addListener( ControllerListener* listener )
  {
    listeners_.push_back( listener );
  }

  void Controller::removeListener( ControllerListener* listener )
  {
    listeners_.remove( listener );
  }

  Controller::Type Controller::getType() const
  {
    return type_;
  }

  const ControllerState& Controller::getState() const
  {
    return state_;
  }

  Controller::~Controller()
  {
  }

}