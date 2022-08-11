#pragma once
#include "nilConfig.h"

#include "nilWindows.h"

namespace nil {

  namespace logitech {

    // Logitech G-Key SDK (1.02.004) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechGkeySDK_1.02.004/ for full SDK and libraries.

    //! \addtogroup Nil
    //! @{

    //! \addtogroup Logitech
    //! @{

#pragma pack( push, 1 )

    struct GkeyCode
    {
      unsigned int keyIdx    : 8; //!< index of the G key or mouse button, for example, 6 for G6 or Button 6
      unsigned int keyDown   : 1; //!< key up or down, 1 is down, 0 is up
      unsigned int mState    : 2; //!< mState (1, 2 or 3 for M1, M2 and M3)
      unsigned int mouse     : 1; //!< indicate if the Event comes from a mouse, 1 is yes, 0 is no.
      unsigned int reserved1 : 4; //!< reserved1
      unsigned int reserved2 : 16; //!< reserved2
    };

    using logiGkeyCB = void (__cdecl *)( GkeyCode gkeyCode, const wchar_t* gkeyOrButtonString, void* context );

    struct logiGkeyCBContext
    {
      logiGkeyCB gkeyCallBack;
      void* gkeyContext;
    };

#pragma pack( pop )

    //! G-key type.
    using GKey = unsigned int;

    //! \class GKeyListener
    //! A G-Key listener base class.
    //! Derive your own G-Key input listener from this class.
    class GKeyListener {
    public:
      //! G-Key press event.
      virtual void onGKeyPressed( GKey key ) = 0;

      //! G-Key release event.
      virtual void onGKeyReleased( GKey key ) = 0;
    };

    using GKeyListenerList = list<GKeyListener*>;

    using fnLogiGkeyInit = BOOL (*)( logiGkeyCBContext* context );
    using fnLogiGkeyGetMouseButtonString = wchar_t* (*)( int button );
    using fnLogiGkeyGetKeyboardGkeyString = wchar_t* (*)( int key, int mode );
    using fnLogiGkeyShutdown = void (*)();

    using GKeyQueue = queue<GkeyCode>;

    //! \class GKeySDK
    //! Logitech G-Key SDK module.
    //! \sa ExternalModule
    class GKeySDK: public ExternalModule {
    private:
      struct Functions {
        fnLogiGkeyInit pfnLogiGkeyInit;
        fnLogiGkeyGetMouseButtonString pfnLogiGkeyGetMouseButtonString;
        fnLogiGkeyGetKeyboardGkeyString pfnLogiGkeyGetKeyboardGkeyString;
        fnLogiGkeyShutdown pfnLogiGkeyShutdown;
        Functions();
      } funcs_; //!< G-Key SDK's import functions

      logiGkeyCBContext context_; //!< The context
      SRWLOCK lock_; //!< The lock
      GKeyQueue queue_; //!< The queue
      GKeyListenerList listeners_; //!< The listeners

      static void __cdecl keyCallback( GkeyCode key, const wchar_t* name, void* context );
    public:
      GKeySDK();

      //! Try to initialize the G-Key SDK module.
      InitReturn initialize() override;

      //! Add a G-Key event listener.
      void addListener( GKeyListener* listener );

      //! Remove a G-Key event listener.
      void removeListener( GKeyListener* listener );

      //! Update the GKeySDK.
      void update();

      //! Shut down the G-Key SDK module.
      void shutdown() override;

      virtual ~GKeySDK();
    };

    // Logitech LED SDK (1.01.005.1) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechLEDSDK_1.01.005.1/ for full SDK and libraries.

    const int LOGITECH_LED_MOUSE = 0x0001;
    const int LOGITECH_LED_KEYBOARD = 0x0002;
    const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD;

    using fnLogiLedInit = BOOL (*)();
    using fnLogiLedSaveCurrentLighting = BOOL (*)( int deviceType );
    using fnLogiLedSetLighting = BOOL (*)( int deviceType, int redPercentage, int greenPercentage, int bluePercentage );
    using fnLogiLedRestoreLighting = BOOL (*)( int deviceType );
    using fnLogiLedShutdown = void (*)();

    //! \class LedSDK
    //! Logitech Led SDK module.
    //! \sa ExternalModule
    class LedSDK: public ExternalModule {
    private:
      struct Functions {
        fnLogiLedInit pfnLogiLedInit;
        fnLogiLedSaveCurrentLighting pfnLogiLedSaveCurrentLighting;
        fnLogiLedSetLighting pfnLogiLedSetLighting;
        fnLogiLedRestoreLighting pfnLogiLedRestoreLighting;
        fnLogiLedShutdown pfnLogiLedShutdown;
        Functions();
      } funcs_; //!< Led SDK's import functions

      bool isOriginalSaved_; //!< Whether original LED state was saved on init or not
    public:
      LedSDK();

      //! Try to initialize the Led SDK module.
      InitReturn initialize() override;

      //! Set LED lighting to given color value on supported hardware.
      void setLighting( const Color& color );

      //! Shut down the Led SDK module.
      void shutdown() override;

      virtual ~LedSDK();
    };

    //! @}

    //! @}

  }

}