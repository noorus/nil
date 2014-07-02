#pragma once

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilPnP.h"
#include "nilHID.h"
#include "nilLogitech.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  class System;
  class DeviceInstance;

  class Mouse;
  class Keyboard;
  class Controller;

  //! \class Device
  //! A generic input device in the system.
  class Device
  {
    friend class System;
    friend class XInputController;
    public:
      //! Device handler types.
      enum Handler: int
      {
        Handler_DirectInput = 0, //!< Implemented by DirectInput
        Handler_XInput, //!< Implemented by XInput
        Handler_RawInput //!< Implemented by Raw Input API
      };
      //! Device types.
      enum Type: int
      {
        Device_Keyboard = 0, //!< I'm a keyboard
        Device_Mouse, //!< I'm a mouse
        Device_Controller //!< I'm a controller
      };
      //! Device status types.
      enum Status: int
      {
        Status_Disconnected = 0, //!< Disconnected but not forgotten
        Status_Pending, //!< Pending refresh
        Status_Connected //!< Up-to-date and available
      };
    protected:
      DeviceID mID; //!< Unique identifier, only valid per session
      Status mStatus; //!< Current status
      Status mSavedStatus; //!< Status backup when updating
      String mName; //!< Device name
      Type mType; //!< Device type
      System* mSystem; //!< My owner
      DeviceInstance* mInstance; //!< My instance, if created
      bool mDisconnectFlagged; //!< Whether I am flagged for disconnection or not
      int mTypedIndex; //!< This is a device-type-specific index for the device

      explicit Device( System* system, DeviceID id, Type type );
      virtual ~Device();
      virtual void update(); //!< Update our instance
      virtual void setStatus( Status status ); //!< Set status
      virtual void saveStatus(); //!< Backup current status
      virtual const Status getSavedStatus(); //!< Get backed up status
      virtual void onDisconnect(); //!< On unplugged or otherwise disabled
      virtual void onConnect(); //!< On plugged or otherwise enabled
      virtual void flagDisconnected(); //!< Flag as disconnected for deletion on next update
    public:
      virtual void enable(); //!< Enable the device, creating a DeviceInstance
      virtual void disable(); //!< Disable the device, destroying it's DeviceInstance
      virtual DeviceInstance* getInstance(); //!< Get my instance, if available
      virtual const DeviceID getID() const; //!< Get session-specific identifier
      virtual const DeviceID getStaticID() const = 0; //!< Get static identifier
      virtual const Handler getHandler() const = 0; //!< Get my handler
      virtual const Type getType() const; //!< Get my type
      virtual const Status getStatus() const; //!< Get my status
      virtual const String& getName() const; //!< Get my name
      virtual System* getSystem(); //!< Get my owning system
      virtual const bool isDisconnectFlagged() const; //!< Am I flagged for disconnection?
  };

  //! A list of devices.
  typedef list<Device*> DeviceList;

  class RawInputDeviceInfo
  {
    protected:
      RID_DEVICE_INFO* mRawInfo;
      const Device::Type rawInfoResolveType() const;
    public:
      RawInputDeviceInfo( HANDLE handle );
      ~RawInputDeviceInfo();
  };

  //! \class RawInputDevice
  //! Device abstraction base class for Raw Input API devices.
  //! \sa RawInputDeviceInfo
  //! \sa Device
  class RawInputDevice: public RawInputDeviceInfo, public Device
  {
    friend class System;
    private:
      HANDLE mRawHandle; //!< The internal raw input device handle
      String mRawPath; //!< The full input device raw path

      RawInputDevice( System* system, DeviceID id, HANDLE rawHandle, String& rawPath );
    public:
      virtual ~RawInputDevice();
      virtual const Handler getHandler() const;
      virtual const DeviceID getStaticID() const;

      //! Get the RawInput device handle.
      virtual const HANDLE getRawHandle() const;

      //! Get the RawInput device path.
      virtual const String& getRawPath() const;

      //! Get the RawInput device information structure.
      virtual const RID_DEVICE_INFO* getRawInfo() const;
  };

  //! \class DirectInputDevice
  //! Device abstraction base class for DirectInput devices.
  //! \sa Device
  class DirectInputDevice: public Device
  {
    friend class System;
    private:
      GUID mProductID; //!< DirectInput's product identifier
      GUID mInstanceID; //!< DirectInput's instance identifier

      DirectInputDevice( System* system, DeviceID id,
        LPCDIDEVICEINSTANCEW instance );
    public:
      virtual const Handler getHandler() const;
      virtual const DeviceID getStaticID() const;

      //! Get the DirectInput product ID.
      virtual const GUID getProductID() const;

      //! Get the DirectInput instance ID.
      virtual const GUID getInstanceID() const;
  };

  //! \class XInputDevice
  //! Device abstraction base class for XInput devices.
  //! \sa Device
  class XInputDevice: public Device
  {
    friend class System;
    private:
      int mXInputID; //!< Internal XInput device identifier
      bool mIdentified; //!< true if identified
      XINPUT_CAPABILITIES mCapabilities; //!< Internal XInput capabilities

      XInputDevice( System* system, DeviceID id, int xinputID );
      virtual void identify();
      virtual void setStatus( Status status );
      virtual void onDisconnect();
      virtual void onConnect();
    public:
      virtual const Handler getHandler() const;
      virtual const DeviceID getStaticID() const;

      //! Get the XInput device ID.
      virtual const int getXInputID() const;

      //! Get the XInput device capabilities.
      virtual const XINPUT_CAPABILITIES& getCapabilities() const; //!< Get XInput caps
  };

  //! \class DeviceInstance
  //! Device instance base class.
  class DeviceInstance
  {
    protected:
      System* mSystem; //!< The system
      Device* mDevice; //!< The device
    public:
      //! Constructor.
      //! \param system The system.
      //! \param device The device.
      DeviceInstance( System* system, Device* device );

      //! Update this DeviceInstance.
      //! \note This is called by System, no need to do it yourself.
      virtual void update() = 0;

      //! Get the device that owns me.
      virtual const Device* getDevice() const;

      //! Destructor.
      virtual ~DeviceInstance();
  };

  //! \brief A list of device instances.
  typedef list<DeviceInstance*> DeviceInstanceList;

  //! \addtogroup Mouse
  //! @{

  //! \struct MouseState
  //! Mouse state structure.
  struct MouseState
  {
    public:
      MouseState();
      void reset(); //!< Reset the state of one-shot components
      vector<Button> mButtons; //!< My buttons
      Wheel mWheel; //!< My wheel
      Movement mMovement; //!< My movement
  };

  //! \class MouseListener
  //! Mouse event listener base class.
  //! Derive your own listener from this class.
  class MouseListener
  {
    public:
      //! Called when the mouse is moved.
      virtual void onMouseMoved(
        Mouse* mouse, const MouseState& state ) = 0;

      //! Called when a button is pressed on the mouse.
      virtual void onMouseButtonPressed(
        Mouse* mouse, const MouseState& state, size_t button ) = 0;

      //! Called when a button is released on the mouse.
      virtual void onMouseButtonReleased(
        Mouse* mouse, const MouseState& state, size_t button ) = 0;

      //! Called when the mouse wheel is rotated.
      virtual void onMouseWheelMoved(
        Mouse* mouse, const MouseState& state ) = 0;
  };

  typedef list<MouseListener*> MouseListenerList;

  //! \class Mouse
  //! Mouse device instance base class.
  //! \sa DeviceInstance
  class Mouse: public DeviceInstance
  {
    protected:
      MouseState mState;  //!< Current state
      Vector2i mLastPosition; //!< Previous position when mouse gives absolutes
      MouseListenerList mListeners; //!< Registered event listeners
    public:
      //! Constructor.
      //! \param system The system.
      //! \param device The device.
      Mouse( System* system, Device* device );

      //! Add a mouse input listener.
      //! \param listener The listener.
      virtual void addListener( MouseListener* listener );

      //! Remove a mouse input listener.
      //! \param listener The listener.
      virtual void removeListener( MouseListener* listener );

      virtual void update() = 0;

      //! Destructor.
      virtual ~Mouse();
  };

  //! \class RawInputMouse
  //! Mouse implemented by Raw Input API.
  //! \sa Mouse
  class RawInputMouse: public Mouse
  {
    friend class System;
    private:
      unsigned int mSampleRate; //!< The sample rate
      bool mHorizontalWheel; //!< true to horizontal wheel

      //! My raw input callback.
      virtual void onRawInput( const RAWMOUSE& input );
    public:
      //! Constructor.
      //! \param device The device.
      RawInputMouse( RawInputDevice* device );

      virtual void update();

      //! Destructor.
      virtual ~RawInputMouse();
  };

  //! @}

  //! \addtogroup Keyboard
  //! @{

  //! \class KeyboardListener
  //! Keyboard event listener base class.
  //! Derive your own listener from this class.
  class KeyboardListener
  {
    public:
      //! Called when a key is pressed on the keyboard.
      virtual void onKeyPressed(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;

      //! Called when a pressed down key triggers a key repeat on the keyboard.
      virtual void onKeyRepeat(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;

      //! Called when a key is released on the keyboard.
      virtual void onKeyReleased(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;
  };

  typedef list<KeyboardListener*> KeyboardListenerList;

  //! \class Keyboard
  //! Keyboard device instance base class.
  //! \sa DeviceInstance
  class Keyboard: public DeviceInstance
  {
    protected:
      KeyboardListenerList mListeners; //!< Registered event listeners
    public:
      //! KeyCode values.
      enum KeyCode: VirtualKeyCode
      {
        Key_LeftShift = 0xA0,
        Key_RightShift,
        Key_LeftControl,
        Key_RightControl,
        Key_LeftAlt,
        Key_RightAlt,
        Key_NumpadEnter = 0xD8 // Random unused code for our repurposing
      };

      //! Constructor.
      //! \param system The system.
      //! \param device The device.
      Keyboard( System* system, Device* device );

      //! Add a keyboard input listener.
      //! \param listener The listener.
      virtual void addListener( KeyboardListener* listener );

      //! Remove a keyboard input listener.
      //! \param listener The listener.
      virtual void removeListener( KeyboardListener* listener );

      virtual void update() = 0;

      //! Destructor.
      virtual ~Keyboard();
  };

  //! \class RawInputKeyboard
  //! Keyboard implemented by Raw Input API.
  //! \sa Keyboard
  class RawInputKeyboard: public Keyboard
  {
    friend class System;
    private:
      list<VirtualKeyCode> mPressedKeys; //!< List of keys that are currently pressed

      //! My raw input callback.
      virtual void onRawInput( const RAWKEYBOARD& input );
    public:
      //! Constructor.
      //! \param device The device.
      RawInputKeyboard( RawInputDevice* device );

      virtual void update();

      //! Destructor.
      virtual ~RawInputKeyboard();
  };

  //! @}

  //! \addtogroup Controller
  //! @{

  //! \struct ControllerState
  //! Game controller state structure.
  struct ControllerState
  {
    public:
      ControllerState();
      void reset(); //!< Reset the state of my components
      vector<Button> mButtons; //!< My buttons
      vector<Axis> mAxes; //!< My axes
      vector<Slider> mSliders; //!< My sliders
      vector<POV> mPOVs; //!< My POVs
  };

  //! \class ControllerListener
  //! Game controller event listener base class.
  //! Derive your own listener from this class.
  class ControllerListener
  {
    public:
      //! Called when a digital push button is pressed on the controller.
      virtual void onControllerButtonPressed( Controller* controller,
        const ControllerState& state, size_t button ) = 0;

      //! Called when a digital push button is released on the controller.
      virtual void onControllerButtonReleased(  Controller* controller,
        const ControllerState& state, size_t button ) = 0;

      //! Called when an analog axis component is moved on the controller.
      virtual void onControllerAxisMoved( Controller* controller,
        const ControllerState& state, size_t axis ) = 0;

      //! Called when a slider component is moved on the controller.
      virtual void onControllerSliderMoved( Controller* controller,
        const ControllerState& state, size_t slider ) = 0;

      //! Called when a POV (D-pad) component is changed on the controller.
      virtual void onControllerPOVMoved( Controller* controller,
        const ControllerState& state, size_t pov ) = 0;
  };

  typedef list<ControllerListener*> ControllerListenerList;

  //! \class Controller
  //! Game controller device instance base class.
  //! \sa DeviceInstance
  class Controller: public DeviceInstance
  {
    public:
      //! Possible controller types.
      enum Type: int {
        Controller_Unknown = 0, //!< An enum constant representing the controller unknown option
        Controller_Joystick, //!< I'm a joystick
        Controller_Gamepad, //!< I'm a gamepad
        Controller_Firstperson, //!< I'm a shootie-thingie
        Controller_Driving, //!< I'm a driving wheel, I guess?
        Controller_Flight, //!< I'm... A cockpit?
        Controller_DancePad, //!< I'm a steppy platform thing
        Controller_Guitar, //!< I'm a guitar. I guess I have 5 strings.
        Controller_Bass, //!< I'm a bass, so, 4 strings..?
        Controller_Drumkit, //!< I'm a drumkit
        Controller_ArcadePad //!< I'm a huge arcade controller
      };
    protected:
      Type mType; //!< The type of controller I am
      ControllerState mState; //!< Current controls state
      ControllerListenerList mListeners; //!< Registered state change listeners

      //! Figure out changes in state and fire change events accordingly.
      virtual void fireChanges( const ControllerState& lastState );
    public:
      //! Constructor.
      //! \param system The system.
      //! \param device The device.
      Controller( System* system, Device* device );

      //! Destructor.
      virtual ~Controller();

      //! Add a controller input listener.
      virtual void addListener( ControllerListener* listener );

      //! Remove a controller input listener.
      virtual void removeListener( ControllerListener* listener );

      virtual void update() = 0;

      //! Get the Controller type.
      virtual const Type getType() const;

      //! Get the Controller state.
      virtual const ControllerState& getState() const;
  };

  //! \class DirectInputController
  //! Game controller implemented by DirectInput.
  //! \sa Controller
  class DirectInputController: public Controller
  {
    private:
      IDirectInputDevice8W* mDIDevice; //!< The DirectInput device
      DIDEVCAPS mDICapabilities; //!< DirectInput capabilities
      size_t mAxisEnum; //!< Internal axis enumeration
      size_t mSliderEnum; //!< Internal slider enumeration
      const Cooperation mCooperation; //!< Cooperation mode

      //! Axis value filter.
      inline Real filterAxis( int val );

      //! DirectInput controller components enumeration callback.
      static BOOL CALLBACK diComponentsEnumCallback(
        LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer );
    public:
      //! Constructor.
      //! \param device The device.
      DirectInputController( DirectInputDevice* device, const Cooperation coop );

      virtual void update();

      //! Destructor.
      virtual ~DirectInputController();
  };

  //! \class XInputController
  //! Game controller implemented by XInput.
  //! \sa Controller
  class XInputController: public Controller
  {
    private:
      DWORD mLastPacket; //!< Internal previous input packet's ID
      XINPUT_STATE mXInputState; //!< Internal XInput state

      //! Filter a left thumb axis value.
      inline Real filterLeftThumbAxis( int val );

      //! Filter a right thumb axis value.
      inline Real filterRightThumbAxis( int val );

      //! Filter a trigger value.
      inline Real filterTrigger( int val );
    public:
      //! Constructor.
      //! \param device The device.
      XInputController( XInputDevice* device );

      virtual void update();

      //! Destructor.
      virtual ~XInputController();
  };

  //! @}

  typedef map<HANDLE,RawInputMouse*> RawMouseMap;
  typedef map<HANDLE,RawInputKeyboard*> RawKeyboardMap;

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
      const Cooperation mCooperation; //!< Cooperation mode
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
      virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );
      //! \b Internal My PnP unplug callback.
      virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );
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

      //! Query if this System is initializing.
      //! \return true if initializing, false if not.
      const bool isInitializing() const;

      //! Destructor.
      ~System();
  };

  //! @}

}