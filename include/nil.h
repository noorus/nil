#pragma once
#include "nilConfig.h"

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilCommon.h"

#ifdef NIL_PLATFORM_WINDOWS
# include "nilWindows.h"
# include "nilWindowsPNP.h"
#endif

namespace nil {

  //! \addtogroup Nil
  //! @{

  //! \class SystemListener
  //! System event listener base class.
  //! Derive your own listener from this class.
  class SystemListener {
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

#ifdef NIL_PLATFORM_WINDOWS

  //! \class System
  //! The Nil input system root.
  //! Create one to get started.
  //! \sa PnPListener
  //! \sa RawListener
  class System: public windows::PnPListener, public windows::RawListener, public std::enable_shared_from_this<System> {
  friend class Device;
  friend class DirectInputController;
  friend class RawInputKeyboard;
  friend class RawInputMouse;
  friend class RawInputController;
  private:
    DeviceID idPool_ = 0; //!< Device indexing pool
    int mouseIdPool_ = 0; //!< Mouse indexing pool
    int keyboardIdPool_ = 0; //!< Keyboard indexing pool
    int controllerIdPool_ = 0; //!< Controller indexing pool
    vector<DeviceID> xinputIds_; //!< XInput device ID mapping
    set<uint32_t> specialHandlingDeviceIDs_; //!< Device VID/PID identifiers that will be ignored by DirectInput
    IDirectInput8W* dinput_ = nullptr; //!< Our DirectInput instance
    HINSTANCE instance_; //!< Host application instance handle
    HWND window_; //!< Host application window handle
    unique_ptr<windows::EventMonitor> eventMonitor_; //!< Our Plug-n-Play & raw input event monitor
    DeviceList devices_; //!< List of known devices
    unique_ptr<windows::HIDManager> hidManager_; //!< Our HID manager
    bool initializing_ = true; //!< Are we initializing?
    RawMouseMap mouseMap_; //!< Raw mouse events mapping
    RawKeyboardMap keyboardMap_; //!< Raw keyboard events mapping
    RawControllerMap controllerMap_; //!< Raw controller events mapping
    SystemListener* listener_; //!< Our single event listener
    unique_ptr<XInput> xinput_; //!< XInput module handler
    const Cooperation coop_; //!< Cooperation mode
    struct Internals {
      bool swapMouseButtons;
      STICKYKEYS storedStickyKeys;
      TOGGLEKEYS storedToggleKeys;
      FILTERKEYS storedFilterKeys;
      void store();
      void disableHotkeyHelpers();
      void restore();
    } internals_;
    void initializeDevices();
    void refreshDevices();
    void identifySpecialHandlingDevices();
    DeviceID getNextID();
    int getNextMouseIndex();
    int getNextKeyboardIndex();
    int getNextControllerIndex();
    bool getDefaultMouseButtonSwapping();
    void deviceConnect( DevicePtr device );
    void deviceDisconnect( DevicePtr device );
    void mouseEnabled( DevicePtr device, MousePtr instance );
    void keyboardEnabled( DevicePtr device, KeyboardPtr instance );
    void controllerEnabled( DevicePtr device, ControllerPtr instance );
    void mouseDisabled( DevicePtr device, MousePtr instance );
    void keyboardDisabled( DevicePtr device, KeyboardPtr instance );
    void controllerDisabled( DevicePtr device, ControllerPtr instance );
    void mapMouse( HANDLE handle, RawInputMouse* mouse );
    void unmapMouse( HANDLE handle );
    void mapKeyboard( HANDLE handle, RawInputKeyboard* keyboard );
    void unmapKeyboard( HANDLE handle );
    void mapController( HANDLE handle, RawInputController* controller );
    void unmapController( HANDLE handle );
    //! \b Internal My PnP plug callback.
    void onPnPPlug( const GUID& deviceClass, const wideString& devicePath ) override;
    //! \b Internal My PnP unplug callback.
    void onPnPUnplug( const GUID& deviceClass, const wideString& devicePath ) override;
    //! \b Internal My Raw arrival callback.
    void onRawArrival( HANDLE handle ) override;
    //! \b Internal My Raw mouse input callback.
    void onRawMouseInput( HANDLE handle, const RAWMOUSE& input, const bool sinked ) override;
    //! \b Internal My Raw keyboard input callback.
    void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input, const bool sinked ) override;
    //! \b Internal My Raw HID input callback.
    void onRawHIDInput( HANDLE handle, const RAWHID& input, const bool sinked ) override;
    //! \b Internal My Raw removal callback.
    void onRawRemoval( HANDLE handle ) override;
    //! \b Internal My DirectInput device enumeration callback.
    static BOOL CALLBACK diDeviceEnumCallback(
      LPCDIDEVICEINSTANCEW instance, LPVOID referer );

  private:
    //! Private constructor.
    System( HINSTANCE instance, HWND window, const Cooperation coop, SystemListener* listener );
    System() = default;

  public:
    //! Factory function. Use this to construct your instance.
    //! \param  instance  Handle of the host instance.
    //! \param  window    Handle of the host window.
    //! \param  coop      Cooperation mode.
    //! \param  listener  Listener for system events.
    [[nodiscard]] static SystemPtr create( HINSTANCE instance, HWND window, const Cooperation coop, SystemListener* listener );

    //! Initialize this system.
    //! Call only once after constructing, before the first update().
    void initialize();

    //! Updates this System.
    //! All listened events get triggered from inside this call.
    void update();

    //! Get currently known devices.
    //! \return The devices.
    DeviceList& getDevices();

    XInput* getXInput();

    //! Query if this System is initializing.
    //! \return true if initializing, false if not.
    bool isInitializing() const;

    inline SystemPtr ptr() { return shared_from_this(); }

    //! Destructor.
    ~System();
  };

#endif

  //! @}

}