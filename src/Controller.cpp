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
    for ( auto button : mButtons )
      button.pushed = false;

    for ( auto axis : mAxes )
      axis.absolute = 0.0f;

    for ( auto slider : mSliders )
      slider.absolute = Vector2i::ZERO;

    for ( auto pov : mPOVs )
      pov.direction = POV::Centered;
  }

  // Controller class

  Controller::Controller( System* system, Device* device ):
  DeviceInstance( system, device ), mType( Controller_Unknown )
  {
    //
  }

  void Controller::fireChanges( const ControllerState& lastState )
  {
    // Notify all our listeners
    for ( auto listener : mListeners )
    {
      // Buttons
      for ( size_t i = 0; i < mState.mButtons.size(); i++ )
        if ( !lastState.mButtons[i].pushed && mState.mButtons[i].pushed )
          listener->onControllerButtonPressed( this, mState, i );
        else if ( lastState.mButtons[i].pushed && !mState.mButtons[i].pushed )
          listener->onControllerButtonReleased( this, mState, i );

      // Axes
      for ( size_t i = 0; i < mState.mAxes.size(); i++ )
        if ( lastState.mAxes[i].absolute != mState.mAxes[i].absolute )
          listener->onControllerAxisMoved( this, mState, i );

      // Sliders
      for ( size_t i = 0; i < mState.mSliders.size(); i++ )
        if ( lastState.mSliders[i].absolute != mState.mSliders[i].absolute )
          listener->onControllerSliderMoved( this, mState, i );

      // POVs
      for ( size_t i = 0; i < mState.mPOVs.size(); i++ )
        if ( lastState.mPOVs[i].direction != mState.mPOVs[i].direction )
          listener->onControllerPOVMoved( this, mState, i );
    }
  }

  const Controller::Type Controller::getType() const
  {
    return mType;
  }

  const ControllerState& Controller::getState() const
  {
    return mState;
  }

  Controller::~Controller()
  {
    //
  }

}