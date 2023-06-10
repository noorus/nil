#pragma once
#include "nilConfig.h"
#include "nilTypes.h"
#include "nilPredefs.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace nil {

  //! \addtogroup Nil
  //! @{
  //!

  namespace windows {

    //! \class PnPListener
    //! Plug-n-Play event listener base class.
    class PnPListener {
    public:
      //! Plug-n-Play device plug event.
      virtual void onPnPPlug( const GUID& deviceClass, const wideString& devicePath ) = 0;

      //! Plug-n-Play device unplug event.
      virtual void onPnPUnplug( const GUID& deviceClass, const wideString& devicePath ) = 0;
    };

    using PnPListenerPtr = PnPListener*;

    //! A list of Plug-and-Play event listeners.
    using PnPListenerList = list<PnPListenerPtr>;

    //! \class RawListener
    //! Raw input event listener base class.
    class RawListener {
    public:
      //! Raw input device arrival event.
      virtual void onRawArrival( HANDLE handle ) = 0;

      //! Raw mouse input event.
      virtual void onRawMouseInput( HANDLE handle, const RAWMOUSE& input, const bool sinked ) = 0;

      //! Raw keyboard input event.
      virtual void onRawKeyboardInput( HANDLE handle, const RAWKEYBOARD& input, const bool sinked ) = 0;

      //! Raw HID input event.
      virtual void onRawHIDInput( HANDLE handle, const RAWHID& input, const bool sinked ) = 0;

      //! Raw input device removal event.
      virtual void onRawRemoval( HANDLE handle ) = 0;
    };

    using RawListenerPtr = RawListener*;

    //! A list of raw input device listeners.
    using RawListenerList = list<RawListenerPtr>;

    //! \class EventMonitor
    //! Monitors for Plug-n-Play (USB) and Raw device events.
    class EventMonitor {
    protected:
      HINSTANCE instance_; //!< Host application instance handle
      ATOM class_ = 0; //!< Class registration handle
      HWND window_ = nullptr; //!< Window handle
      HDEVNOTIFY notifications_ = nullptr; //!< Device notifications registration
      PnPListenerList pnpListeners_; //!< Our Plug-n-Play listeners
      RawListenerList rawListeners_; //!< Our raw listeners
      vector<uint8_t> inputBuffer_; //!< Buffer for input reads
      const Cooperation coop_; //!< Cooperation mode
    protected:
      //! \b Internal Register myself for event notifications.
      void registerNotifications();

      //! \b Internal Unregister myself from event notifications.
      void unregisterNotifications();

      //! \b Internal Handle interface arrival.
      void handleInterfaceArrival( const GUID& deviceClass,
        const wideString& devicePath );

      //! \b Internal Handle interface removal.
      void handleInterfaceRemoval( const GUID& deviceClass,
        const wideString& devicePath );

      //! \b Internal Handle raw device arrival.
      void handleRawArrival( HANDLE handle );

      //! \b Internal Handle raw input.
      void handleRawInput( HRAWINPUT input, const bool sinked );

      //! \b Internal Handle raw device removal.
      void handleRawRemoval( HANDLE handle );

      //! \b Internal Hidden window procedure for receiving messages.
      static LRESULT CALLBACK wndProc( HWND window, UINT message,
        WPARAM wParam, LPARAM lParam );
    public:
      EventMonitor( HINSTANCE instance, const Cooperation coop );

      //! Register a listener for Plug-n-Play events.
      void registerPnPListener( PnPListenerPtr listener );

      //! Unregister a listener from Plug-n-Play events.
      void unregisterPnPListener( PnPListenerPtr listener );

      //! Register a listener for raw input events.
      void registerRawListener( RawListenerPtr listener );

      //! Unregister a listener from raw input events.
      void unregisterRawListener( RawListenerPtr listener );

      //! Update the EventMonitor, triggering new events.
      void update();

      ~EventMonitor();
    };

    //! \class HIDRecord
    //! A Human Interface Device instance currently present in the system.
    //! \sa HIDManager
    class HIDRecord {
    private:
      wideString path_; //!< Raw device path
      uint16_t usbVid_; //!< USB Vendor ID for this device
      uint16_t usbPid_; //!< USB Product ID for this device
      uint32_t hidIdent_; //!< Combined HID identifier
      HIDP_CAPS caps_; //!< HID API capabilities
      utf8String name_; //!< Device name
      utf8String manufacturer_; //!< Device manufacturer
      utf8String serial_; //!< Device serial number

      bool available_ = false; //!< Is this device actually available?
      bool isXInput_ = false; //!< Am I an XInput device?
      bool isRDP_ = false; //!< Am I a Remote Desktop device?

      const KnownDeviceRecord* knownDevice_ = nullptr;

      void identify(); //!< \b Internal Figure out what I am
    public:
      //! Constructor.
      //! \param  path   Full system path to the device.
      //! \param  handle Device handle.
      HIDRecord( const wideString& path, HANDLE handle );

      HIDConnectionType connectionType() const;

      KnownDeviceType knownDeviceType() const;

      //! Am I available for usage?
      bool isAvailable() const;

      //! Am I a Remote Desktop device?
      bool isRDP() const;

      //! Am I an XInput device?
      bool isXInput() const;

      //! Get full device path.
      const wideString& getPath() const;

      //! Get device name.
      const utf8String& getName() const;

      //! Get manufacturer name.
      //! Can be empty.
      const utf8String& getManufacturer() const;

      //! Get serial number.
      //! Can be empty.
      const utf8String& getSerialNumber() const;

      //! Format a pretty display name.
      //! Called by RawInputDevice.
      utf8String makePrettyName( const utf8String& raw, int typedIndex ) const;

      //! Get USB usage page ID.
      uint16_t getUsagePage() const;

      //! Get USB usage ID.
      uint16_t getUsage() const;

      //! Get combined VID/PID identifier.
      uint32_t getIdentifier() const;

      //! Destructor.
      ~HIDRecord() = default;
    };

    using HIDRecordPtr = shared_ptr<HIDRecord>;

    //! \brief A list of HID records.
    using HIDRecordList = list<HIDRecordPtr>;

    //! \class HIDManager
    //! Manages a list of connected Human Interface Devices.
    //! \note The HIDManager has to be registered as PnPListener on an EventMonitor.
    //! \sa PnPListener
    //! \sa EventMonitor
    class HIDManager: public PnPListener {
    private:
      HIDRecordList records_; //!< Records container

      //! \b Internal My PnP plug callback.
      void onPnPPlug( const GUID& deviceClass, const wideString& devicePath ) override;

      //! \b Internal My PnP unplug callback.
      void onPnPUnplug( const GUID& deviceClass, const wideString& devicePath ) override;

      //! \b Internal Process an existing device.
      void processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
        SP_DEVINFO_DATA& deviceData, const wideString& devicePath );

      //! \b Internal Initialization stuff
      void initialize();
    public:
      //! Default constructor.
      HIDManager();

      //! Get the list of active HID records.
      const HIDRecordList& getRecords() const;

      HIDRecordPtr getRecordByPath( const wideString& devicePath );

      //! Destructor.
      virtual ~HIDManager();
    };

  }

  //! @}

}