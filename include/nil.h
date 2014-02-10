#pragma once

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilPnP.h"
#include "nilHID.h"
#include "nilLogitech.h"

namespace nil {

  class System;
  class DeviceInstance;

  class Mouse;
  class Keyboard;
  class Controller;

  // Device implementations

  //! \class Device
  //! Input device information entry.
  class Device {
  friend class System;
  friend class XInputController;
  public:
    enum Handler: int {
      Handler_DirectInput = 0, //!< Implemented by DirectInput
      Handler_XInput, //!< Implemented by XInput
      Handler_RawInput //!< Implemented by Raw Input API
    };
    enum Type: int {
      Device_Keyboard = 0, //!< I'm a keyboard
      Device_Mouse, //!< I'm a mouse
      Device_Controller //!< I'm a controller
    };
    enum Status: int {
      Status_Disconnected = 0, //!< Disconnected but not forgotten
      Status_Pending, //!< Pending refresh
      Status_Connected //!< Up-to-date and available
    };
  protected:
    DeviceID mID; //!< Unique identifier, only valid per System-session
    Status mStatus; //!< Current status
    Status mSavedStatus; //!< Status backup when updating
    String mName; //!< Device name
    Type mType; //!< Device type
    System* mSystem; //!< My owner
    DeviceInstance* mInstance; //!< My instance, if created
    bool mDisconnectFlagged; //!< Has there been a problem with me?
    int mTypedIndex; //!< This is a device-type-specific index for the device
    explicit Device( System* system, DeviceID id, Type type );
    virtual ~Device();
    void initAfterTyped();
    virtual void update(); //!< Update our instance
    virtual void setStatus( Status status ); //!< Set status
    virtual void saveStatus(); //!< Backup current status
    virtual const Status getSavedStatus(); //!< Get backed up status
    virtual void onDisconnect(); //!< On unplugged or otherwise disabled
    virtual void onConnect(); //!< On plugged or otherwise enabled
    virtual void flagDisconnected(); //!< Flag for disconnection
  public:
    virtual void enable(); //!< Create our instance
    virtual void disable(); //!< Destroy our instance
    virtual DeviceInstance* getInstance(); //!< Get our instance, if available
    virtual const DeviceID getID() const; //!< Get unique identifier
    virtual const Handler getHandler() const = 0; //!< Get handler
    virtual const Type getType() const; //!< Get type
    virtual const Status getStatus() const; //!< Get status
    virtual const String& getName() const; //!< Get name
    virtual System* getSystem() const; //!< Get owner system
    virtual const bool isDisconnectFlagged() const; //!< Are we flagged for disconnection?
  };

  typedef list<Device*> DeviceList;

  //! \class RawInputDevice
  //! Device abstraction base class for Raw Input API devices.
  class RawInputDevice: public Device {
  friend class System;
  protected:
    HANDLE mRawHandle;
    String mRawPath;
    RID_DEVICE_INFO* mRawInfo;
    RawInputDevice( System* system, DeviceID id, HANDLE rawHandle, String& rawPath );
  public:
    virtual ~RawInputDevice();
    virtual const Handler getHandler() const;
    virtual const HANDLE getRawHandle();
    virtual const String& getRawPath();
    virtual const RID_DEVICE_INFO* getRawInfo();
  };

  //! \class DirectInputDevice
  //! Device abstraction base class for DirectInput devices.
  class DirectInputDevice: public Device {
  friend class System;
  protected:
    GUID mProductID;
    GUID mInstanceID;
    DirectInputDevice( System* system, DeviceID id,
      LPCDIDEVICEINSTANCEW instance );
  public:
    virtual const Handler getHandler() const;
    virtual const GUID getProductID();
    virtual const GUID getInstanceID();
  };

  //! \class XInputDevice
  //! Device abstraction base class for XInput devices.
  class XInputDevice: public Device {
  friend class System;
  protected:
    int mXInputID;
    bool mIdentified;
    XINPUT_CAPABILITIES mCapabilities;
    XInputDevice( System* system, DeviceID id, int xinputID );
    virtual void identify(); //!< Internally identify our type
    virtual void setStatus( Status status );
    virtual void onDisconnect();
    virtual void onConnect();
  public:
    virtual const Handler getHandler() const;
    virtual const int getXInputID();
    virtual const XINPUT_CAPABILITIES& getCapabilities(); //!< Get XInput caps
  };

