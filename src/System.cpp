#include "nil.h"
#include "nilUtil.h"

namespace nil {

  System::System( HWND window ): mDirectInput( nullptr ),
  mWindow( window ), mInstance( 0 )
  {
    if ( !IsWindow( mWindow ) )
      EXCEPT( L"Passed window handle is invalid" );

    mInstance = GetModuleHandleW( nullptr );

    HRESULT hr = DirectInput8Create( mInstance, DIRECTINPUT_VERSION,
      IID_IDirectInput8, (LPVOID*)&mDirectInput, NULL );

    if ( FAILED( hr ) )
      EXCEPT_DINPUT( hr, L"Could not instance DirectInput 8" );
  }

  System::~System()
  {
    SAFE_RELEASE( mDirectInput );
  }

}