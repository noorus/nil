#pragma once
#include "nilTypes.h"
#include "nilPnP.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace Nil {

  //! Known USB vendor IDs that might be useful.
  enum USBKnownVendor: uint16_t
  {
    USBVendor_Microsoft = 0x045E,
    USBVendor_Logitech  = 0x046D
  };

  //! \class HIDRecord
  //! A Human Interface Device instance currently present in the system.
  class HIDRecord
  {
    protected:
      String mPath;
      uint16_t mVendorID; //!< USB Vendor ID for this device
      uint16_t mProductID; //!< USB Product ID for this device
      uint32_t mIdentifier; //!< Combined HID identifier
      HIDP_CAPS mCapabilities; //!< HID API capabilities
      String mName; //!< Device name
      String mManufacturer; //!< Device manufacturer
      String mSerialNumber; //!< Device serial number
      bool mIsXInput; //!< Is this an XInput device?
      bool mIsRDP; //!< Is this a Remote Desktop device?
      void identify();
    public:
      HIDRecord( const String& path, HANDLE handle );
      bool isRDP() const; //!< Is this a Remote Desktop device?
      bool isXInput() const; //!< Is this an XInput device?
      bool isMicrosoft() const; //!< Is this a Microsoft device?
      bool isLogitech() const; //!< Is this a Logitech device?
      const String& getPath() const; //!< Get device path
      const String& getName() const; //!< Get device name
      const String& getManufacturer() const; //!< Get manufacturer name
      const String& getSerialNumber() const; //!< Get serial number
      uint16_t getUsagePage() const; //!< Get USB usage page ID
      uint16_t getUsage() const; //!< Get USB usage ID
      uint32_t getIdentifier() const; //!< Get combined VID/PID identifier
      ~HIDRecord();
  };

  typedef list<HIDRecord*> HIDRecordList;

  //! \class HIDManager
  //! Manages a list of connected Human Interface Devices.
  class HIDManager: public PnPListener
  {
    protected:
      HIDRecordList mRecords; //!< Records container
      virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );
      virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );
      void processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
        SP_DEVINFO_DATA& deviceData, const String& devicePath );
      void initialize();
    public:
      HIDManager();
      const HIDRecordList& getRecords() const; //!< Get the list of active HID records
      ~HIDManager();
  };

}