#pragma once

#ifndef NTDDI_VERSION
# define NTDDI_VERSION NTDDI_VISTA
# define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbt.h>

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
  protected:
    GUID mProductID;
    GUID mDeviceID;
  public:
    Device( GUID productID, GUID deviceID );
    const GUID getProductID();
    const GUID getDeviceID();
    ~Device();
  };

  typedef list<Device*> DeviceList;

  class PnPListener {
  friend class PnPMonitor;
  protected:
    virtual void onPlug( const GUID& deviceClass, const wstring& devicePath ) = 0;
    virtual void onUnplug( const GUID& deviceClass, const wstring& devicePath ) = 0;
  };

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
    IDirectInput8* mDirectInput;
    HWND mWindow;
    HINSTANCE mInstance;
    PnPMonitor* mMonitor;
    DeviceList mDevices;
  protected:
    void enumerate();
    bool resolveDevice( const wstring& devicePath );
    virtual void onPlug( const GUID& deviceClass, const wstring& devicePath );
    virtual void onUnplug( const GUID& deviceClass, const wstring& devicePath );
    static BOOL CALLBACK diEnumCallback(
      LPCDIDEVICEINSTANCE instance, LPVOID referer );
  public:
    System( HWND window );
    void update();
    ~System();
  };

}