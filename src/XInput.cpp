#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

# define NIL_LOAD_EXTERNAL_FUNC(x) mFunctions.pfn##x##=(fn##x##)GetProcAddress(mModule,#x)

namespace Nil {

  XInput::Functions::Functions(): pfnXInputGetState( nullptr ),
  pfnXInputSetState( nullptr ), pfnXInputGetCapabilities( nullptr )
  {
  }

  XInput::XInput(): mVersion( Version_None )
  {
  }

  XInput::InitReturn XInput::initialize()
  {
    mModule = LoadLibraryW( L"xinput1_4.dll" );
    if ( mModule )
      mVersion = Version_14;
    else
    {
      mModule = LoadLibraryW( L"xinput1_3.dll" );
      if ( mModule )
        mVersion = Version_13;
      else
      {
        mModule = LoadLibraryW( L"xinput9_1_0.dll" );
        if ( mModule )
          mVersion = Version_910;
        else
          return Initialization_ModuleNotFound;
      }
    }

    NIL_LOAD_EXTERNAL_FUNC( XInputGetState );
    NIL_LOAD_EXTERNAL_FUNC( XInputSetState );
    NIL_LOAD_EXTERNAL_FUNC( XInputGetCapabilities );

    if ( !mFunctions.pfnXInputGetState
      || !mFunctions.pfnXInputSetState
      || !mFunctions.pfnXInputGetCapabilities )
      return Initialization_MissingExports;

    mInitialized = true;

    return Initialization_OK;
  }

  void XInput::shutdown()
  {
    if ( mModule )
    {
      FreeLibrary( mModule );
      mModule = NULL;
    }
    mInitialized = false;
  }

  XInput::~XInput()
  {
    shutdown();
  }

}