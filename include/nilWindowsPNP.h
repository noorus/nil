#pragma once
#include "nilConfig.h"
#include "nilTypes.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace nil {

  //! \addtogroup Nil
  //! @{

  namespace windows {

    //! USB Usage Page IDs.
    //! See See USB HID Usage Tables version 1.1, page 15
    enum USBUsagePage {
      USBUsagePage_Desktop = 0x01
    };

    //! USB Generic Desktop Page usage IDs.
    //! See USB HID Usage Tables version 1.1, page 27
    enum USBDesktopUsage {
      USBDesktopUsage_Mice = 0x02,
      USBDesktopUsage_Keyboards = 0x06
    };

    //! \class PnPListener
    //! Plug-n-Play event listener base class.
    class PnPListener {
    public:
      //! Plug-n-Play device plug event.
      virtual void onPnPPlug( const GUID& deviceClass, const wideString& devicePath ) = 0;

      //! Plug-n-Play device unplug event.
      virtual void onPnPUnplug( const GUID& deviceClass, const wideString& devicePath ) = 0;
    };

    //! A list of Plug-and-Play event listeners.
    using PnPListenerList = list<PnPListener*>;

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

      //! Raw input device removal event.
      virtual void onRawRemoval( HANDLE handle ) = 0;
    };

    //! A list of raw input device listeners.
    using RawListenerList = list<RawListener*>;

    //! \class EventMonitor
    //! Monitors for Plug-n-Play (USB) and Raw device events.
    class EventMonitor {
    protected:
      HINSTANCE instance_; //!< Host application instance handle
      ATOM class_; //!< Class registration handle
      HWND window_; //!< Window handle
      HDEVNOTIFY notifications_; //!< Device notifications registration
      PnPListenerList pnpListeners_; //!< Our Plug-n-Play listeners
      RawListenerList rawListeners_; //!< Our raw listeners
      void* inputBuffer_; //!< Buffer for input reads
      unsigned int inputBufferSize_; //!< Size of input buffer
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
      void registerPnPListener( PnPListener* listener );

      //! Unregister a listener from Plug-n-Play events.
      void unregisterPnPListener( PnPListener* listener );

      //! Register a listener for raw input events.
      void registerRawListener( RawListener* listener );

      //! Unregister a listener from raw input events.
      void unregisterRawListener( RawListener* listener );

      //! Update the EventMonitor, triggering new events.
      void update();

      ~EventMonitor();
    };

    //! Known USB vendor IDs that might be useful.
    enum USBKnownVendor: uint16_t {
      USBVendor_Microsoft = 0x045E, //!< Microsoft
      USBVendor_Logitech = 0x046D //!< Logitech
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
      bool available_; //!< Is this device actually available?
      bool isXInput_; //!< Am I an XInput device?
      bool isRDP_; //!< Am I a Remote Desktop device?

      void identify(); //!< \b Internal Figure out what I am
    public:
      //! Constructor.
      //! \param  path   Full system path to the device.
      //! \param  handle Device handle.
      HIDRecord( const wideString& path, HANDLE handle );

      //! Am I available for usage?
      bool isAvailable() const;

      //! Am I a Remote Desktop device?
      bool isRDP() const;

      //! Am I an XInput device?
      bool isXInput() const;

      //! Am I a Microsoft device?
      bool isMicrosoft() const;

      //! Am I a Logitech device?
      bool isLogitech() const;

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

      //! Get USB usage page ID.
      uint16_t getUsagePage() const;

      //! Get USB usage ID.
      uint16_t getUsage() const;

      //! Get combined VID/PID identifier.
      uint32_t getIdentifier() const;

      //! Destructor.
      ~HIDRecord() = default;
    };

    //! \brief A list of HID records.
    using HIDRecordList = list<HIDRecord*>;

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

      //! Destructor.
      virtual ~HIDManager();
    };

  }

  //! @}

}