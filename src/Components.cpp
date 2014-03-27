#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  Button::Button(): pushed( false )
  {
  }

  Axis::Axis(): absolute( NIL_REAL_ZERO )
  {
  }

  Slider::Slider(): absolute( NIL_REAL_ZERO, NIL_REAL_ZERO )
  {
  }

  POV::POV(): direction( POV::Centered )
  {
  }

  Wheel::Wheel(): relative( 0 )
  {
  }

  Movement::Movement(): relative( 0, 0 )
  {
  }

}