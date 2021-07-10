#pragma once

#include "nilTypes.h"

namespace nil {

  //! \addtogroup Nil
  //! @{

  //! \addtogroup Components
  //! @{

  //! \struct Button
  //! Digital push button component.
  struct Button
  {
    bool pushed; //!< Pushed state
    Button();
  };

  //! \struct Axis
  //! Analog axis controller component.
  struct Axis
  {
    Real absolute; //!< Absolute value in {-1..1}
    Axis();
  };

  //! \struct Slider
  //! Two-dimensional analog controller component.
  struct Slider
  {
    Vector2f absolute; //!< Absolute value in [{-1..1},{-1..1}]
    Slider();
  };

  //! \struct POV
  //! Digital directional Point-of-View controller component.
  //! Also known as the D-pad.
  struct POV
  {
    public:
      static const POVDirection Centered   = 0x00000000; //!< Quick direction constant, centered
      static const POVDirection North      = 0x00000001; //!< Quick direction constant, pointing north
      static const POVDirection South      = 0x00000010; //!< Quick direction constant, pointing south
      static const POVDirection East       = 0x00000100; //!< Quick direction constant, pointing east
      static const POVDirection West       = 0x00001000; //!< Quick direction constant, pointing west
      static const POVDirection NorthEast  = 0x00000101; //!< Quick direction constant, pointing northeast
      static const POVDirection SouthEast  = 0x00000110; //!< Quick direction constant, pointing southeast
      static const POVDirection NorthWest  = 0x00001001; //!< Quick direction constant, pointing northwest
      static const POVDirection SouthWest  = 0x00001010; //!< Quick direction constant, pointing southwest
      POVDirection direction; //!< Absolute current direction
      POV();
  };

  //! \struct Wheel
  //! Mouse wheel component.
  struct Wheel
  {
    int relative; //!< Wheel rotation delta, in eights of a degree
    Wheel();
  };

  //! \struct Movement
  //! Mouse movement component.
  struct Movement
  {
    Vector2i relative; //!< Relative value delta in points
    Movement();
  };

  //! @}

  //! @}

}