#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Component::Component( Type type ): mType( type )
  {
    //
  }

  Button::Button(): Component( Component::Button ), mPushed( false )
  {
    //
  }

  Axis::Axis(): Component( Component::Axis ), mAbsolute( 0 )
  {
    //
  }

  Slider::Slider(): Component( Component::Slider )
  {
    //
  }

}