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

# define NIL_MAX_XINPUT_DEVICES 4

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

  typedef int DeviceID;

  //! \class Device
  //! Input device information entry.
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
      State_Connected //!< Up-to-date and available
    };
  protected:
    DeviceID mID;
    State mState;
    Device( DeviceID id );
    void onUnplug();
    void onPlug();
    void setState( State state );
  public:
    virtual const DeviceID getID();
    virtual const Type getType() = 0;
    virtual const State getState();
  };

  class DirectInputDevice: public Device {
  friend class System;
  protected:
    GUID mProductID;
    GUID mInstanceID;
    DirectInputDevice( DeviceID id, const GUID& productID, const GUID& instanceID );
  public:
    virtual const Type getType();
    virtual const GUID getProductID();
    virtual const GUID getInstanceID();
  };

  class XInputDevice: public Device {
  friend class System;
  protected:
    int mXInputID;
    XInputDevice( DeviceID id, int xinputID );
  public:
    virtual const Type getType();
    virtual const int getXInputID();
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
    HINSTANCE mInstance; //!< Host application instance handle
    ATOM mClass; //!< Class registration handle
    HWND mWindow; //!< Window handle
    HDEVNOTIFY mNotifications; //!< Device notifications registration
    PnPListener* mListener; //!< Our listener
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
    DeviceID mIDPool;
    vector<DeviceID> mXInputIDs;
    vector<unsigned long> mXInputDeviceIDs;
    IDirectInput8W* mDirectInput; //!< Our DirectInput instance
    HINSTANCE mInstance; //!< Host application instance handle
    HWND mWindow; //!< Host application window handle
    PnPMonitor* mMonitor; //!< Our Plug-n-Play event monitor
    DeviceList mDevices; //!< List of known devices
    void initializeDevices();
    void refreshDevices();
    void identifyXInputDevices();
    DeviceID getNextID();
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