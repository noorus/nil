#include "nil.h"
#include "nilLogitech.h"
#include "nilUtil.h"

# define NIL_LOAD_SDK_FUNC(x) mFunctions.pfn##x##=(fn##x##)GetProcAddress(mModule,#x)

namespace nil {

  namespace Logitech {

    class DummyGKeyListener: public GKeyListener {
    public:
      virtual void onGKeyPressed( GKey key )
      {
        wprintf_s( L"G-Key pressed: %d\r\n", key );
      }
      virtual void onGKeyReleased( GKey key )
      {
        wprintf_s( L"G-Key released: %d\r\n", key );
      }
    };

    DummyGKeyListener gDummyGKeyListener;

    const wchar_t* cLogitechGKeyModuleName = L"LogitechGkey.dll";

    GKeySDK::Functions::Functions():
    pfnLogiGkeyInit( nullptr ),
    pfnLogiGkeyGetMouseButtonString( nullptr ),
    pfnLogiGkeyGetKeyboardGkeyString( nullptr ),
    pfnLogiGkeyShutdown( nullptr )
    {
    }

    GKeySDK::GKeySDK(): mModule( NULL )
    {
      InitializeSRWLock( &mLock );

      mListeners.push_back( &gDummyGKeyListener );

      mModule = LoadLibraryW( cLogitechGKeyModuleName );
      if ( !mModule )
        NIL_EXCEPT_WINAPI( L"Couldn't load Logitech GKey module" );

      NIL_LOAD_SDK_FUNC( LogiGkeyInit );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetMouseButtonString );
      NIL_LOAD_SDK_FUNC( LogiGkeyGetKeyboardGkeyString );
      NIL_LOAD_SDK_FUNC( LogiGkeyShutdown );

      if ( !mFunctions.pfnLogiGkeyInit
        || !mFunctions.pfnLogiGkeyGetMouseButtonString
        || !mFunctions.pfnLogiGkeyGetKeyboardGkeyString
        || !mFunctions.pfnLogiGkeyShutdown )
        NIL_EXCEPT( L"Couldn't load Logitech GKey module" );

      mContext.gkeyContext = this;
      mContext.gkeyCallBack = keyCallback;

      if ( !mFunctions.pfnLogiGkeyInit( &mContext ) )
        NIL_EXCEPT( L"Couldn't initialize Logitech GKey module" );
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

    GKeySDK::~GKeySDK()
    {
      if ( mModule )
      {
        if ( mFunctions.pfnLogiGkeyShutdown )
          mFunctions.pfnLogiGkeyShutdown();
        FreeLibrary( mModule );
      }
    }

  }

}