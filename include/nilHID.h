#pragma once
#include "nilTypes.h"
#include "nilPnP.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! Known USB vendor IDs that might be useful.
  enum USBKnownVendor: uint16_t
  {
    USBVendor_Microsoft = 0x045E,
    USBVendor_Logitech  = 0x046D
  };

  //! \class HIDRecord
  //! \brief A Human Interface Device instance currently present in the system.
  class HIDRecord
  {
    protected:
      String mPath; //!< Raw device path
      uint16_t mVendorID; //!< USB Vendor ID for this device
      uint16_t mProductID; //!< USB Product ID for this device
      uint32_t mIdentifier; //!< Combined HID identifier
      HIDP_CAPS mCapabilities; //!< HID API capabilities
      String mName; //!< Device name
      String mManufacturer; //!< Device manufacturer
      String mSerialNumber; //!< Device serial number
      bool mIsXInput; //!< Is this an XInput device?
      bool mIsRDP; //!< Is this a Remote Desktop device?
      void identify(); //!< \b Internal Figure out what I am
    public:
      //! \brief Constructor.
      //! \param  path   Full device path.
      //! \param  handle Device handle.
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
      //! \brief Destructor.
      ~HIDRecord();
  };

  //! \brief A list of HID records.
  typedef list<HIDRecord*> HIDRecordList;

  //! \class HIDManager
  //! \brief Manages a list of connected Human Interface Devices.
  //! \sa PnPListener
  class HIDManager: public PnPListener
  {
    protected:
      HIDRecordList mRecords; //!< Records container

      //! \brief \b Internal My PnP plug callback.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );

      //! \brief \b Internal My PnP unplug callback.
      //! \param  deviceClass The device class.
      //! \param  devicePath  Full path to the device.
      virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );

      //! \brief Process an existing device.
      //! \param interfaceData Information describing the interface.
      //! \param deviceData    Information describing the device.
      //! \param devicePath    Full pathname of the device file.
      void processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
        SP_DEVINFO_DATA& deviceData, const String& devicePath );

      //! \brief Initialize this HIDManager.
      void initialize();
    public:
      //! \brief Default constructor.
      HIDManager();

      //! \brief Get the list of active HID records.
      //! \return The records.
      const HIDRecordList& getRecords() const;

      //! \brief Destructor.
      ~HIDManager();
  };

  //! @}

}