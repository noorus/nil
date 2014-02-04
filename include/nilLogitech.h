#pragma once
#include "nil.h"

namespace nil {

  //! \class ExternalModule
  //! Base class for an external, optional module supported by the system.
  class ExternalModule {
  protected:
    HMODULE mModule;
    bool mInitialized;
  public:
    enum InitReturn: unsigned int {
      Initialization_OK = 0,
      Initialization_ModuleNotFound,
      Initialization_MissingExports,
      Initialization_Unavailable
    };
    ExternalModule();
    virtual InitReturn initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const;
  };

  namespace Logitech {

    // G-Key SDK (1.02.004)

#pragma pack( push, 1 )

    typedef struct
    {
      unsigned int keyIdx         : 8;        // index of the G key or mouse button, for example, 6 for G6 or Button 6
      unsigned int keyDown        : 1;        // key up or down, 1 is down, 0 is up
      unsigned int mState         : 2;        // mState (1, 2 or 3 for M1, M2 and M3)
      unsigned int mouse          : 1;        // indicate if the Event comes from a mouse, 1 is yes, 0 is no.
      unsigned int reserved1      : 4;        // reserved1
      unsigned int reserved2      : 16;       // reserved2
    } GkeyCode;

    typedef void (__cdecl *logiGkeyCB)( GkeyCode gkeyCode, const wchar_t* gkeyOrButtonString, void* context );

    typedef struct
    {
      logiGkeyCB gkeyCallBack;
      void* gkeyContext;
    } logiGkeyCBContext;

#pragma pack( pop )

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

    const int LOGITECH_LED_MOUSE = 0x0001;
    const int LOGITECH_LED_KEYBOARD = 0x0002;
    const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD;

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