#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Button::Button(): pushed( false )
  {
  }

  Axis::Axis(): absolute( 0 )
  {
  }

  Slider::Slider(): absolute( 0, 0 )
  {
  }

  POV::POV(): direction( POV::Centered )
  {
  }

}