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
  //! \brief Input device information entry.
  class Device
  {
    friend class System;
    friend class XInputController;
    public:
      //! \brief Device handler types.
      enum Handler: int
      {
        Handler_DirectInput = 0, //!< Implemented by DirectInput
        Handler_XInput, //!< Implemented by XInput
        Handler_RawInput //!< Implemented by Raw Input API
      };
      //! \brief Device types.
      enum Type: int
      {
        Device_Keyboard = 0, //!< I'm a keyboard
        Device_Mouse, //!< I'm a mouse
        Device_Controller //!< I'm a controller
      };
      //! \brief Device status types.
      enum Status: int
      {
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

      //! \brief Constructor.
      //! \param [in,out] system The owning input system.
      //! \param  id             Device identifier.
      //! \param  type           Device type.
      explicit Device( System* system, DeviceID id, Type type );
      //! \brief Destructor.
      virtual ~Device();
      virtual void update(); //!< Update our instance
      virtual void setStatus( Status status ); //!< Set status
      virtual void saveStatus(); //!< Backup current status
      virtual const Status getSavedStatus(); //!< Get backed up status
      virtual void onDisconnect(); //!< On unplugged or otherwise disabled
      virtual void onConnect(); //!< On plugged or otherwise enabled
      virtual void flagDisconnected(); //!< Flag as disconnected for deletion on next update
    public:
      virtual void enable(); //!< Create our instance
      virtual void disable(); //!< Destroy our instance
      virtual DeviceInstance* getInstance(); //!< Get our instance, if available
      virtual const DeviceID getID() const; //!< Get session-specific identifier
      virtual const DeviceID getStaticID() const = 0; //!< Get static identifier
      virtual const Handler getHandler() const = 0; //!< Get handler
      virtual const Type getType() const; //!< Get type
      virtual const Status getStatus() const; //!< Get status
      virtual const String& getName() const; //!< Get name
      virtual System* getSystem(); //!< Get owner system
      virtual const bool isDisconnectFlagged() const; //!< Are we flagged for disconnection?
  };

  //! \brief A list of devices.
  typedef list<Device*> DeviceList;

  //! \brief Information about a raw input device.
  class RawInputDeviceInfo
  {
    protected:
      RID_DEVICE_INFO* mRawInfo;  //!< Internal raw input information

      //! \brief Raw information resolve type.
      //! \return A Device::Type.
      const Device::Type rawInfoResolveType() const;
    public:
      //! \brief Constructor.
      //! \param  handle Handle of the handle.
      RawInputDeviceInfo( HANDLE handle );
      //! \brief Destructor.
      ~RawInputDeviceInfo();
  };

  //! \class RawInputDevice
  //! \brief Device abstraction base class for Raw Input API devices.
  //! \sa RawInputDeviceInfo
  //! \sa Device
  class RawInputDevice: public RawInputDeviceInfo, public Device
  {
    friend class System;
    protected:
      HANDLE mRawHandle; //!< The internal raw input device handle
      String mRawPath; //!< The full input device raw path

      //! \brief Constructor.
      //! \param [in,out] system  The system.
      //! \param  id              The identifier.
      //! \param  rawHandle       Handle of the raw.
      //! \param [in,out] rawPath Full pathname of the raw file.
      RawInputDevice( System* system, DeviceID id, HANDLE rawHandle, String& rawPath );
    public:
      //! \brief Destructor.
      virtual ~RawInputDevice();

      //! \brief Handler, called when the get.
      //! \return The handler.
      virtual const Handler getHandler() const;

      //! \brief Gets the static identifier.
      //! \return The static identifier.
      virtual const DeviceID getStaticID() const;

      //! \brief Gets the raw input handle.
      //! \return The raw handle.
      virtual const HANDLE getRawHandle() const;

      //! \brief Gets the raw input path.
      //! \return The raw path.
      virtual const String& getRawPath() const;

      //! \brief Gets the raw input information.
      //! \return null if it fails, else the raw information.
      virtual const RID_DEVICE_INFO* getRawInfo() const;
  };

  //! \class DirectInputDevice
  //! \brief Device abstraction base class for DirectInput devices.
  //! \sa Device
  class DirectInputDevice: public Device
  {
    //! \brief A system.
    //! \sa Device
    friend class System;
    protected:
      GUID mProductID; //!< DirectInput's product identifier
      GUID mInstanceID; //!< DirectInput's instance identifier

      //! \brief Constructor.
      //! \param [in,out] system The system.
      //! \param  id             The identifier.
      //! \param  instance       The instance.
      DirectInputDevice( System* system, DeviceID id,
        LPCDIDEVICEINSTANCEW instance );
    public:
      //! \brief Handler, called when the get.
      //! \return The handler.
      virtual const Handler getHandler() const;

      //! \brief Gets static identifier.
      //! \return The static identifier.
      virtual const DeviceID getStaticID() const;

      //! \brief Gets product identifier.
      //! \return The product identifier.
      virtual const GUID getProductID() const;

      //! \brief Gets instance identifier.
      //! \return The instance identifier.
      virtual const GUID getInstanceID() const;
  };

  //! \class XInputDevice
  //! \brief Device abstraction base class for XInput devices.
  //! \sa Device
  class XInputDevice: public Device
  {
    friend class System;
    protected:
      int mXInputID; //!< Internal XInput device identifier
      bool mIdentified; //!< true if identified
      XINPUT_CAPABILITIES mCapabilities; //!< Internal XInput capabilities

      //! \brief Constructor.
      //! \param [in,out] system If non-null, the system.
      //! \param  id             The identifier.
      //! \param  xinputID       Identifier for the xinput.
      XInputDevice( System* system, DeviceID id, int xinputID );

      //! \brief Identifies this XInputDevice.
      virtual void identify(); //!< Internally identify our type

      //! \brief Sets the status.
      //! \param  status The status.
      virtual void setStatus( Status status );

      //! \brief Executes the disconnect action.
      virtual void onDisconnect();

      //! \brief Executes the connect action.
      virtual void onConnect();
    public:
      //! \brief Handler, called when the get.
      //! \return The handler.
      virtual const Handler getHandler() const;

      //! \brief Gets static identifier.
      //! \return The static identifier.
      virtual const DeviceID getStaticID() const;

      //! \brief Get x coordinate input identifier.
      //! \return The x coordinate input identifier.
      virtual const int getXInputID() const;

      //! \brief Gets the capabilities.
      //! \return The capabilities.
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
      //! \brief Constructor.
      //! \param [in,out] system If non-null, the system.
      //! \param [in,out] device If non-null, the device.
      DeviceInstance( System* system, Device* device );

      //! \brief Updates this DeviceInstance.
      virtual void update() = 0;

      //! \brief Gets the device.
      //! \return null if it fails, else the device.
      virtual const Device* getDevice() const;

      //! \brief Destructor.
      virtual ~DeviceInstance();
  };

  //! \brief A list of device instances.
  typedef list<DeviceInstance*> DeviceInstanceList;

  //! \addtogroup Mouse
  //! @{

  //! \struct MouseState
  //! \brief Mouse state structure.
  struct MouseState
  {
    public:
      //! \brief Default constructor.
      MouseState();
      void reset(); //!< Reset the state of one-shot components
      vector<Button> mButtons; //!< My buttons
      Wheel mWheel; //!< My wheel
      Movement mMovement; //!< My movement
  };

  //! \class MouseListener
  //! \brief Mouse event listener base class.
  //!  Derive your own listener from this class.
  class MouseListener
  {
    public:
      //! \brief Mouse move event.
      //! \param  mouse         The mouse.
      //! \param  state         The state.
      virtual void onMouseMoved(
        Mouse* mouse, const MouseState& state ) = 0;

      //! \brief Mouse button press event.
      //! \param  mouse         The mouse.
      //! \param  state         The state.
      //! \param  button        The button.
      virtual void onMouseButtonPressed(
        Mouse* mouse, const MouseState& state, size_t button ) = 0;

      //! \brief Mouse button release event.
      //! \param  mouse         The mouse.
      //! \param  state         The state.
      //! \param  button        The button.
      virtual void onMouseButtonReleased(
        Mouse* mouse, const MouseState& state, size_t button ) = 0;

      //! \brief Mouse wheel move event.
      //! \param  mouse         The mouse.
      //! \param  state         The state.
      virtual void onMouseWheelMoved(
        Mouse* mouse, const MouseState& state ) = 0;
  };

  //! \brief A list of mouse input listeners.
  typedef list<MouseListener*> MouseListenerList;

  //! \class Mouse
  //! \brief Mouse device instance base class.
  //! \sa DeviceInstance
  class Mouse: public DeviceInstance
  {
    protected:
      MouseState mState;  //!< Current state
      Vector2i mLastPosition; //!< Previous position when mouse gives absolutes
      MouseListenerList mListeners; //!< Registered event listeners
    public:
      //! \brief Constructor.
      //! \param system The system.
      //! \param device The device.
      Mouse( System* system, Device* device );

      //! \brief Add a mouse input listener.
      //! \param listener The listener.
      virtual void addListener( MouseListener* listener );

      //! \brief Remove a mouse input listener.
      //! \param listener The listener.
      virtual void removeListener( MouseListener* listener );

      //! \brief Updates this Mouse.
      //!        This is called by System, no need to do it yourself.
      virtual void update() = 0;

      //! \brief Destructor.
      virtual ~Mouse();
  };

  //! \class RawInputMouse
  //! \brief Mouse implemented by Raw Input API.
  //! \sa Mouse
  class RawInputMouse: public Mouse
  {
    //! \brief A system.
    friend class System;
    protected:
      unsigned int mSampleRate; //!< The sample rate
      bool mHorizontalWheel;  //!< true to horizontal wheel

      //! \brief Internal, handle incoming input.
      //! \param  input The input.
      virtual void onRawInput( const RAWMOUSE& input );
    public:
      //! \brief Constructor.
      //! \param device The device.
      RawInputMouse( RawInputDevice* device );

      //! \copydoc Mouse::update()
      virtual void update();

      //! \brief Destructor.
      virtual ~RawInputMouse();
  };

  //! @}

  //! \addtogroup Keyboard
  //! @{

  //! \class KeyboardListener
  //! \brief Keyboard event listener base class.
  //!  Derive your own listener from this class.
  class KeyboardListener
  {
    public:
      //! \brief Key press event.
      //! \param [in,out] keyboard The keyboard.
      //! \param  keycode          The keycode.
      virtual void onKeyPressed(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;

      //! \brief Key repeat event.
      //! \param [in,out] keyboard The keyboard.
      //! \param  keycode          The keycode.
      virtual void onKeyRepeat(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;

      //! \brief Key release event.
      //! \param [in,out] keyboard The keyboard.
      //! \param  keycode          The keycode.
      virtual void onKeyReleased(
        Keyboard* keyboard, const VirtualKeyCode keycode ) = 0;
  };

  //! \brief A list of keyboard input listeners.
  typedef list<KeyboardListener*> KeyboardListenerList;

  //! \class Keyboard
  //! \brief Keyboard device instance base class.
  //! \sa DeviceInstance
  class Keyboard: public DeviceInstance
  {
    protected:
      KeyboardListenerList mListeners;  //!< Registered event listeners
    public:
      //! \brief KeyCode values.
      enum KeyCode: VirtualKeyCode
      {
        Key_LeftShift = 0xA0, //!< An enum constant representing the key left shift option
        Key_RightShift,
        Key_LeftControl,
        Key_RightControl,
        Key_LeftAlt,
        Key_RightAlt,
        Key_NumpadEnter = 0xD8 // Random unused code for our repurposing
      };

      //! \brief Constructor.
      //! \param system The system.
      //! \param device The device.
      Keyboard( System* system, Device* device );

      //! \brief Add a keyboard input listener.
      //! \param listener The listener.
      virtual void addListener( KeyboardListener* listener );

      //! \brief Remove a keyboard input listener.
      //! \param listener The listener.
      virtual void removeListener( KeyboardListener* listener );

      //! \brief Updates this Keyboard.
      //!        This is called by System, no need to do it yourself.
      virtual void update() = 0;

      //! \brief Destructor.
      virtual ~Keyboard();
  };

  //! \class RawInputKeyboard
  //! \brief Keyboard implemented by Raw Input API.
  //! \sa Keyboard
  class RawInputKeyboard: public Keyboard
  {
    //! \brief A system.
    friend class System;
    protected:
      list<VirtualKeyCode> mPressedKeys;  //!< List of keys that are currently pressed

      //! \brief Internal, handle incoming input.
      //! \param  input The input.
      virtual void onRawInput( const RAWKEYBOARD& input );
    public:
      //! \brief Constructor.
      //! \param device The device.
      RawInputKeyboard( RawInputDevice* device );

      //! \copydoc Keyboard::update()
      virtual void update();

      //! \brief Destructor.
      virtual ~RawInputKeyboard();
  };

  //! @}

  //! \addtogroup Controller
  //! @{

  //! \struct ControllerState
  //! \brief Game controller state structure.
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
  //! \brief Game controller event listener base class.
  //!  Derive your own listener from this class.
  class ControllerListener
  {
    public:
      //! \brief Digital button press event.
      //! \param  controller  The controller.
      //! \param  state       The state.
      //! \param  button      The button.
      virtual void onControllerButtonPressed( Controller* controller,
        const ControllerState& state, size_t button ) = 0;

      //! \brief Digital button release event.
      //! \param  controller  The controller.
      //! \param  state       The state.
      //! \param  button      The button.
      virtual void onControllerButtonReleased(  Controller* controller,
        const ControllerState& state, size_t button ) = 0;

      //! \brief Axis move event.
      //! \param  controller  The controller.
      //! \param  state       The state.
      //! \param  axis        The axis.
      virtual void onControllerAxisMoved( Controller* controller,
        const ControllerState& state, size_t axis ) = 0;

      //! \brief Slider move event.
      //! \param  controller  The controller.
      //! \param  state       The state.
      //! \param  slider      The slider.
      virtual void onControllerSliderMoved( Controller* controller,
        const ControllerState& state, size_t slider ) = 0;

      //! \brief POV (D-pad) move event.
      //! \param  controller  The controller.
      //! \param  state       The state.
      //! \param  pov         The POV.
      virtual void onControllerPOVMoved( Controller* controller,
        const ControllerState& state, size_t pov ) = 0;
  };

  //! \brief A list of controller input listeners.
  typedef list<ControllerListener*> ControllerListenerList;

  //! \class Controller
  //! \brief Game controller device instance base class.
  //! \sa DeviceInstance
  class Controller: public DeviceInstance
  {
    public:
      //! \brief Type of controller.
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
      ControllerListenerList mListeners;  //!< Registered state change listeners

      //! \brief Figure out changes in state and fire change events accordingly.
      //! \param lastState Previous state.
      virtual void fireChanges( const ControllerState& lastState );
    public:
      //! \brief Constructor.
      //! \param system The system.
      //! \param device The device.
      Controller( System* system, Device* device );

      //! \brief Destructor.
      virtual ~Controller();

      //! \brief Add a controller input listener.
      //! \param listener The listener.
      virtual void addListener( ControllerListener* listener );

      //! \brief Remove a controller input listener.
      //! \param listener The listener.
      virtual void removeListener( ControllerListener* listener );

      //! \brief Get the Controller type.
      //! \return The type.
      virtual const Type getType() const;

      //! \brief Get the Controller state.
      //! \return The state.
      virtual const ControllerState& getState() const;
  };

  //! \class DirectInputController
  //! \brief Game controller implemented by DirectInput.
  //! \sa Controller
  class DirectInputController: public Controller
  {
    protected:
      IDirectInputDevice8W* mDIDevice; //!< The DirectInput device
      DIDEVCAPS mDICapabilities; //!< DirectInput capabilities
      size_t mAxisEnum; //!< Internal axis enumeration
      size_t mSliderEnum; //!< Internal slider enumeration

      //! \brief Axis value filter.
      //! \param  val The value.
      //! \return A Real.
      inline Real filterAxis( int val );

      //! \brief Callback, called when the di components enum.
      //! \param  component The component.
      //! \param  referer   The referer.
      //! \return A CALLBACK.
      static BOOL CALLBACK diComponentsEnumCallback(
        LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer );
    public:
      //! \brief Constructor.
      //! \param device The device.
      DirectInputController( DirectInputDevice* device );

      //! \copydoc Controller::update()
      virtual void update();

      //! \brief Destructor.
      virtual ~DirectInputController();
  };

  //! \class XInputController
  //! \brief Game controller implemented by XInput.
  //! \sa Controller
  class XInputController: public Controller
  {
    protected:
      DWORD mLastPacket; //!< Internal previous input packet's ID
      XINPUT_STATE mXInputState; //!< Internal XInput state

      //! \brief Filter a left thumb axis value.
      //! \param  val The value.
      //! \return A Real.
      inline Real filterLeftThumbAxis( int val );

      //! \brief Filter a right thumb axis value.
      //! \param  val The value.
      //! \return A Real.
      inline Real filterRightThumbAxis( int val );

      //! \brief Filter a trigger value.
      //! \param  val The value.
      //! \return A Real.
      inline Real filterTrigger( int val );
    public:
      //! \brief Constructor.
      //! \param device The device.
      XInputController( XInputDevice* device );

      //! \copydoc Controller::update()
      virtual void update();

      //! \brief Destructor.
      virtual ~XInputController();
  };

  //! @}

  //! \brief A map of raw mouse input handlers.
  typedef map<HANDLE,RawInputMouse*> RawMouseMap;
  //! \brief A map of raw keyboard input handlers.
  typedef map<HANDLE,RawInputKeyboard*> RawKeyboardMap;

  //! \class SystemListener
  //! \brief System event listener base class.
  //!  Derive your own listener from this class.
  class SystemListener
  {
    public:
      //! \brief Executes the device connected action.
      //! \param device The device.
      virtual void onDeviceConnected( Device* device ) = 0;

      //! \brief Executes the device disconnected action.
      //! \param device The device.
      virtual void onDeviceDisconnected( Device* device ) = 0;

      //! \brief Executes the mouse enabled action.
      //! \param device   The device.
      //! \param instance The mouse instance.
      virtual void onMouseEnabled( Device* device, Mouse* instance ) = 0;

      //! \brief Executes the keyboard enabled action.
      //! \param device   The device.
      //! \param instance The keyboard instance.
      virtual void onKeyboardEnabled( Device* device, Keyboard* instance ) = 0;

      //! \brief Executes the controller enabled action.
      //! \param device   The device.
      //! \param instance The controller instance.
      virtual void onControllerEnabled( Device* device, Controller* instance ) = 0;

      //! \brief Executes the mouse disabled action.
      //! \param device   The device.
      //! \param instance The mouse instance.
      virtual void onMouseDisabled( Device* device, Mouse* instance ) = 0;

      //! \brief Executes the keyboard disabled action.
      //! \param device   The device.
      //! \param instance The keyboard instance.
      virtual void onKeyboardDisabled( Device* device, Keyboard* instance ) = 0;

      //! \brief Executes the controller disabled action.
      //! \param device   The device.
      //! \param instance The controller instance.
      virtual void onControllerDisabled( Device* device, Controller* instance ) = 0;
  };

  //! \brief The input system root.
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
    protected:
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

      //! \brief \b Internal Initialize the devices.
      void initializeDevices();

      //! \brief \b Internal Refresh devices.
      void refreshDevices();

      //! \brief \b Internal Identify XInput devices.
      void identifyXInputDevices();

      //! \brief \b Internal Get the next assignable device identifier.
      //! \return The next identifier.
      DeviceID getNextID();

      //! \brief \b Internal Get the next mouse index.
      //! \return The next mouse index.
      int getNextMouseIndex();

      //! \brief \b Internal Get the next keyboard index.
      //! \return The next keyboard index.
      int getNextKeyboardIndex();

      //! \brief \b Internal Get the next controller index.
      //! \return The next controller index.
      int getNextControllerIndex();

      //! \brief \b Internal Run device connect event.
      //! \param device The device.
      void deviceConnect( Device* device );

      //! \brief \b Internal Run device disconnect event.
      //! \param device The device.
      void deviceDisconnect( Device* device );

      //! \brief \b Internal On mouse enabled.
      //! \param device   The device.
      //! \param instance The instance.
      void mouseEnabled( Device* device, Mouse* instance );

      //! \brief \b Internal On keyboard enabled.
      //! \param device   The device.
      //! \param instance The instance.
      void keyboardEnabled( Device* device, Keyboard* instance );

      //! \brief \b Internal On controller enabled.
      //! \param device   The device.
      //! \param instance The instance.
      void controllerEnabled( Device* device, Controller* instance );

      //! \brief \b Internal On mouse disabled.
      //! \param device   The device.
      //! \param instance The instance.
      void mouseDisabled( Device* device, Mouse* instance );

      //! \brief \b Internal On keyboard disabled.
      //! \param device   The device.
      //! \param instance The instance.
      void keyboardDisabled( Device* device, Keyboard* instance );

      //! \brief \b Internal On controller disabled.
      //! \param device   The device.
      //! \param instance The instance.
      void controllerDisabled( Device* device, Controller* instance );

      //! \brief \b Internal Map incoming raw mouse events to a device.
      //! \param  handle  Raw input handle.
      //! \param  mouse   The mouse.
      void mapMouse( HANDLE handle, RawInputMouse* mouse );

      //! \brief \b Internal Unmap incoming raw mouse events for a handle.
      //! \param  handle  Raw input handle.
      void unmapMouse( HANDLE handle );

      //! \brief \b Internal Map incoming raw keyboard events to a device.
      //! \param  handle    Raw input handle.
      //! \param  keyboard  The keyboard.
      void mapKeyboard( HANDLE handle, RawInputKeyboard* keyboard );

      //! \brief \b Internal Unmap incoming raw keyboard events for a handle.
      //! \param  handle  Raw input handle.
      void unmapKeyboard( HANDLE handle );

      //! \brief \b Internal My PnP plug callback.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );

      //! \brief \b Internal My PnP unplug callback.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );

      //! \brief \b Internal My Raw arrival callback.
      //! \param  handle Handle of the raw input device.
      virtual void onRawArrival( HANDLE handle );

      //! \brief \b Internal My Raw mouse input callback.
      //! \param  handle Handle of the raw input device.
      //! \param  input  The input.
      virtual void onRawMouseInput( HANDLE handle, const RAWMOUSE& input );

      //! \brief \b Internal My Raw keyboard input callback.
      //! \param  handle Handle of the raw input device.
      //! \param  input  The input.
      virtual void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input );

      //! \brief \b Internal My Raw removal callback.
      //! \param  handle Handle of the raw input device.
      virtual void onRawRemoval( HANDLE handle );

      //! \brief \b Internal My DirectInput device enumeration callback.
      //! \param  instance The instance.
      //! \param  referer  The referer.
      //! \return A CALLBACK.
      static BOOL CALLBACK diDeviceEnumCallback(
        LPCDIDEVICEINSTANCEW instance, LPVOID referer );
    public:
      //! \brief Constructor.
      //! \param  instance  Handle of the host instance.
      //! \param  window    Handle of the host window.
      //! \param  listener  Listener for system events.
      System( HINSTANCE instance, HWND window, SystemListener* listener );

      //! \brief Updates this System.
      //!  All listened events get triggered from inside this call.
      void update();

      //! \brief Get currently known devices.
      //! \return The devices.
      DeviceList& getDevices();

      //! \brief Get Logitech G-Key SDK, if available.
      //! \return null if it fails, else the Logitech G-Keys SDK object.
      Logitech::GKeySDK* getLogitechGKeys();

      //! \brief Get Logitech LED SDK, if available.
      //! \return null if it fails, else the Logitech LED SDK object.
      Logitech::LedSDK* getLogitechLEDs();

      //! \brief Query if this System is initializing.
      //! \return true if initializing, false if not.
      const bool isInitializing() const;

      //! \brief Destructor.
      ~System();
  };

  //! @}

}