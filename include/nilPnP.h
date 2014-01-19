#pragma once
#include "nilTypes.h"

namespace nil {

  //! \class PnPListener
  //! Plug-n-Play event listener.
  class PnPListener {
  public:
    virtual void onPlug( const GUID& deviceClass,
      const String& devicePath ) = 0;
    virtual void onUnplug( const GUID& deviceClass,
      const String& devicePath ) = 0;
  };

  typedef list<PnPListener*> PnPListenerList;

  //! \class PnPMonitor
  //! Monitors for Plug-n-Play (USB) device events.
  class PnPMonitor {
  protected:
    HINSTANCE mInstance; //!< Host application instance handle
    ATOM mClass; //!< Class registration handle
    HWND mWindow; //!< Window handle
    HDEVNOTIFY mNotifications; //!< Device notifications registration
    PnPListenerList mListeners; //!< Our listeners
  protected:
    void registerNotifications();
    void unregisterNotifications();
    void handleArrival( const GUID& deviceClass,
      const String& devicePath );
    void handleRemoval( const GUID& deviceClass,
      const String& devicePath );
    static LRESULT CALLBACK wndProc( HWND window, UINT message,
      WPARAM wParam, LPARAM lParam );
  public:
    PnPMonitor( HINSTANCE instance, PnPListener* listener );
    void registerListener( PnPListener* listener );
    void unregisterListener( PnPListener* listener );
    void update();
    ~PnPMonitor();
  };

}