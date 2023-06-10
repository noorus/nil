#pragma once
#include "nilConfig.h"

#include "nilTypes.h"
#include "nilComponents.h"
#include "nilException.h"
#include "nilCommon.h"
#include "nilWindowsPNP.h"
#include "nilPredefs.h"

namespace nil {

  //! \addtogroup Nil
  //! @{

  class System;
  class DeviceInstance;

  class Mouse;
  class Keyboard;
  class Controller;

  //! \class ExternalModule
  //! Base class for an external, optional module supported by the system.
  class ExternalModule {
  protected:
    HMODULE module_; //!< The module handle
    bool initialized_; //!< Whether the module is initialized
  public:
    ExternalModule();

    //! Possible initialization call return values.
    enum InitReturn: unsigned int {
      Initialization_OK = 0,  //!< Module initialized OK
      Initialization_ModuleNotFound, //!< Module was not found
      Initialization_MissingExports, //!< Module was missing expected exports
      Initialization_Unavailable //!< Module was unavailable for use
    };

    //! Initializes this ExternalModule.
    virtual InitReturn initialize() = 0;

    //! Shuts down this ExternalModule and frees any resources it is using.
    virtual void shutdown() = 0;

    //! Query whether the module is initialized or not.
    virtual bool isInitialized() const;
  };

  class RawInputDeviceInfo {
  protected:
    vector<uint8_t> rawDeviceInfo_;
    Device::Type rawInfoResolveType() const;
  public:
    RawInputDeviceInfo( HANDLE handle );
    virtual ~RawInputDeviceInfo();

    //! Get the RawInput device information structure.
    virtual const RID_DEVICE_INFO* getRawInfo() const;
  };

  //! \class RawInputDevice
  //! Device abstraction base class for Raw Input API devices.
  //! \sa RawInputDeviceInfo
  //! \sa Device
  class RawInputDevice: public RawInputDeviceInfo, public Device, public std::enable_shared_from_this<RawInputDevice> {
  friend class System;
  private:
    HANDLE rawHandle_; //!< The internal raw input device handle
    wideString rawPath_; //!< The full input device raw path
    windows::HIDRecordPtr hidRecord_; //!< The associated HID record entry
  public:
    RawInputDevice( SystemPtr system, DeviceID id, HANDLE rawHandle, wideString& rawPath, windows::HIDRecordPtr hid );
    virtual ~RawInputDevice();

    Handler getHandler() const override;
    DeviceID getStaticID() const override;
    shared_ptr<Device> ptr() override { return dynamic_pointer_cast<Device>( shared_from_this() ); }

    //! Get the RawInput device handle.
    virtual HANDLE getRawHandle() const;

    //! Get the RawInput device path.
    virtual const wideString& getRawPath() const;

    //! Get the associated HID record entry.
    windows::HIDRecordPtr getHIDReccord() const;
  };

  using RawInputDevicePtr = shared_ptr<RawInputDevice>;

  //! \class DirectInputDevice
  //! Device abstraction base class for DirectInput devices.
  //! \sa Device
  class DirectInputDevice: public Device, public std::enable_shared_from_this<DirectInputDevice> {
  friend class System;
  private:
    GUID pid_; //!< DirectInput's product identifier
    GUID inst_; //!< DirectInput's instance identifier

  public:
    DirectInputDevice( SystemPtr system, DeviceID id, LPCDIDEVICEINSTANCEW instance );

    Handler getHandler() const override;
    DeviceID getStaticID() const override;
    shared_ptr<Device> ptr() override { return dynamic_pointer_cast<Device>( shared_from_this() ); }

    //! Get the DirectInput product ID.
    virtual const GUID getProductID() const;

    //! Get the DirectInput instance ID.
    virtual const GUID getInstanceID() const;
  };

  using DirectInputDevicePtr = shared_ptr<DirectInputDevice>;

  using fnXInputGetState = DWORD( WINAPI* )( DWORD dwUserIndex, XINPUT_STATE* pState );
  using fnXInputSetState = DWORD( WINAPI* )( DWORD dwUserIndex, XINPUT_VIBRATION* pVibration );
  using fnXInputGetCapabilities = DWORD( WINAPI* )( DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities );

  //! \class XInput
  //! XInput dynamic module loader.
  //! \sa ExternalModule
  class XInput: public ExternalModule {
  public:
    enum Version {
      Version_None = 0,
      Version_910,
      Version_13,
      Version_14
    } version_ = Version_None;
    struct Functions {
      fnXInputGetState pfnXInputGetState = nullptr;
      fnXInputSetState pfnXInputSetState = nullptr;
      fnXInputGetCapabilities pfnXInputGetCapabilities = nullptr;
    } funcs_;
    InitReturn initialize() override;
    void shutdown() override;
    virtual ~XInput();
  };

  //! \class XInputDevice
  //! Device abstraction base class for XInput devices.
  //! \sa Device
  class XInputDevice: public Device, public std::enable_shared_from_this<XInputDevice> {
  friend class System;
  private:
    int xinputId_; //!< Internal XInput device identifier
    XINPUT_CAPABILITIES caps_; //!< Internal XInput capabilities
    bool identified_ = false; //!< true if identified
    virtual void identify();
    void setStatus( Status status ) override;
    void onDisconnect() override;
    void onConnect() override;

  public:
    XInputDevice( SystemPtr system, DeviceID id, int xinputID );

    Handler getHandler() const override;
    DeviceID getStaticID() const override;
    shared_ptr<Device> ptr() override { return dynamic_pointer_cast<Device>( shared_from_this() ); }

    //! Get the XInput device ID.
    virtual int getXInputID() const;

