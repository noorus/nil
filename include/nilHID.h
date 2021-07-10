#pragma once
#include "nilTypes.h"
#include "nilPnP.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace nil {

  //! \addtogroup Nil
  //! @{

  //! Known USB vendor IDs that might be useful.
  enum USBKnownVendor: uint16_t
  {
    USBVendor_Microsoft = 0x045E, //!< Microsoft
    USBVendor_Logitech  = 0x046D //!< Logitech
  };

  //! \class HIDRecord
  //! A Human Interface Device instance currently present in the system.
  //! \sa HIDManager
  class HIDRecord
  {
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
      ~HIDRecord();
  };

  //! \brief A list of HID records.
  using HIDRecordList = list<HIDRecord*>;

  //! \class HIDManager
  //! Manages a list of connected Human Interface Devices.
  //! \note The HIDManager has to be registered as PnPListener on an EventMonitor.
  //! \sa PnPListener
  //! \sa EventMonitor
  class HIDManager: public PnPListener
  {
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
      ~HIDManager();
  };

  //! @}

}