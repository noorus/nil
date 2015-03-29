#pragma once

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilPnP.h"
#include "nilHID.h"
#include "nilCommon.h"
#include "nilWindows.h"
#include "nilLogitech.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! \class SystemListener
  //! System event listener base class.
  //! Derive your own listener from this class.
  class SystemListener
  {
    public:
      //! Called when a device is connected to the system.
      virtual void onDeviceConnected( Device* device ) = 0;

      //! Called when a device is disconnected from the system.
      virtual void onDeviceDisconnected( Device* device ) = 0;

      //! Called when a connected mouse device is enabled (instanced).
      virtual void onMouseEnabled( Device* device, Mouse* instance ) = 0;

      //! Called when a connected keyboard device is enabled (instanced).
      virtual void onKeyboardEnabled( Device* device, Keyboard* instance ) = 0;

      //! Called when a connected controller device is enabled (instanced).
      virtual void onControllerEnabled( Device* device, Controller* instance ) = 0;

      //! Called when a mouse device is disabled.
      //! The instance is destroyed immediately after.
      virtual void onMouseDisabled( Device* device, Mouse* instance ) = 0;

      //! Called when a keyboard device is disabled.
      //! The instance is destroyed immediately after.
      virtual void onKeyboardDisabled( Device* device, Keyboard* instance ) = 0;

      //! Called when a controller device is disabled.
      //! The instance is destroyed immediately after.
      virtual void onControllerDisabled( Device* device, Controller* instance ) = 0;
  };

  //! \class System
  //! The Nil input system root.
  //! Create one to get started.
  //! \sa PnPListener
  //! \sa RawListener
  class System: public PnPListener, public RawListener
  {
    friend class Device;
    friend class DirectInputController;
    friend class RawInputKeyboard;
    friend class RawInputMouse;
    private:
      DeviceID mIDPool; //!< Device indexing pool
      int mMouseIndexPool;  //!< Mouse indexing pool
      int mKeyboardIndexPool; //!< Keyboard indexing pool
      int mControllerIndexPool; //!< Controller indexing pool
      vector<DeviceID> mXInputIDs;  //!< XInput device ID mapping
      vector<uint32_t> mXInputDeviceIDs;  //!< Tracked list of XInput VIDs & PIDs
      IDirectInput8W* mDirectInput; //!< Our DirectInput instance
      HINSTANCE mInstance;  //!< Host application instance handle
      HWND mWindow; //!< Host application window handle
      EventMonitor* mMonitor; //!< Our Plug-n-Play & raw input event monitor
      DeviceList mDevices;  //!< List of known devices
      HIDManager* mHIDManager;  //!< Our HID manager
      bool mInitializing; //!< Are we initializing?
      RawMouseMap mMouseMapping;  //!< Raw mouse events mapping
      RawKeyboardMap mKeyboardMapping;  //!< Raw keyboard events mapping
      Logitech::GKeySDK* mLogitechGKeys;  //!< External module for Logitech G-Keys
      Logitech::LedSDK* mLogitechLEDs;  //!< External module for Logitech LEDs
      SystemListener* mListener;  //!< Our single event listener
      XInput* mXInput; //!< XInput module handler
      const Cooperation mCooperation; //!< Cooperation mode
      struct Internals {
        bool swapMouseButtons;
        STICKYKEYS storedStickyKeys;
        TOGGLEKEYS storedToggleKeys;
        FILTERKEYS storedFilterKeys;
        void store();
        void disableHotKeyHarassment();
        void restore();
      } mInternals;
      void initializeDevices();
      void refreshDevices();
      void identifyXInputDevices();
      DeviceID getNextID();
      int getNextMouseIndex();
      int getNextKeyboardIndex();
      int getNextControllerIndex();
      void deviceConnect( Device* device );
      void deviceDisconnect( Device* device );
      void mouseEnabled( Device* device, Mouse* instance );
      void keyboardEnabled( Device* device, Keyboard* instance );
      void controllerEnabled( Device* device, Controller* instance );
      void mouseDisabled( Device* device, Mouse* instance );
      void keyboardDisabled( Device* device, Keyboard* instance );
      void controllerDisabled( Device* device, Controller* instance );
      void mapMouse( HANDLE handle, RawInputMouse* mouse );
      void unmapMouse( HANDLE handle );
      void mapKeyboard( HANDLE handle, RawInputKeyboard* keyboard );
      void unmapKeyboard( HANDLE handle );
      //! \b Internal My PnP plug callback.
      virtual void onPnPPlug( const GUID& deviceClass, const wideString& devicePath );
      //! \b Internal My PnP unplug callback.
      virtual void onPnPUnplug( const GUID& deviceClass, const wideString& devicePath );
      //! \b Internal My Raw arrival callback.
      virtual void onRawArrival( HANDLE handle );
      //! \b Internal My Raw mouse input callback.
      virtual void onRawMouseInput( HANDLE handle, const RAWMOUSE& input, const bool sinked );
      //! \b Internal My Raw keyboard input callback.
      virtual void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input, const bool sinked );
      //! \b Internal My Raw removal callback.
      virtual void onRawRemoval( HANDLE handle );
      //! \b Internal My DirectInput device enumeration callback.
      static BOOL CALLBACK diDeviceEnumCallback(
        LPCDIDEVICEINSTANCEW instance, LPVOID referer );
    public:
      //! Constructor.
      //! \param  instance  Handle of the host instance.
      //! \param  window    Handle of the host window.
      //! \param  coop      Cooperation mode.
      //! \param  listener  Listener for system events.
      System( HINSTANCE instance, HWND window, const Cooperation coop,
        SystemListener* listener );

      //! Updates this System.
      //! All listened events get triggered from inside this call.
      void update();

      //! Get currently known devices.
      //! \return The devices.
      DeviceList& getDevices();

      //! Get Logitech G-Key SDK, if available.
      //! \return null if it fails, else the Logitech G-Keys SDK object.
      Logitech::GKeySDK* getLogitechGKeys();

      //! Get Logitech LED SDK, if available.
      //! \return null if it fails, else the Logitech LED SDK object.
      Logitech::LedSDK* getLogitechLEDs();

      XInput* getXInput();

      //! Query if this System is initializing.
      //! \return true if initializing, false if not.
      const bool isInitializing() const;

      //! Destructor.
      ~System();
  };

  //! @}

}