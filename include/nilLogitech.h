#pragma once
#include "nilWindows.h"

namespace Nil {

  namespace Logitech {

    // Logitech G-Key SDK (1.02.004) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechGkeySDK_1.02.004/ for full SDK and libraries.

    //! \addtogroup Nil
    //! @{

    //! \addtogroup Logitech
    //! @{

#pragma pack( push, 1 )

    typedef struct
    {
      unsigned int keyIdx         : 8;  //!< index of the G key or mouse button, for example, 6 for G6 or Button 6
      unsigned int keyDown        : 1;  //!< key up or down, 1 is down, 0 is up
      unsigned int mState         : 2;  //!< mState (1, 2 or 3 for M1, M2 and M3)
      unsigned int mouse          : 1;  //!< indicate if the Event comes from a mouse, 1 is yes, 0 is no.
      unsigned int reserved1      : 4;  //!< reserved1
      unsigned int reserved2      : 16; //!< reserved2
    } GkeyCode;

    typedef void (__cdecl *logiGkeyCB)( GkeyCode gkeyCode, const wchar_t* gkeyOrButtonString, void* context );

    typedef struct
    {
      logiGkeyCB gkeyCallBack;
      void* gkeyContext;
    } logiGkeyCBContext;

#pragma pack( pop )

    //! G-key type.
    typedef unsigned int GKey;

    //! \class GKeyListener
    //! A G-Key listener base class.
    //! Derive your own G-Key input listener from this class.
    class GKeyListener
    {
      public:
        //! G-Key press event.
        virtual void onGKeyPressed( GKey key ) = 0;

        //! G-Key release event.
        virtual void onGKeyReleased( GKey key ) = 0;
    };

    typedef list<GKeyListener*> GKeyListenerList;

    typedef BOOL (*fnLogiGkeyInit)( logiGkeyCBContext* context );
    typedef wchar_t* (*fnLogiGkeyGetMouseButtonString)( int button );
    typedef wchar_t* (*fnLogiGkeyGetKeyboardGkeyString)( int key, int mode );
    typedef void (*fnLogiGkeyShutdown)();

    typedef queue<GkeyCode> GKeyQueue;

    //! \class GKeySDK
    //! Logitech G-Key SDK module.
    //! \sa ExternalModule
    class GKeySDK: public ExternalModule
    {
      private:
        struct Functions {
          fnLogiGkeyInit pfnLogiGkeyInit;
          fnLogiGkeyGetMouseButtonString pfnLogiGkeyGetMouseButtonString;
          fnLogiGkeyGetKeyboardGkeyString pfnLogiGkeyGetKeyboardGkeyString;
          fnLogiGkeyShutdown pfnLogiGkeyShutdown;
          Functions();
        } mFunctions; //!< G-Key SDK's import functions

        logiGkeyCBContext mContext; //!< The context
        SRWLOCK mLock;  //!< The lock
        GKeyQueue mQueue; //!< The queue
        GKeyListenerList mListeners;  //!< The listeners

        static void __cdecl keyCallback( GkeyCode key, const wchar_t* name, void* context );
      public:
        GKeySDK();

        //! Try to initialize the G-Key SDK module.
        virtual InitReturn initialize();

        //! Add a G-Key event listener.
        void addListener( GKeyListener* listener );

        //! Remove a G-Key event listener.
        void removeListener( GKeyListener* listener );

        //! Update the GKeySDK.
        void update();

        //! Shut down the G-Key SDK module.
        virtual void shutdown();

        ~GKeySDK();
    };

    // Logitech LED SDK (1.01.005.1) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechLEDSDK_1.01.005.1/ for full SDK and libraries.

    const int LOGITECH_LED_MOUSE = 0x0001;
    const int LOGITECH_LED_KEYBOARD = 0x0002;
    const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD;

    typedef BOOL (*fnLogiLedInit)();
    typedef BOOL (*fnLogiLedSaveCurrentLighting)( int deviceType );
    typedef BOOL (*fnLogiLedSetLighting)( int deviceType, int redPercentage, int greenPercentage, int bluePercentage );
    typedef BOOL (*fnLogiLedRestoreLighting)( int deviceType );
    typedef void (*fnLogiLedShutdown)();

    //! \class LedSDK
    //! Logitech Led SDK module.
    //! \sa ExternalModule
    class LedSDK: public ExternalModule
    {
      private:
        struct Functions {
          fnLogiLedInit pfnLogiLedInit;
          fnLogiLedSaveCurrentLighting pfnLogiLedSaveCurrentLighting;
          fnLogiLedSetLighting pfnLogiLedSetLighting;
          fnLogiLedRestoreLighting pfnLogiLedRestoreLighting;
          fnLogiLedShutdown pfnLogiLedShutdown;
          Functions();
        } mFunctions; //!< Led SDK's import functions

        bool mSavedOriginal; //!< Whether original LED state was saved on init or not
      public:
        LedSDK();

        //! Try to initialize the Led SDK module.
        virtual InitReturn initialize();

        //! Set LED lighting to given color value on supported hardware.
        void setLighting( const Color& color );

        //! Shut down the Led SDK module.
        virtual void shutdown();

        ~LedSDK();
    };

    //! @}

    //! @}

  }

}