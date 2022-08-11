#pragma once
#include "nilConfig.h"

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
    bool pushed = false; //!< Pushed state
  };

  //! \struct Axis
  //! Analog axis controller component.
  struct Axis
  {
    Real absolute = NIL_REAL_ZERO; //!< Absolute value in {-1..1}
  };

  //! \struct Slider
  //! Two-dimensional analog controller component.
  struct Slider
  {
    Vector2f absolute = Vector2f( NIL_REAL_ZERO, NIL_REAL_ZERO ); //!< Absolute value in [{-1..1},{-1..1}]
  };

  //! \struct POV
  //! Digital directional Point-of-View controller component.
  //! Also known as the D-pad.
  struct POV
  {
    static const POVDirection Centered   = 0x00000000; //!< Quick direction constant, centered
    static const POVDirection North      = 0x00000001; //!< Quick direction constant, pointing north
    static const POVDirection South      = 0x00000010; //!< Quick direction constant, pointing south
    static const POVDirection East       = 0x00000100; //!< Quick direction constant, pointing east
    static const POVDirection West       = 0x00001000; //!< Quick direction constant, pointing west
    static const POVDirection NorthEast  = 0x00000101; //!< Quick direction constant, pointing northeast
    static const POVDirection SouthEast  = 0x00000110; //!< Quick direction constant, pointing southeast
    static const POVDirection NorthWest  = 0x00001001; //!< Quick direction constant, pointing northwest
    static const POVDirection SouthWest  = 0x00001010; //!< Quick direction constant, pointing southwest
    POVDirection direction = Centered; //!< Absolute current direction
  };

  //! \struct Wheel
  //! Mouse wheel component.
  struct Wheel
  {
    int relative = 0; //!< Wheel rotation delta, in eights of a degree
  };

  //! \struct Movement
  //! Mouse movement component.
  struct Movement
  {
    Vector2i relative = Vector2i( 0, 0 ); //!< Relative value delta in points
  };

  //! @}

  //! @}

}