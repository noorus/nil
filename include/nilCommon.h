#pragma once

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilPnP.h"
#include "nilHID.h"

namespace nil {

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
      utf8String mName; //!< Device name
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
      virtual const utf8String& getName() const; //!< Get my name
      virtual System* getSystem(); //!< Get my owning system
      virtual const bool isDisconnectFlagged() const; //!< Am I flagged for disconnection?
  };

  //! A list of devices.
  typedef list<Device*> DeviceList;

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
      bool mSwapButtons; //!< Whether first & second buttons are swapped
    public:
      //! Constructor.
      //! \param system The system.
      //! \param device The device.
      //! \param swapButtons Whether to swap first & second buttons.
      Mouse( System* system, Device* device, const bool swapButtons );

      //! Add a mouse input listener.
      //! \param listener The listener.
      virtual void addListener( MouseListener* listener );

      //! Remove a mouse input listener.
      //! \param listener The listener.
      virtual void removeListener( MouseListener* listener );

      virtual void update() = 0;

      //! Get the Mouse state.
      virtual const MouseState& getState() const;

      //! Destructor.
      virtual ~Mouse();
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

  //! @}

  //! @}

}