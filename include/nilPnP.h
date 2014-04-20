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
  //! Plug-n-Play event listener.
  class PnPListener
  {
    public:
      //! \brief Plug-n-Play device plug event.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPPlug( const GUID& deviceClass,
        const String& devicePath ) = 0;

      //! \brief Plug-n-Play device unplug event.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPUnplug( const GUID& deviceClass,
        const String& devicePath ) = 0;
  };

  //! \brief A list of Plug-and-Play event listeners.
  typedef list<PnPListener*> PnPListenerList;

  //! \class RawListener
  //! Raw input event listener.
  class RawListener
  {
    public:
      //! \brief Raw input device arrival event.
      //! \param  handle Raw input device handle.
      virtual void onRawArrival(
        HANDLE handle ) = 0;

      //! \brief Raw mouse input event.
      //! \param  handle Raw input device handle.
      //! \param  input  The input.
      virtual void onRawMouseInput(
        HANDLE handle, const RAWMOUSE& input ) = 0;

      //! \brief Raw keyboard input event.
      //! \param  handle Raw input device handle.
      //! \param  input  The input.
      virtual void onRawKeyboardInput(
        HANDLE handle, const RAWKEYBOARD& input ) = 0;

      //! \brief Raw input device removal event.
      //! \param  handle Raw input device handle.
      virtual void onRawRemoval(
        HANDLE handle ) = 0;
  };

  //! \brief A list of raw input device listeners.
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
      //! \brief Internal, register myself for event notifications.
      void registerNotifications();

      //! \brief Internal, unregister myself from event notifications.
      void unregisterNotifications();

      //! \brief Internal, handle interface arrival.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      void handleInterfaceArrival( const GUID& deviceClass,
        const String& devicePath );

      //! \brief Internal, handle interface removal.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      void handleInterfaceRemoval( const GUID& deviceClass,
        const String& devicePath );

      //! \brief Internal, handle raw device arrival.
      //! \param  handle Raw device handle.
      void handleRawArrival( HANDLE handle );

      //! \brief Internal, handle raw input.
      //! \param  input The input.
      void handleRawInput( HRAWINPUT input );

      //! \brief Internal, handle raw device removal.
      //! \param  handle Raw device handle.
      void handleRawRemoval( HANDLE handle );

      //! \brief Internal hidden window procedure for receiving messages.
      //! \param  window  Handle of the window.
      //! \param  message The message.
      //! \param  wParam  The wParam field of the message.
      //! \param  lParam  The lParam field of the message.
      //! \return LRESULT
      static LRESULT CALLBACK wndProc( HWND window, UINT message,
        WPARAM wParam, LPARAM lParam );
    public:
      //! \brief Constructor.
      //! \param  instance The instance.
      EventMonitor( HINSTANCE instance );

      //! \brief Register a listener for Plug-n-Play events.
      //! \param listener The listener.
      void registerPnPListener( PnPListener* listener );

      //! \brief Unregister a listener from Plug-n-Play events.
      //! \param listener The listener.
      void unregisterPnPListener( PnPListener* listener );

      //! \brief Register a listener for raw input events.
      //! \param listener The listener.
      void registerRawListener( RawListener* listener );

      //! \brief Unregister a listener from raw input events.
      //! \param listener The listener.
      void unregisterRawListener( RawListener* listener );

      //! \brief Update the EventMonitor, triggering new events.
      void update();

      //! \brief Destructor.
      ~EventMonitor();
  };

  //! @}

}