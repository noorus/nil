#include "nilConfig.h"

#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

#ifdef NIL_PLATFORM_WINDOWS

# define NIL_LOAD_SDK_FUNC(x) funcs_.pfn##x##=(fn##x##)GetProcAddress(module_,#x)

namespace nil {

  namespace logitech {

    const wchar_t* cLogitechLedModuleName = L"LogitechLed.dll";

    LedSDK::Functions::Functions():
    pfnLogiLedInit( nullptr ),
    pfnLogiLedSaveCurrentLighting( nullptr ),
    pfnLogiLedSetLighting( nullptr ),
    pfnLogiLedRestoreLighting( nullptr ),
    pfnLogiLedShutdown( nullptr )
    {
    }

    LedSDK::LedSDK(): isOriginalSaved_( false )
    {
      //
    }

    LedSDK::InitReturn LedSDK::initialize()
    {
      module_ = LoadLibraryW( cLogitechLedModuleName );
      if ( !module_ )
        return Initialization_ModuleNotFound;

      funcs_.pfnLogiLedInit = (fnLogiLedInit)GetProcAddress( module_, "LogiLedInit" );
      funcs_.pfnLogiLedSaveCurrentLighting = (fnLogiLedSaveCurrentLighting)GetProcAddress( module_, "LogiLedSaveCurrentLighting" );
      funcs_.pfnLogiLedSetLighting = (fnLogiLedSetLighting)GetProcAddress( module_, "LogiLedSetLighting" );
      funcs_.pfnLogiLedRestoreLighting = (fnLogiLedRestoreLighting)GetProcAddress( module_, "LogiLedRestoreLighting" );
      funcs_.pfnLogiLedShutdown = (fnLogiLedShutdown)GetProcAddress( module_, "LogiLedShutdown" );

      if ( !funcs_.pfnLogiLedInit
        || !funcs_.pfnLogiLedSaveCurrentLighting
        || !funcs_.pfnLogiLedSetLighting
        || !funcs_.pfnLogiLedRestoreLighting
        || !funcs_.pfnLogiLedShutdown )
        return Initialization_MissingExports;

      if ( !funcs_.pfnLogiLedInit() )
        return Initialization_Unavailable;

      if ( funcs_.pfnLogiLedSaveCurrentLighting( LOGITECH_LED_ALL ) )
        isOriginalSaved_ = true;

      isInitialized_ = true;

      return Initialization_OK;
    }

    void LedSDK::setLighting( const Color& color )
    {
      if ( !isInitialized_ )
        return;

      auto r = static_cast<int>( color.r * 100.0f );
      auto g = static_cast<int>( color.g * 100.0f );
      auto b = static_cast<int>( color.b * 100.0f );

      funcs_.pfnLogiLedSetLighting( LOGITECH_LED_ALL, r, g, b );
    }

    void LedSDK::shutdown()
    {
      if ( module_ )
      {
        if ( isOriginalSaved_ )
          funcs_.pfnLogiLedRestoreLighting( LOGITECH_LED_ALL );
        if ( isInitialized_ )
          funcs_.pfnLogiLedShutdown();
        FreeLibrary( module_ );
        module_ = nullptr;
      }
      isInitialized_ = false;
      isOriginalSaved_ = false;
    }

    LedSDK::~LedSDK()
    {
      shutdown();
    }

  }

}

#endif