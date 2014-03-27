#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

# define NIL_LOAD_SDK_FUNC(x) mFunctions.pfn##x##=(fn##x##)GetProcAddress(mModule,#x)

namespace Nil {

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
      mModule = LoadLibraryW( cLogitechLedModuleName );
      if ( !mModule )
        return Initialization_ModuleNotFound;

      NIL_LOAD_SDK_FUNC( LogiLedInit );
      NIL_LOAD_SDK_FUNC( LogiLedSaveCurrentLighting );
      NIL_LOAD_SDK_FUNC( LogiLedSetLighting );
      NIL_LOAD_SDK_FUNC( LogiLedRestoreLighting );
      NIL_LOAD_SDK_FUNC( LogiLedShutdown );

      if ( !mFunctions.pfnLogiLedInit
        || !mFunctions.pfnLogiLedSaveCurrentLighting
        || !mFunctions.pfnLogiLedSetLighting
        || !mFunctions.pfnLogiLedRestoreLighting
        || !mFunctions.pfnLogiLedShutdown )
        return Initialization_MissingExports;

      if ( !mFunctions.pfnLogiLedInit() )
        return Initialization_Unavailable;

      if ( mFunctions.pfnLogiLedSaveCurrentLighting( LOGITECH_LED_ALL ) )
        mSavedOriginal = true;

      mInitialized = true;

      return Initialization_OK;
    }

    void LedSDK::setLighting( const Color& color )
    {
      if ( !mInitialized )
        return;

      int r = (int)( color.r * 100.0f );
      int g = (int)( color.g * 100.0f );
      int b = (int)( color.b * 100.0f );

      mFunctions.pfnLogiLedSetLighting( LOGITECH_LED_ALL, r, g, b );
    }

    void LedSDK::shutdown()
    {
      if ( mModule )
      {
        if ( mSavedOriginal )
          mFunctions.pfnLogiLedRestoreLighting( LOGITECH_LED_ALL );
        if ( mInitialized )
          mFunctions.pfnLogiLedShutdown();
        FreeLibrary( mModule );
        mModule = NULL;
      }
      mInitialized = false;
      mSavedOriginal = false;
    }

    LedSDK::~LedSDK()
    {
      shutdown();
    }

  }

}