    //! Get the XInput device capabilities.
    virtual const XINPUT_CAPABILITIES& getCapabilities() const; //!< Get XInput caps
  };

  using XInputDevicePtr = shared_ptr<XInputDevice>;

  //! \addtogroup Mouse
  //! @{

  //! \class RawInputMouse
  //! Mouse implemented by Raw Input API.
  //! \sa Mouse
  class RawInputMouse: public Mouse, public std::enable_shared_from_this<RawInputMouse> {
  friend class System;
  private:
    unsigned int sampleRate_; //!< The sample rate
    bool hasHorizontalWheel_; //!< true to horizontal wheel

    //! My raw input callback.
    virtual void onRawInput( const RAWMOUSE& input );
  public:
    //! Constructor.
    //! \param device The device.
    //! \param swapButtons Whether to swap the first & second buttons.
    RawInputMouse( RawInputDevicePtr device, const bool swapButtons );

    void update() override;

    shared_ptr<DeviceInstance> ptr() override { return dynamic_pointer_cast<DeviceInstance>( shared_from_this() ); }

    //! Destructor.
    virtual ~RawInputMouse();
  };

  using RawInputMousePtr = shared_ptr<RawInputMouse>;

  //! @}

  //! \addtogroup Keyboard
  //! @{

  //! \class RawInputKeyboard
  //! Keyboard implemented by Raw Input API.
  //! \sa Keyboard
  class RawInputKeyboard: public Keyboard, public std::enable_shared_from_this<RawInputKeyboard> {
  friend class System;
  private:
    set<VirtualKeyCode> pressedKeys_; //!< Set of keys that are currently pressed

    //! My raw input callback.
    virtual void onRawInput( const RAWKEYBOARD& input );
  public:
    //! Constructor.
    //! \param device The device.
    RawInputKeyboard( RawInputDevicePtr device );

    void update() override;

    shared_ptr<DeviceInstance> ptr() override { return dynamic_pointer_cast<DeviceInstance>( shared_from_this() ); }

    //! Destructor.
    virtual ~RawInputKeyboard();
  };

  using RawInputKeyboardPtr = shared_ptr<RawInputKeyboard>;

  //! @}

  //! \addtogroup Controller
  //! @{

  //! \class RawInputController
  //! Controller implemented by Raw Input API.
  //! \sa Controller
  class RawInputController : public Controller, public std::enable_shared_from_this<RawInputController> {
  friend class System;
  private:
    //! My raw input callback.
    virtual void onRawInput( const RAWHID& input );
    KnownDeviceType devType_ = KnownDevice_Unknown;
    HIDConnectionType connType_ = HIDConnection_Unknown;
    void setupDualSense();
    void handleDualSense( const uint8_t* buf );
  public:
    //! Constructor.
    //! \param device The device.
    RawInputController( RawInputDevicePtr device );

    void update() override;

    shared_ptr<DeviceInstance> ptr() override { return dynamic_pointer_cast<DeviceInstance>( shared_from_this() ); }

    //! Destructor.
    virtual ~RawInputController();
  };

  using RawInputControllerPtr = shared_ptr<RawInputController>;

  //! @}

  //! \addtogroup Controller
  //! @{

  //! \class DirectInputController
  //! Game controller implemented by DirectInput.
  //! \sa Controller
  class DirectInputController: public Controller, public std::enable_shared_from_this<DirectInputController> {
  private:
    IDirectInputDevice8W* diDevice_ = nullptr; //!< The DirectInput device
    DIDEVCAPS diCaps_; //!< DirectInput capabilities
    size_t axisEnum_ = 0; //!< Internal axis enumeration
    size_t sliderEnum_; //!< Internal slider enumeration
    const Cooperation coop_; //!< Cooperation mode

    //! Axis value filter.
    inline Real filterAxis( int val );

    //! DirectInput controller components enumeration callback.
    static BOOL CALLBACK diComponentsEnumCallback(
      LPCDIDEVICEOBJECTINSTANCEW component, LPVOID referer );
  public:
    //! Constructor.
    //! \param device The device.
    DirectInputController( DirectInputDevicePtr device, const Cooperation coop );

    void update() override;

    shared_ptr<DeviceInstance> ptr() override { return dynamic_pointer_cast<DeviceInstance>( shared_from_this() ); }

    //! Destructor.
    virtual ~DirectInputController();
  };

  using DirectInputControllerPtr = shared_ptr<DirectInputController>;

  //! \class XInputController
  //! Game controller implemented by XInput.
  //! \sa Controller
  class XInputController: public Controller, public std::enable_shared_from_this<XInputController> {
  private:
    DWORD lastPacket_ = 0; //!< Internal previous input packet's ID
    XINPUT_STATE xinputState_ = { 0 }; //!< Internal XInput state

    //! Filter a thumb axis value.
    inline Real filterThumbAxis( int val, int deadzone );

    //! Filter a trigger value.
    inline Real filterTrigger( int val );
  public:
    //! Constructor.
    //! \param device The device.
    XInputController( XInputDevicePtr device );

    void update() override;

    shared_ptr<DeviceInstance> ptr() override { return dynamic_pointer_cast<DeviceInstance>( shared_from_this() ); }

    //! Destructor.
    virtual ~XInputController();
  };

  using XInputControllerPtr = shared_ptr<XInputController>;

  //! @}

  using RawMouseMap = map<HANDLE, RawInputMouse*>;
  using RawKeyboardMap = map<HANDLE, RawInputKeyboard*>;
  using RawControllerMap = map<HANDLE, RawInputController*>;

  //! @}

}