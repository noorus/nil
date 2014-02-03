#pragma once
#include <windows.h>
#include <LogitechGkey.h>

namespace nil {

  namespace Logitech {

    // G-Key SDK

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

    class GKeySDK {
    protected:
      HMODULE mModule;
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
      void update();
      ~GKeySDK();
    };

  }

}