#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

# define NIL_LOAD_EXTERNAL_FUNC(x) funcs_.pfn##x##=(fn##x##)GetProcAddress(module_,#x)

namespace nil {

  XInput::Functions::Functions(): pfnXInputGetState( nullptr ),
  pfnXInputSetState( nullptr ), pfnXInputGetCapabilities( nullptr )
  {
  }

  XInput::XInput(): version_( Version_None )
  {
  }

  XInput::InitReturn XInput::initialize()
  {
    module_ = LoadLibraryW( L"xinput1_4.dll" );
    if ( module_ )
      version_ = Version_14;
    else
    {
      module_ = LoadLibraryW( L"xinput1_3.dll" );
      if ( module_ )
        version_ = Version_13;
      else
      {
        module_ = LoadLibraryW( L"xinput9_1_0.dll" );
        if ( module_ )
          version_ = Version_910;
        else
          return Initialization_ModuleNotFound;
      }
    }

    NIL_LOAD_EXTERNAL_FUNC( XInputGetState );
    NIL_LOAD_EXTERNAL_FUNC( XInputSetState );
    NIL_LOAD_EXTERNAL_FUNC( XInputGetCapabilities );

    if ( !funcs_.pfnXInputGetState
      || !funcs_.pfnXInputSetState
      || !funcs_.pfnXInputGetCapabilities )
      return Initialization_MissingExports;

    isInitialized_ = true;

    return Initialization_OK;
  }

  void XInput::shutdown()
  {
    if ( module_ )
    {
      FreeLibrary( module_ );
      module_ = nullptr;
    }
    isInitialized_ = false;
  }

  XInput::~XInput()
  {
    shutdown();
  }

}