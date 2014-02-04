#pragma once
#include <windows.h>
#include <LogitechGkey.h>
#include <LogitechLed.h>

namespace nil {

  namespace Logitech {

    // G-Key SDK (1.02.004)

    typedef unsigned int GKey;

    class GKeyListener {
    public:
      virtual void onGKeyPressed( GKey key ) = 0;
      virtual void onGKeyReleased( GKey key ) = 0;
    };

    typedef list<GKeyListener*> GKeyListenerList;

    typedef BOOL (*fnLogiGkeyInit)( logiGkeyCBContext* context );
    typedef wchar_t* (*fnLogiGkeyGetMouseButtonString)( int button );
    typedef wchar_t* (*fnLogiGkeyGetKeyboardGkeyString)( int key, int mode );
    typedef void (*fnLogiGkeyShutdown)();

    typedef queue<GkeyCode> GKeyQueue;

    class GKeySDK: public ExternalModule {
    protected:
      struct Functions {
        fnLogiGkeyInit pfnLogiGkeyInit;
        fnLogiGkeyGetMouseButtonString pfnLogiGkeyGetMouseButtonString;
        fnLogiGkeyGetKeyboardGkeyString pfnLogiGkeyGetKeyboardGkeyString;
        fnLogiGkeyShutdown pfnLogiGkeyShutdown;
        Functions();
      } mFunctions;
      logiGkeyCBContext mContext;
      SRWLOCK mLock;
      GKeyQueue mQueue;
      GKeyListenerList mListeners;
      static void __cdecl keyCallback( GkeyCode key, const wchar_t* name, void* context );
    public:
      GKeySDK();
      virtual InitReturn initialize();
      void update();
      virtual void shutdown();
      ~GKeySDK();
    };

    // LED SDK (1.01.005.1)

    typedef BOOL (*fnLogiLedInit)();
    typedef BOOL (*fnLogiLedSaveCurrentLighting)( int deviceType );
    typedef BOOL (*fnLogiLedSetLighting)( int deviceType, int redPercentage, int greenPercentage, int bluePercentage );
    typedef BOOL (*fnLogiLedRestoreLighting)( int deviceType );
    typedef void (*fnLogiLedShutdown)();

    class LedSDK: public ExternalModule {
    protected:
      struct Functions {
        fnLogiLedInit pfnLogiLedInit;
        fnLogiLedSaveCurrentLighting pfnLogiLedSaveCurrentLighting;
        fnLogiLedSetLighting pfnLogiLedSetLighting;
        fnLogiLedRestoreLighting pfnLogiLedRestoreLighting;
        fnLogiLedShutdown pfnLogiLedShutdown;
        Functions();
      } mFunctions;
      bool mSavedOriginal;
    public:
      LedSDK();
      virtual InitReturn initialize();
      void setLighting( const Color& color );
      virtual void shutdown();
      ~LedSDK();
    };

  }

}