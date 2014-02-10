#pragma once
#include "nilTypes.h"

namespace nil {

  enum USBUsagePage {
    USBUsagePage_Desktop = 0x01
  };

  enum USBUsage {
    USBUsage_Mice = 0x02,
    USBUsage_Keyboards = 0x06
  };

  //! \class PnPListener
  //! Plug-n-Play event listener.
  class PnPListener {
  public:
    virtual void onPnPPlug( const GUID& deviceClass,
      const String& devicePath ) = 0;
    virtual void onPnPUnplug( const GUID& deviceClass,
      const String& devicePath ) = 0;
  };

  typedef list<PnPListener*> PnPListenerList;

  //! \class RawListener
  //! Raw input event listener.
  class RawListener {
  public:
    virtual void onRawArrival( HANDLE handle ) = 0;
    virtual void onRawMouseInput( HANDLE handle, const RAWMOUSE& input ) = 0;
    virtual void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input ) = 0;
    virtual void onRawRemoval( HANDLE handle ) = 0;
  };

  typedef list<RawListener*> RawListenerList;

  //! \class EventMonitor
  //! Monitors for Plug-n-Play (USB) and Raw device events.
  class EventMonitor {
  protected:
    HINSTANCE mInstance; //!< Host application instance handle
    ATOM mClass; //!< Class registration handle
    HWND mWindow; //!< Window handle
    HDEVNOTIFY mNotifications; //!< Device notifications registration
    PnPListenerList mPnPListeners; //!< Our Plug-n-Play listeners
    RawListenerList mRawListeners; //!< Our raw listeners
    void* mInputBuffer; //!< Buffer for input reads
    unsigned int mInputBufferSize; //!< Size of input buffer
  protected:
    void registerNotifications();
    void unregisterNotifications();
    void handleInterfaceArrival( const GUID& deviceClass,
      const String& devicePath );
    void handleInterfaceRemoval( const GUID& deviceClass,
      const String& devicePath );
    void handleRawArrival( HANDLE handle );
    void handleRawInput( HRAWINPUT input );
    void handleRawRemoval( HANDLE handle );
    static LRESULT CALLBACK wndProc( HWND window, UINT message,
      WPARAM wParam, LPARAM lParam );
  public:
    EventMonitor( HINSTANCE instance );
    void registerPnPListener( PnPListener* listener );
    void unregisterPnPListener( PnPListener* listener );
    void registerRawListener( RawListener* listener );
    void unregisterRawListener( RawListener* listener );
    void update();
    ~EventMonitor();
  };

}