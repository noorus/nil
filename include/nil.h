#pragma once

#include "nilTypes.h"
#include "nilException.h"
#include "nilPnP.h"
#include "nilHID.h"

namespace nil {

  class System;
  class DeviceInstance;

  class Mouse;
  class Keyboard;
  class Controller;

  //! \struct Button
  //! Digital push button component.
  struct Button {
  public:
    bool pushed; //!< Pushed state
    Button();
  };

  //! \struct Axis
  //! Analog axis controller component.
  struct Axis {
  public:
    Real absolute; //!< Absolute value in {-1..1}
    Axis();
  };

  //! \struct Slider
  //! Two-dimensional analog controller component.
  struct Slider {
  public:
    Vector2i absolute; //!< Absolute value in [{-1..1},{-1..1}]
    Slider();
  };

  //! \struct POV
  //! Digital directional Point-of-View controller component.
  //! Also known as the D-pad.
  struct POV {
  public:
    static const POVDirection Centered   = 0x00000000;
    static const POVDirection North      = 0x00000001;
    static const POVDirection South      = 0x00000010;
    static const POVDirection East       = 0x00000100;
    static const POVDirection West       = 0x00001000;
    static const POVDirection NorthEast  = 0x00000101;
    static const POVDirection SouthEast  = 0x00000110;
    static const POVDirection NorthWest  = 0x00001001;
    static const POVDirection SouthWest  = 0x00001010;
    POVDirection direction; //!< Absolute current directions
    POV();
  };

  //! \struct Wheel
  //! Mouse wheel component.
  struct Wheel {
  public:
    int relative; //!< Wheel rotation delta, in eights of a degree
    Wheel();
  };

  //! \struct Movement
  //! Mouse movement component.
  struct Movement {
  public:
    Vector2i relative; //!< Relative value change in pixels
    Movement();
  };

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
      Device_Controller //!< No, I'm a controller
    };
    enum Status: int {
      Status_Disconnected = 0, //!< Disconnected but not forgotten
      Status_Pending, //!< Pending refresh
      Status_Connected //!< Up-to-date and available
    };
  protected:
    DeviceID mID; //!< Unique nil-specific identifier
    Status mStatus; //!< Current status
    Status mSavedStatus; //!< Status backup when updating
    String mName; //!< Device name
    Type mType; //!< Device type
    System* mSystem; //!< My owner
    DeviceInstance* mInstance; //!< My instance, if created
    bool mDisconnectFlagged; //!< Has there been a problem with me?
    explicit Device( System* system, DeviceID id, Type type );
    virtual ~Device();
    virtual void create(); //!< Create our instance
    virtual void update(); //!< Update our instance
    virtual void destroy(); //!< Destroy our instance
    virtual void setStatus( Status status ); //!< Set status
    virtual void saveStatus(); //!< Backup current status
    virtual const Status getSavedStatus(); //!< Get backed up status
    virtual void onDisconnect(); //!< On unplugged or otherwise disabled
    virtual void onConnect(); //!< On plugged or otherwise enabled
    virtual void flagDisconnected(); //!< Flag for disconnection
  public:
    virtual const DeviceID getID(); //!< Get unique identifier
    virtual const Handler getHandler() = 0; //!< Get handler
    virtual const Type getType(); //!< Get type
    virtual const Status getStatus(); //!< Get status
    virtual const String& getName(); //!< Get name
    virtual System* getSystem(); //!< Get owner system
    virtual const bool isDisconnectFlagged(); //!< Are we flagged for disconnection?
  };

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
    virtual const Handler getHandler();
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
    virtual const Handler getHandler();
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
    virtual const Handler getHandler();
    virtual const int getXInputID();
    virtual const XINPUT_CAPABILITIES& getCapabilities(); //!< Get XInput caps
  };

  typedef list<Device*> DeviceList;

  //! \class DeviceInstance
  //! Device instance base class.
  class DeviceInstance {
  protected:
    System* mSystem;
    Device* mDevice;
  public:
    DeviceInstance( System* system, Device* device );
    virtual void update() = 0;
    virtual ~DeviceInstance();
  };

  //! \struct MouseState
  //! Mouse state structure.
  struct MouseState {
  public:
    void reset(); //!< Reset the state of one-shot components
    MouseState();
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
    MouseListenerList mListeners; //!< Registered state change listeners
  public:
    Mouse( System* system, Device* device );
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

  //! \class Keyboard
  //! Keyboard device instance base class.
  class Keyboard: public DeviceInstance {
  protected:
  public:
    Keyboard( System* system, Device* device );
    virtual void update() = 0;
    virtual ~Keyboard();
  };

  //! \class RawInputKeyboard
  //! Keyboard implemented by Raw Input API.
  class RawInputKeyboard: public Keyboard {
  friend class System;
  protected:
    virtual void onRawInput( const RAWKEYBOARD& input );
  public:
    RawInputKeyboard( RawInputDevice* device );
    virtual void update();
    virtual ~RawInputKeyboard();
  };

  //! \struct ControllerState
  //! Game controller state structure.
  struct ControllerState {
  public:
    void reset(); //!< Reset the state of my components
    ControllerState();
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
    inline Real filterThumbAxis( int val );
    inline Real filterTrigger( int val );
  public:
    XInputController( XInputDevice* device );
    virtual void update();
    virtual ~XInputController();
  };

  typedef map<HANDLE,RawInputMouse*> RawMouseMap;
  typedef map<HANDLE,RawInputKeyboard*> RawKeyboardMap;

  //! \class System
  //! The input system root.
  class System: public PnPListener, public RawListener {
  friend class DirectInputController;
  friend class RawInputKeyboard;
  friend class RawInputMouse;
  protected:
    DeviceID mIDPool; //!< Device indexing pool
    vector<DeviceID> mXInputIDs; //!< XInput device ID mapping
    vector<uint32_t> mXInputDeviceIDs; //!< Tracked list of XInput VIDs & PIDs
    IDirectInput8W* mDirectInput; //!< Our DirectInput instance
    HINSTANCE mInstance; //!< Host application instance handle
    HWND mWindow; //!< Host application window handle
    PnPMonitor* mMonitor; //!< Our Plug-n-Play event monitor
    DeviceList mDevices; //!< List of known devices
    HIDManager* mHIDManager; //!< Our HID manager
    bool mInitializing; //!< Are we initializing?
    RawMouseMap mMouseMapping; //!< Mouse events mapping
    RawKeyboardMap mKeyboardMapping; //!< Keyboard events mapping
    void initializeDevices();
    void refreshDevices();
    void identifyXInputDevices();
    DeviceID getNextID();
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
    System( HINSTANCE instance, HWND window );
    void update();
    const bool isInitializing();
    ~System();
  };

}