  //! \class DeviceInstance
  //! Device instance base class.
  class DeviceInstance {
  protected:
    System* mSystem;
    Device* mDevice;
  public:
    DeviceInstance( System* system, Device* device );
    virtual void update() = 0;
    virtual const Device* getDevice();
    virtual ~DeviceInstance();
  };

  typedef list<DeviceInstance*> DeviceInstanceList;

  // Mouse implementations

  //! \struct MouseState
  //! Mouse state structure.
  struct MouseState {
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
  class MouseListener {
  public:
    virtual void onMouseMoved(
      Mouse* mouse, const MouseState& state ) = 0;
    virtual void onMouseButtonPressed(
      Mouse* mouse, const MouseState& state, size_t button ) = 0;
    virtual void onMouseButtonReleased(
      Mouse* mouse, const MouseState& state, size_t button ) = 0;
    virtual void onMouseWheelMoved(
      Mouse* mouse, const MouseState& state ) = 0;
  };

  typedef list<MouseListener*> MouseListenerList;

  //! \class Mouse
  //! Mouse device instance base class.
  class Mouse: public DeviceInstance {
  protected:
    MouseState mState; //!< Current state
    Vector2i mLastPosition; //!< Previous position when mouse gives absolutes
    MouseListenerList mListeners; //!< Registered event listeners
  public:
    Mouse( System* system, Device* device );
    virtual void addListener( MouseListener* listener );
    virtual void removeListener( MouseListener* listener );
    virtual void update() = 0;
    virtual ~Mouse();
  };

  //! \class RawInputMouse
  //! Mouse implemented by Raw Input API.
  class RawInputMouse: public Mouse {
  friend class System;
  protected:
    unsigned int mSampleRate;
    bool mHorizontalWheel;
    virtual void onRawInput( const RAWMOUSE& input );
  public:
    RawInputMouse( RawInputDevice* device );
    virtual void update();
    virtual ~RawInputMouse();
  };

  // Keyboard implementations

