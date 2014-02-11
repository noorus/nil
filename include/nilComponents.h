#pragma once

#include "nilTypes.h"

namespace nil {

  // Components

  //! \struct Button
  //! Digital push button component.
  struct Button
  {
    public:
      bool pushed; //!< Pushed state
      Button();
  };

  //! \struct Axis
  //! Analog axis controller component.
  struct Axis
  {
    public:
      Real absolute; //!< Absolute value in {-1..1}
      Axis();
  };

  //! \struct Slider
  //! Two-dimensional analog controller component.
  struct Slider
  {
    public:
      Vector2f absolute; //!< Absolute value in [{-1..1},{-1..1}]
      Slider();
  };

  //! \struct POV
  //! Digital directional Point-of-View controller component.
  //! Also known as the D-pad.
  struct POV
  {
    public:
      static const POVDirection Centered   = 0x00000000;
      static const POVDirection North      = 0x00000001;
      static const POVDirection South      = 0x00000010;
      static const POVDirection East       = 0x00000100;
      static const POVDirection West       = 0x00001000;
      static const POVDirection NorthEast  = 0x00000101;
      static const POVDirection SouthEast  = 0x00000110;
      static const POVDirection NorthWest  = 0x00001001;
      static const POVDirection SouthWest  = 0x00001010;
      POVDirection direction; //!< Absolute current directions
      POV();
  };

  //! \struct Wheel
  //! Mouse wheel component.
  struct Wheel
  {
    public:
      int relative; //!< Wheel rotation delta, in eights of a degree
      Wheel();
  };

  //! \struct Movement
  //! Mouse movement component.
  struct Movement
  {
    public:
      Vector2i relative; //!< Relative value delta in points
      Movement();
  };

}