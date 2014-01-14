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

  typedef std::string utf8String;
  typedef std::wstring String;
  typedef float Real;

  using std::list;
  using std::vector;
  using std::wstringstream;
  using boost::variant;

  //! \class WinAPIError
  //! Container for a Windows API error description.
  struct WinAPIError {
  public:
    uint32_t code;
    String description;
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
    String mDescription;
    String mSource;
    mutable String mFullDescription;
    mutable utf8String mUTF8Description;
    variant<WinAPIError> mAdditional;
    void handleAdditional( HRESULT hr = 0 );
  public:
    Exception( const String& description, Type type = Generic );
    Exception( const String& description, const String& source,
      Type type = Generic );
    Exception( const String& description, const String& source,
      HRESULT hr, Type type = Generic );
    virtual const String& getFullDescription() const;
    virtual const char* what() const throw();
  };

  typedef int DeviceID;

  class System;
  class DeviceInstance;

  //! \class Device
  //! Input device information entry.
  class Device {
  friend class System;
  public:
    enum Handler {
      Handler_DirectInput,
      Handler_XInput
    };
    enum Type {
      Device_Keyboard,
      Device_Mouse,
      Device_Controller
    };
    enum Status {
      Status_Disconnected, //!< Disconnected but not forgotten
      Status_Pending, //!< Pending refresh
      Status_Connected //!< Up-to-date and available
    };
  protected:
    DeviceID mID;
    Status mStatus;
    Status mSavedStatus;
    String mName;
    Type mType;
    System* mSystem;
    DeviceInstance* mInstance;
    Device( System* system, DeviceID id, Type type );
    virtual void create();
    virtual void destroy();
    virtual void setStatus( Status status );
    virtual void saveStatus();
    virtual const Status getSavedStatus();
    virtual void onDisconnect();
    virtual void onConnect();
  public:
    virtual const DeviceID getID();
    virtual const Handler getHandler() = 0;
    virtual const Type getType();
    virtual const Status getStatus();
    virtual const String& getName();
    virtual System* getSystem();
  };

  class DirectInputDevice: public Device {
  friend class System;
  protected:
    GUID mProductID;
    GUID mInstanceID;
    DirectInputDevice( System* system, DeviceID id,
      LPCDIDEVICEINSTANCEW instance );
  public:
    virtual const Handler getHandler();
    virtual const GUID getProductID();
    virtual const GUID getInstanceID();
  };

  class XInputDevice: public Device {
  friend class System;
  protected:
    int mXInputID;
    bool mIdentified;
    XINPUT_CAPABILITIES mCapabilities;
    XInputDevice( System* system, DeviceID id, int xinputID );
    virtual void identify();
    virtual void setStatus( Status status );
    virtual void onDisconnect();
    virtual void onConnect();
  public:
    virtual const Handler getHandler();
    virtual const int getXInputID();
  };

  typedef list<Device*> DeviceList;

  class DeviceInstance {
  protected:
    System* mSystem;
    Device* mDevice;
  public:
    DeviceInstance( System* system, Device* device );
    virtual ~DeviceInstance();
  };

  class Mouse: public DeviceInstance {
  protected:
  public:
    Mouse( System* system, Device* device );
    virtual ~Mouse();
  };

  class Keyboard: public DeviceInstance {
  protected:
  public:
    Keyboard( System* system, Device* device );
    virtual ~Keyboard();
  };

  class Controller: public DeviceInstance {
  public:
    enum Type {
      Controller_Joystick,
      Controller_Gamepad,
      Controller_Firstperson,
      Controller_Driving,
      Controller_Flight,
      Controller_DancePad,
      Controller_Guitar,
      Controller_Drumkit,
      Controller_ArcadePad
    };
  protected:
    Type mType;
  public:
    Controller( System* system, Device* device );
    virtual ~Controller();
    virtual const Type getType();
  };

  class DirectInputMouse: public Mouse {
  protected:
  public:
    DirectInputMouse( DirectInputDevice* device );
    virtual ~DirectInputMouse();
  };

  class DirectInputKeyboard: public Keyboard {
  protected:
  public:
    DirectInputKeyboard( DirectInputDevice* device );
    virtual ~DirectInputKeyboard();
  };

  class DirectInputController: public Controller {
  protected:
  public:
    DirectInputController( DirectInputDevice* device );
    virtual ~DirectInputController();
  };

  class XInputController: public Controller {
  protected:
  public:
    XInputController( XInputDevice* device );
    virtual ~XInputController();
  };

  //! \class PnPListener
  //! Plug-n-Play event listener.
  class PnPListener {
  friend class PnPMonitor;
  protected:
    virtual void onPlug( const GUID& deviceClass,
      const String& devicePath ) = 0;
    virtual void onUnplug( const GUID& deviceClass,
      const String& devicePath ) = 0;
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
    DeviceID mIDPool; //!< Device indexing pool
    vector<DeviceID> mXInputIDs; //!< XInput device ID mapping
    vector<uint32_t> mXInputDeviceIDs; //!< Tracked list of XInput VIDs & PIDs
    IDirectInput8W* mDirectInput; //!< Our DirectInput instance
    HINSTANCE mInstance; //!< Host application instance handle
    HWND mWindow; //!< Host application window handle
    PnPMonitor* mMonitor; //!< Our Plug-n-Play event monitor
    DeviceList mDevices; //!< List of known devices
    bool mInitializing; //!< Are we initializing
    void initializeDevices();
    void refreshDevices();
    void identifyXInputDevices();
    DeviceID getNextID();
    virtual void onPlug( const GUID& deviceClass, const String& devicePath );
    virtual void onUnplug( const GUID& deviceClass, const String& devicePath );
    static BOOL CALLBACK diEnumCallback(
      LPCDIDEVICEINSTANCEW instance, LPVOID referer );
  public:
    System( HINSTANCE instance, HWND window );
    void update();
    const bool isInitializing();
    ~System();
  };

}