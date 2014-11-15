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
      wideString mRawPath; //!< The full input device raw path

      RawInputDevice( System* system, DeviceID id, HANDLE rawHandle, wideString& rawPath );
    public:
      virtual ~RawInputDevice();
      virtual const Handler getHandler() const;
      virtual const DeviceID getStaticID() const;

      //! Get the RawInput device handle.
      virtual const HANDLE getRawHandle() const;

      //! Get the RawInput device path.
      virtual const wideString& getRawPath() const;

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

  //! \addtogroup Mouse
  //! @{

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

  typedef map<HANDLE, RawInputMouse*> RawMouseMap;
  typedef map<HANDLE, RawInputKeyboard*> RawKeyboardMap;

  //! @}

}