#include "nil.h"
#include "nilUtil.h"

namespace nil {

  ControllerState::ControllerState()
  {
    clear();
  }

  void ControllerState::clear()
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
          listener->onButtonPressed( mState, i );
        else if ( lastState.mButtons[i].pushed && !mState.mButtons[i].pushed )
          listener->onButtonReleased( mState, i );

      // Axes
      for ( size_t i = 0; i < mState.mAxes.size(); i++ )
        if ( lastState.mAxes[i].absolute != mState.mAxes[i].absolute )
          listener->onAxisMoved( mState, i );

      // Sliders
      for ( size_t i = 0; i < mState.mSliders.size(); i++ )
        if ( lastState.mSliders[i].absolute != mState.mSliders[i].absolute )
          listener->onSliderMoved( mState, i );

      // POVs
      for ( size_t i = 0; i < mState.mPOVs.size(); i++ )
        if ( lastState.mPOVs[i].direction != mState.mPOVs[i].direction )
          listener->onPOVMoved( mState, i );
    }
  }

  const ControllerState& Controller::getState() const
  {
    return mState;
  }

  Controller::~Controller()
  {
    //
  }

  const Controller::Type Controller::getType()
  {
    return mType;
  }

}