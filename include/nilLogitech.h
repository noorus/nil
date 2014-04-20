#pragma once
#include "nil.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! \class ExternalModule
  //! \brief Base class for an external, optional module supported by the system.
  class ExternalModule
  {
    protected:
      HMODULE mModule; //!< The module handle
      bool mInitialized; //!< true if initialized
    public:
      //! \brief Values that represent InitReturn.
      enum InitReturn: unsigned int {
        Initialization_OK = 0,  //!< An enum constant representing the initialization ok option
        Initialization_ModuleNotFound,
        Initialization_MissingExports,
        Initialization_Unavailable
      };

      //! \brief Default constructor.
      ExternalModule();

      //! \brief Initializes this ExternalModule.
      //! \return An InitReturn.
      virtual InitReturn initialize() = 0;

      //! \brief Shuts down this ExternalModule and frees any resources it is using.
      virtual void shutdown() = 0;

      //! \brief Query if this ExternalModule is initialized.
      //! \return true if initialized, false if not.
      virtual bool isInitialized() const;
  };

  //! \addtogroup Logitech
  //! @{

  namespace Logitech {

    // Logitech G-Key SDK (1.02.004) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechGkeySDK_1.02.004/ for full SDK and libraries.

#pragma pack( push, 1 )

    //! \brief Logitech G-Key code
    typedef struct
    {
      unsigned int keyIdx         : 8;  //!< index of the G key or mouse button, for example, 6 for G6 or Button 6
      unsigned int keyDown        : 1;  //!< key up or down, 1 is down, 0 is up
      unsigned int mState         : 2;  //!< mState (1, 2 or 3 for M1, M2 and M3)
      unsigned int mouse          : 1;  //!< indicate if the Event comes from a mouse, 1 is yes, 0 is no.
      unsigned int reserved1      : 4;  //!< reserved1
      unsigned int reserved2      : 16; //!< reserved2
    } GkeyCode;

    //! \brief Logitech G-Key callback function signature.
    typedef void (__cdecl *logiGkeyCB)( GkeyCode gkeyCode, const wchar_t* gkeyOrButtonString, void* context );

    //! \brief Logitech G-Key SDK callback context.
    typedef struct
    {
      logiGkeyCB gkeyCallBack;  //!< G-Key callback function
      void* gkeyContext;  //!< Callback context
    } logiGkeyCBContext;

#pragma pack( pop )

    //! \brief G-key type.
    typedef unsigned int GKey;

    //! \class GKeyListener
    //! \brief A G-Key listener base class.
    //!  Derive your own G-Key input listener from this class.
    class GKeyListener
    {
      public:
        //! \brief G-Key press event.
        //! \param  key Keycode.
        virtual void onGKeyPressed( GKey key ) = 0;

        //! \brief G-Key release event.
        //! \param  key Keycode.
        virtual void onGKeyReleased( GKey key ) = 0;
    };

    //! \brief A list of G-Key input listeners.
    typedef list<GKeyListener*> GKeyListenerList;

    //! \brief Import function signature for LogiGkeyInit
    typedef BOOL (*fnLogiGkeyInit)( logiGkeyCBContext* context );
    //! \brief Import function signature for LogiGkeyGetMouseButtonString
    typedef wchar_t* (*fnLogiGkeyGetMouseButtonString)( int button );
    //! \brief Import function signature for LogiGkeyGetKeyboardGkeyString
    typedef wchar_t* (*fnLogiGkeyGetKeyboardGkeyString)( int key, int mode );
    //! \brief Import function signature for LogiGkeyShutdown
    typedef void (*fnLogiGkeyShutdown)();

    //! \brief A queue of pressed G-keys.
    typedef queue<GkeyCode> GKeyQueue;

    //! \class GKeySDK
    //! \brief Logitech G-Key SDK module.
    //! \sa ExternalModule
    class GKeySDK: public ExternalModule
    {
      protected:
        //! \brief G-Key SDK's import functions.
        struct Functions {
          fnLogiGkeyInit pfnLogiGkeyInit; //!< fnLogiGkeyInit
          fnLogiGkeyGetMouseButtonString pfnLogiGkeyGetMouseButtonString; //!< fnLogiGkeyGetMouseButtonString
          fnLogiGkeyGetKeyboardGkeyString pfnLogiGkeyGetKeyboardGkeyString; //!< fnLogiGkeyGetKeyboardGkeyString
          fnLogiGkeyShutdown pfnLogiGkeyShutdown; //!< fnLogiGkeyShutdown
          Functions();
        } mFunctions; //!< My import functions
        logiGkeyCBContext mContext; //!< The context
        SRWLOCK mLock;  //!< The lock
        GKeyQueue mQueue; //!< The queue
        GKeyListenerList mListeners;  //!< The listeners

        //! \brief Callback, called when the key.
        //! \param  key             The key.
        //! \param  name            The name.
        //! \param [in,out] context If non-null, the context.
        static void __cdecl keyCallback( GkeyCode key, const wchar_t* name, void* context );
      public:
        //! \brief Default constructor.
        GKeySDK();

        //! \brief Tries to initialize the G-Key SDK module.
        //! \return An InitReturn code.
        virtual InitReturn initialize();

        //! \brief Adds a listener.
        //! \param [in,out] listener If non-null, the listener.
        void addListener( GKeyListener* listener );

        //! \brief Removes the listener described by listener.
        //! \param [in,out] listener If non-null, the listener.
        void removeListener( GKeyListener* listener );

        //! \brief Updates this GKeySDK.
        void update();

        //! \brief Shuts down the G-Key SDK module and frees any resources.
        virtual void shutdown();

        //! \brief Destructor.
        ~GKeySDK();
    };

    // Logitech LED SDK (1.01.005.1) implementation
    // Portions of following code copyright (c) Logitech.
    // See external/LogitechLEDSDK_1.01.005.1/ for full SDK and libraries.

    const int LOGITECH_LED_MOUSE = 0x0001; //!< Only affect mouse LEDs.
    const int LOGITECH_LED_KEYBOARD = 0x0002; //!< Only affect keyboard LEDs
    const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD; //!< Affect both mouse and keyboard LEDs

    //! \brief Import function signature for LogiLedInit
    typedef BOOL (*fnLogiLedInit)();
    //! \brief Import function signature for LogiLedSaveCurrentLighting
    typedef BOOL (*fnLogiLedSaveCurrentLighting)( int deviceType );
    //! \brief Import function signature for LogiLedSetLighting
    typedef BOOL (*fnLogiLedSetLighting)( int deviceType, int redPercentage, int greenPercentage, int bluePercentage );
    //! \brief Import function signature for LogiLedRestoreLighting
    typedef BOOL (*fnLogiLedRestoreLighting)( int deviceType );
    //! \brief Import function signature for LogiLedShutdown
    typedef void (*fnLogiLedShutdown)();

    //! \class LedSDK
    //! \brief Logitech Led SDK module.
    //! \sa ExternalModule
    class LedSDK: public ExternalModule
    {
      protected:
        //! \brief Led SDK's import functions.
        struct Functions {
          fnLogiLedInit pfnLogiLedInit; //!< fnLogiLedInit
          fnLogiLedSaveCurrentLighting pfnLogiLedSaveCurrentLighting; //!< fnLogiLedSaveCurrentLighting
          fnLogiLedSetLighting pfnLogiLedSetLighting; //!< fnLogiLedSetLighting
          fnLogiLedRestoreLighting pfnLogiLedRestoreLighting; //!< fnLogiLedRestoreLighting
          fnLogiLedShutdown pfnLogiLedShutdown; //!< fnLogiLedShutdown
          Functions();
        } mFunctions; //!< My import functions
        bool mSavedOriginal;  //!< true to saved original
      public:
        //! \brief Default constructor.
        LedSDK();

        //! \brief Tries to initialize the Led SDK module.
        //! \return An InitReturn code.
        virtual InitReturn initialize();

        //! \brief Sets a lighting.
        //! \param  color The color.
        void setLighting( const Color& color );

        //! \brief Shuts down the Led SDK module and frees any resources.
        virtual void shutdown();

        //! \brief Destructor.
        ~LedSDK();
    };

  }

  //! @}

  //! @}

}