#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

# define NIL_LOAD_SDK_FUNC(x) mFunctions.pfn##x##=(fn##x##)GetProcAddress(mModule,#x)

namespace nil {

  namespace Logitech {

    const wchar_t* cLogitechGKeyModuleName = L"LogitechGkey.dll";

    GKeySDK::Functions::Functions():
    pfnLogiGkeyInit( nullptr ),
    pfnLogiGkeyGetMouseButtonString( nullptr ),
    pfnLogiGkeyGetKeyboardGkeyString( nullptr ),
    pfnLogiGkeyShutdown( nullptr )
    {
    }

    GKeySDK::GKeySDK(): ExternalModule()
    {
      InitializeSRWLock( &mLock );
    }

    void GKeySDK::addListener( GKeyListener* listener )
    {
      mListeners.push_back( listener );
    }

    void GKeySDK::removeListener( GKeyListener* listener )
    {
      mListeners.remove( listener );
    }

    GKeySDK::InitReturn GKeySDK::initialize()
    {
      mModule = LoadLibraryW( cLogitechGKeyModuleName );
      if ( !mModule )
        return Initialization_ModuleNotFound;

      NIL_LOAD_SDK_FUNC( LogiGkeyInit );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetMouseButtonString );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetKeyboardGkeyString );
      NIL_LOAD_SDK_FUNC( LogiGkeyShutdown );

      if ( !mFunctions.pfnLogiGkeyInit
        || !mFunctions.pfnLogiGkeyGetMouseButtonString
        || !mFunctions.pfnLogiGkeyGetKeyboardGkeyString
        || !mFunctions.pfnLogiGkeyShutdown )
        return Initialization_MissingExports;

      mContext.gkeyContext = this;
      mContext.gkeyCallBack = keyCallback;

      if ( !mFunctions.pfnLogiGkeyInit( &mContext ) )
        return Initialization_Unavailable;

      mInitialized = true;

      return Initialization_OK;
    }

    void GKeySDK::keyCallback( GkeyCode key, const wchar_t* name, void* context )
    {
      auto sdk = reinterpret_cast<GKeySDK*>( context );
      AcquireSRWLockExclusive( &sdk->mLock );
      sdk->mQueue.push( key );
      ReleaseSRWLockExclusive( &sdk->mLock );
    }

    void GKeySDK::update()
    {
      AcquireSRWLockExclusive( &mLock );
      while ( !mQueue.empty() )
      {
        for ( auto listener : mListeners )
        {
          if ( mQueue.front().keyDown )
            listener->onGKeyPressed( mQueue.front().keyIdx );
          else
            listener->onGKeyReleased( mQueue.front().keyIdx );
        }
        mQueue.pop();
      }
      ReleaseSRWLockExclusive( &mLock );
    }

    void GKeySDK::shutdown()
    {
      AcquireSRWLockExclusive( &mLock );
      if ( mModule )
      {
        if ( mInitialized )
          mFunctions.pfnLogiGkeyShutdown();
        FreeLibrary( mModule );
        mModule = NULL;
      }
      mInitialized = false;
      ReleaseSRWLockExclusive( &mLock );
    }

    GKeySDK::~GKeySDK()
    {
      shutdown();
    }

  }

}