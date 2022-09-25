#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

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

    funcs_.pfnXInputGetState = (fnXInputGetState)GetProcAddress( module_, "XInputGetState" );
    funcs_.pfnXInputSetState = (fnXInputSetState)GetProcAddress( module_, "XInputSetState" );
    funcs_.pfnXInputGetCapabilities = (fnXInputGetCapabilities)GetProcAddress( module_, "XInputGetCapabilities" );

    if ( !funcs_.pfnXInputGetState
      || !funcs_.pfnXInputSetState
      || !funcs_.pfnXInputGetCapabilities )
      return Initialization_MissingExports;

    initialized_ = true;

    return Initialization_OK;
  }

  void XInput::shutdown()
  {
    if ( module_ )
    {
      FreeLibrary( module_ );
      module_ = nullptr;
    }
    initialized_ = false;
  }

  XInput::~XInput()
  {
    shutdown();
  }

}

#endif