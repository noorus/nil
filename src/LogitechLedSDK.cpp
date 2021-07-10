#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

# define NIL_LOAD_SDK_FUNC(x) funcs_.pfn##x##=(fn##x##)GetProcAddress(module_,#x)

namespace nil {

  namespace Logitech {

    const wchar_t* cLogitechLedModuleName = L"LogitechLed.dll";

    LedSDK::Functions::Functions():
    pfnLogiLedInit( nullptr ),
    pfnLogiLedSaveCurrentLighting( nullptr ),
    pfnLogiLedSetLighting( nullptr ),
    pfnLogiLedRestoreLighting( nullptr ),
    pfnLogiLedShutdown( nullptr )
    {
    }

    LedSDK::LedSDK(): ExternalModule(), mSavedOriginal( false )
    {
      //
    }

    LedSDK::InitReturn LedSDK::initialize()
    {
      module_ = LoadLibraryW( cLogitechLedModuleName );
      if ( !module_ )
        return Initialization_ModuleNotFound;

      NIL_LOAD_SDK_FUNC( LogiLedInit );
      NIL_LOAD_SDK_FUNC( LogiLedSaveCurrentLighting );
      NIL_LOAD_SDK_FUNC( LogiLedSetLighting );
      NIL_LOAD_SDK_FUNC( LogiLedRestoreLighting );
      NIL_LOAD_SDK_FUNC( LogiLedShutdown );

      if ( !funcs_.pfnLogiLedInit
        || !funcs_.pfnLogiLedSaveCurrentLighting
        || !funcs_.pfnLogiLedSetLighting
        || !funcs_.pfnLogiLedRestoreLighting
        || !funcs_.pfnLogiLedShutdown )
        return Initialization_MissingExports;

      if ( !funcs_.pfnLogiLedInit() )
        return Initialization_Unavailable;

      if ( funcs_.pfnLogiLedSaveCurrentLighting( LOGITECH_LED_ALL ) )
        mSavedOriginal = true;

      isInitialized_ = true;

      return Initialization_OK;
    }

    void LedSDK::setLighting( const Color& color )
    {
      if ( !isInitialized_ )
        return;

      int r = (int)( color.r * 100.0f );
      int g = (int)( color.g * 100.0f );
      int b = (int)( color.b * 100.0f );

      funcs_.pfnLogiLedSetLighting( LOGITECH_LED_ALL, r, g, b );
    }

    void LedSDK::shutdown()
    {
      if ( module_ )
      {
        if ( mSavedOriginal )
          funcs_.pfnLogiLedRestoreLighting( LOGITECH_LED_ALL );
        if ( isInitialized_ )
          funcs_.pfnLogiLedShutdown();
        FreeLibrary( module_ );
        module_ = nullptr;
      }
      isInitialized_ = false;
      mSavedOriginal = false;
    }

    LedSDK::~LedSDK()
    {
      shutdown();
    }

  }

}