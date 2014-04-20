#pragma once
#include "nilTypes.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! USB Usage Page IDs.
  //! See See USB HID Usage Tables version 1.1, page 15
  enum USBUsagePage
  {
    USBUsagePage_Desktop = 0x01
  };

  //! USB Generic Desktop Page usage IDs.
  //! See USB HID Usage Tables version 1.1, page 27
  enum USBDesktopUsage
  {
    USBDesktopUsage_Mice = 0x02,
    USBDesktopUsage_Keyboards = 0x06
  };

  //! \class PnPListener
  //! Plug-n-Play event listener base class.
  class PnPListener
  {
    public:
      //! Plug-n-Play device plug event.
      virtual void onPnPPlug( const GUID& deviceClass,
        const String& devicePath ) = 0;

      //! Plug-n-Play device unplug event.
      virtual void onPnPUnplug( const GUID& deviceClass,
        const String& devicePath ) = 0;
  };

  //! A list of Plug-and-Play event listeners.
  typedef list<PnPListener*> PnPListenerList;

  //! \class RawListener
  //! Raw input event listener base class.
  class RawListener
  {
    public:
      //! Raw input device arrival event.
      virtual void onRawArrival(
        HANDLE handle ) = 0;

      //! Raw mouse input event.
      virtual void onRawMouseInput(
        HANDLE handle, const RAWMOUSE& input ) = 0;

      //! Raw keyboard input event.
      virtual void onRawKeyboardInput(
        HANDLE handle, const RAWKEYBOARD& input ) = 0;

      //! Raw input device removal event.
      virtual void onRawRemoval(
        HANDLE handle ) = 0;
  };

  //! A list of raw input device listeners.
  typedef list<RawListener*> RawListenerList;

  //! \class EventMonitor
  //! Monitors for Plug-n-Play (USB) and Raw device events.
  class EventMonitor
  {
    protected:
      HINSTANCE mInstance;  //!< Host application instance handle
      ATOM mClass;  //!< Class registration handle
      HWND mWindow; //!< Window handle
      HDEVNOTIFY mNotifications;  //!< Device notifications registration
      PnPListenerList mPnPListeners;  //!< Our Plug-n-Play listeners
      RawListenerList mRawListeners;  //!< Our raw listeners
      void* mInputBuffer; //!< Buffer for input reads
      unsigned int mInputBufferSize;  //!< Size of input buffer
    protected:
      //! \b Internal Register myself for event notifications.
      void registerNotifications();

      //! \b Internal Unregister myself from event notifications.
      void unregisterNotifications();

      //! \b Internal Handle interface arrival.
      void handleInterfaceArrival( const GUID& deviceClass,
        const String& devicePath );

      //! \b Internal Handle interface removal.
      void handleInterfaceRemoval( const GUID& deviceClass,
        const String& devicePath );

      //! \b Internal Handle raw device arrival.
      void handleRawArrival( HANDLE handle );

      //! \b Internal Handle raw input.
      void handleRawInput( HRAWINPUT input );

      //! \b Internal Handle raw device removal.
      void handleRawRemoval( HANDLE handle );

      //! \b Internal Hidden window procedure for receiving messages.
      static LRESULT CALLBACK wndProc( HWND window, UINT message,
        WPARAM wParam, LPARAM lParam );
    public:
      EventMonitor( HINSTANCE instance );

      //! Register a listener for Plug-n-Play events.
      void registerPnPListener( PnPListener* listener );

      //! Unregister a listener from Plug-n-Play events.
      void unregisterPnPListener( PnPListener* listener );

      //! Register a listener for raw input events.
      void registerRawListener( RawListener* listener );

      //! Unregister a listener from raw input events.
      void unregisterRawListener( RawListener* listener );

      //! Update the EventMonitor, triggering new events.
      void update();

      ~EventMonitor();
  };

  //! @}

}