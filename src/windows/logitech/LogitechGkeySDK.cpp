#include "nilConfig.h"

#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

#ifdef NIL_PLATFORM_WINDOWS

# define NIL_LOAD_SDK_FUNC(x) funcs_.pfn##x##=(fn##x##)GetProcAddress(module_,#x)

namespace nil {

  namespace logitech {

    const wchar_t* cLogitechGKeyModuleName = L"LogitechGkey.dll";

    GKeySDK::Functions::Functions():
    pfnLogiGkeyInit( nullptr ),
    pfnLogiGkeyGetMouseButtonString( nullptr ),
    pfnLogiGkeyGetKeyboardGkeyString( nullptr ),
    pfnLogiGkeyShutdown( nullptr )
    {
    }

    GKeySDK::GKeySDK()
    {
      InitializeSRWLock( &lock_ );
    }

    void GKeySDK::addListener( GKeyListener* listener )
    {
      listeners_.push_back( listener );
    }

    void GKeySDK::removeListener( GKeyListener* listener )
    {
      listeners_.remove( listener );
    }

    GKeySDK::InitReturn GKeySDK::initialize()
    {
      module_ = LoadLibraryW( cLogitechGKeyModuleName );
      if ( !module_ )
        return Initialization_ModuleNotFound;

      NIL_LOAD_SDK_FUNC( LogiGkeyInit );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetMouseButtonString );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetKeyboardGkeyString );
      NIL_LOAD_SDK_FUNC( LogiGkeyShutdown );

      if ( !funcs_.pfnLogiGkeyInit
        || !funcs_.pfnLogiGkeyGetMouseButtonString
        || !funcs_.pfnLogiGkeyGetKeyboardGkeyString
        || !funcs_.pfnLogiGkeyShutdown )
        return Initialization_MissingExports;

      context_.gkeyContext = this;
      context_.gkeyCallBack = keyCallback;

      if ( !funcs_.pfnLogiGkeyInit( &context_ ) )
        return Initialization_Unavailable;

      isInitialized_ = true;

      return Initialization_OK;
    }

    void GKeySDK::keyCallback( GkeyCode key, const wchar_t* name, void* context )
    {
      UNREFERENCED_PARAMETER( name );

      auto sdk = reinterpret_cast<GKeySDK*>( context );

      ScopedSRWLock lock( &sdk->lock_ );

      sdk->queue_.push( key );
    }

    void GKeySDK::update()
    {
      ScopedSRWLock lock( &lock_ );

      while ( !queue_.empty() )
      {
        for ( auto listener : listeners_ )
        {
          if ( queue_.front().keyDown )
            listener->onGKeyPressed( queue_.front().keyIdx );
          else
            listener->onGKeyReleased( queue_.front().keyIdx );
        }
        queue_.pop();
      }
    }

    void GKeySDK::shutdown()
    {
      ScopedSRWLock lock( &lock_ );

      if ( module_ )
      {
        if ( isInitialized_ )
          funcs_.pfnLogiGkeyShutdown();
        FreeLibrary( module_ );
        module_ = nullptr;
      }

      isInitialized_ = false;
    }

    GKeySDK::~GKeySDK()
    {
      shutdown();
    }

  }

}

#endif