  //! \class KeyboardListener
  //! Keyboard event listener base class.
  //! Derive your own listener from this class.
  class KeyboardListener {
  public:
    virtual void onKeyPressed(
      Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;
    virtual void onKeyRepeat(
      Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;
    virtual void onKeyReleased(
      Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;
  };

  typedef list<KeyboardListener*> KeyboardListenerList;

  //! \class Keyboard
  //! Keyboard device instance base class.
  class Keyboard: public DeviceInstance {
  protected:
    KeyboardListenerList mListeners; //!< Registered event listeners
  public:
    enum KeyCode: VirtualKeyCode {
      Key_LeftShift = 0xA0, // As defined by Windows
      Key_RightShift,
      Key_LeftControl,
      Key_RightControl,
      Key_LeftAlt,
      Key_RightAlt,
      Key_NumpadEnter = 0xD8 // Random unused code for our repurposing
    };
    Keyboard( System* system, Device* device );
    virtual void addListener( KeyboardListener* listener );
    virtual void removeListener( KeyboardListener* listener );
    virtual void update() = 0;
    virtual ~Keyboard();
  };

  //! \class RawInputKeyboard
  //! Keyboard implemented by Raw Input API.
  class RawInputKeyboard: public Keyboard {
  friend class System;
  protected:
    list<VirtualKeyCode> mPressedKeys; //!< List of keys that are currently down
    virtual void onRawInput( const RAWKEYBOARD& input );
  public:
    RawInputKeyboard( RawInputDevice* device );
    virtual void update();
    virtual ~RawInputKeyboard();
  };

  // Controller implementations

  //! \struct ControllerState
  //! Game controller state structure.
  struct ControllerState {
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
  class ControllerListener {
  public:
    virtual void onControllerButtonPressed(
      Controller* controller, const ControllerState& state, size_t button ) = 0;
    virtual void onControllerButtonReleased(
      Controller* controller, const ControllerState& state, size_t button ) = 0;
    virtual void onControllerAxisMoved(
      Controller* controller, const ControllerState& state, size_t axis ) = 0;
    virtual void onControllerSliderMoved(
      Controller* controller, const ControllerState& state, size_t slider ) = 0;
    virtual void onControllerPOVMoved(
      Controller* controller, const ControllerState& state, size_t pov ) = 0;
  };

  typedef list<ControllerListener*> ControllerListenerList;

  //! \class Controller
  //! Game controller device instance base class.
  class Controller: public DeviceInstance {
  public:
    enum Type: int {
      Controller_Unknown = 0, //!< I don't know what I am
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
    virtual void fireChanges( const ControllerState& lastState );
  public:
    Controller( System* system, Device* device );
    virtual ~Controller();
    virtual void addListener( ControllerListener* listener );
    virtual void removeListener( ControllerListener* listener );
    virtual const Type getType() const;
    virtual const ControllerState& getState() const;
  };

  //! \class DirectInputController
  //! Game controller implemented by DirectInput.
  class DirectInputController: public Controller {
  protected:
    IDirectInputDevice8W* mDIDevice;
    DIDEVCAPS mDICapabilities;
    int mAxisEnum;
    int mSliderEnum;
    inline Real filterAxis( int val );
    static BOOL CALLBACK diComponentsEnumCallback(
      LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer );
  public:
    DirectInputController( DirectInputDevice* device );
    virtual void update();
    virtual ~DirectInputController();
  };

  //! \class XInputController
  //! Game controller implemented by XInput.
  class XInputController: public Controller {
  protected:
    DWORD mLastPacket;
    XINPUT_STATE mXInputState;
    inline Real filterLeftThumbAxis( int val );
    inline Real filterRightThumbAxis( int val );
    inline Real filterTrigger( int val );
  public:
    XInputController( XInputDevice* device );
    virtual void update();
    virtual ~XInputController();
  };

  // System implementation

  typedef map<HANDLE,RawInputMouse*> RawMouseMap;
  typedef map<HANDLE,RawInputKeyboard*> RawKeyboardMap;

  //! \class SystemListener
  //! System event listener base class.
  //! Derive your own listener from this class.
  class SystemListener {
  public:
    virtual void onDeviceConnected( Device* device ) = 0;
    virtual void onDeviceDisconnected( Device* device ) = 0;
    virtual void onMouseEnabled( Device* device, Mouse* instance ) = 0;
    virtual void onKeyboardEnabled( Device* device, Keyboard* instance ) = 0;
    virtual void onControllerEnabled( Device* device, Controller* instance ) = 0;
    virtual void onMouseDisabled( Device* device, Mouse* instance ) = 0;
    virtual void onKeyboardDisabled( Device* device, Keyboard* instance ) = 0;
    virtual void onControllerDisabled( Device* device, Controller* instance ) = 0;
  };

  //! \class System
  //! The input system root.
  class System: public PnPListener, public RawListener {
  friend class Device;
  friend class DirectInputController;
  friend class RawInputKeyboard;
  friend class RawInputMouse;
  protected:
    DeviceID mIDPool; //!< Device indexing pool
    int mMouseIndexPool; //!< Mouse indexing pool
    int mKeyboardIndexPool; //!< Keyboard indexing pool
    int mControllerIndexPool; //!< Controller indexing pool
    vector<DeviceID> mXInputIDs; //!< XInput device ID mapping
    vector<uint32_t> mXInputDeviceIDs; //!< Tracked list of XInput VIDs & PIDs
    IDirectInput8W* mDirectInput; //!< Our DirectInput instance
    HINSTANCE mInstance; //!< Host application instance handle
    HWND mWindow; //!< Host application window handle
    EventMonitor* mMonitor; //!< Our Plug-n-Play & raw input event monitor
    DeviceList mDevices; //!< List of known devices
    HIDManager* mHIDManager; //!< Our HID manager
    bool mInitializing; //!< Are we initializing?
    RawMouseMap mMouseMapping; //!< Raw mouse events mapping
    RawKeyboardMap mKeyboardMapping; //!< Raw keyboard events mapping
    Logitech::GKeySDK* mLogitechGKeys; //!< External module for Logitech G-Keys
    Logitech::LedSDK* mLogitechLEDs; //!< External module for Logitech LEDs
    SystemListener* mListener; //!< Our single event listener
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
    virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );
    virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );
    virtual void onRawArrival( HANDLE handle );
    virtual void onRawMouseInput( HANDLE handle, const RAWMOUSE& input );
    virtual void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input );
    virtual void onRawRemoval( HANDLE handle );
    static BOOL CALLBACK diDeviceEnumCallback(
      LPCDIDEVICEINSTANCEW instance, LPVOID referer );
  public:
    System( HINSTANCE instance, HWND window, SystemListener* listener );
    void update();
    DeviceList& getDevices();
    Logitech::GKeySDK* getLogitechGKeys();
    Logitech::LedSDK* getLogitechLEDs();
    const bool isInitializing();
    ~System();
  };

}