#include "nil.h"

namespace nil {

  RawInputKeyboard::RawInputKeyboard( RawInputDevice* device ):
  Keyboard( device->getSystem(), device )
  {
    device->getSystem()->mapKeyboard( device->getRawHandle(), this );
  }

  void RawInputKeyboard::onRawInput( const RAWKEYBOARD& input )
  {
    //
  }

  void RawInputKeyboard::update()
  {
    //
  }

  RawInputKeyboard::~RawInputKeyboard()
  {
    auto rawDevice = static_cast<RawInputDevice*>( mDevice );
    rawDevice->getSystem()->unmapKeyboard( rawDevice->getRawHandle() );
  }

}