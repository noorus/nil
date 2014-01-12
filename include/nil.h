#pragma once

#ifndef NTDDI_VERSION
# define NTDDI_VERSION NTDDI_VISTA
# define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbt.h>
#include <objbase.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>
#include <setupapi.h>

#include <stdlib.h>
#include <stdint.h>

#include <exception>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

namespace nil {

  typedef std::string string;
  typedef std::wstring wstring;

  using std::list;
  using std::vector;
  using std::wstringstream;
  using boost::variant;

  //! \class WinAPIError
  //! Container for a Windows API error description.
  struct WinAPIError {
  public:
    uint32_t code;
    wstring description;
  };

  //! \class Exception
  //! Main exception class. Descendant of std::exception.
  class Exception: public std::exception {
  public:
    enum Type {
      Generic = 0,
      WinAPI,
      DirectInput
    };
  private:
    Exception();
  protected:
    Type mType;
    wstring mDescription;
    wstring mSource;
    mutable wstring mFullDescription;
    mutable string mUTF8Description;
    variant<WinAPIError> mAdditional;
    void handleAdditional( HRESULT hr = 0 );
  public:
    Exception( const wstring& description, Type type = Generic );
    Exception( const wstring& description, const wstring& source,
      Type type = Generic );
    Exception( const wstring& description, const wstring& source,
      HRESULT hr, Type type = Generic );
    virtual const wstring& getFullDescription() const;
    virtual const char* what() const throw();
  };

  //! \class Device
  //! Input device instance.
  class Device {
  friend class System;
  public:
    enum Type {
      Device_DirectInput,
      Device_XInput
    };
    enum State {
      State_Disconnected, //!< Disconnected but not forgotten
      State_Pending, //!< Pending refresh
      State_Current //!< Up-to-date and available
    };
  protected:
    Type mType;
    State mState;
    GUID mProductID;
    GUID mDeviceID;
    struct XInput {
      int deviceID;
    } mXInput;
    void setState( State state );
    void makeXInput( int index );
    Device( Type type, GUID productID, GUID deviceID );
    ~Device();
  public:
    const Type getType();
    const State getState();
    const GUID getProductID();
    const GUID getDeviceID();
  };

  typedef list<Device*> DeviceList;

  //! \class PnPListener
  //! Plug-n-Play event listener.
  class PnPListener {
  friend class PnPMonitor;
  protected:
    virtual void onPlug( const GUID& deviceClass, const wstring& devicePath ) = 0;
    virtual void onUnplug( const GUID& deviceClass, const wstring& devicePath ) = 0;
  };

  //! \class PnPMonitor
  //! Monitors for Plug-n-Play (USB) device events.
  class PnPMonitor {
  protected:
    HINSTANCE mInstance;
    ATOM mClass;
    HWND mWindow;
    HDEVNOTIFY mNotifications;
    PnPListener* mListener;
  protected:
    void registerNotifications();
    void unregisterNotifications();
    static LRESULT CALLBACK wndProc( HWND window, UINT message,
      WPARAM wParam, LPARAM lParam );
  public:
    PnPMonitor( HINSTANCE instance, PnPListener* listener );
    void update();
    ~PnPMonitor();
  };

  //! \class System
  //! The input system.
  class System: public PnPListener {
  protected:
    IDirectInput8* mDirectInput; //!< Our DirectInput instance
    HINSTANCE mInstance; //!< Host application instance handle
    HWND mWindow; //!< Host application window handle
    PnPMonitor* mMonitor; //!< Our Plug-n-Play event monitor
    DeviceList mDevices; //!< List of current devices
    bool mInitializedCOM; //!< Are we responsible for freeing COM?
    void refreshDevices();
    void identifyXInputDevices();
    bool resolveDevice( const wstring& devicePath );
    virtual void onPlug( const GUID& deviceClass, const wstring& devicePath );
    virtual void onUnplug( const GUID& deviceClass, const wstring& devicePath );
    static BOOL CALLBACK diEnumCallback(
      LPCDIDEVICEINSTANCE instance, LPVOID referer );
  public:
    System( HINSTANCE instance, HWND window );
    void update();
    ~System();
  };

}