#pragma once
#include "nilTypes.h"
#include "nilPnP.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace nil {

  //! Known USB vendor IDs that might be important.
  enum USBKnownVendor: uint16_t {
    USBVendor_Microsoft = 0x045E,
    USBVendor_Logitech  = 0x046D
  };

  //! \class HIDRecord
  //! A Human Interface Device instance currently present in the system.
  class HIDRecord {
  protected:
    String mPath;
    uint16_t mVendorID;
    uint16_t mProductID;
    uint32_t mIdentifier;
    HIDP_CAPS mCapabilities;
    String mName;
    String mManufacturer;
    String mSerialNumber;
    bool mIsXInput;
    void identify();
  public:
    HIDRecord( const String& path, HANDLE handle );
    bool isXInput() const;
    bool isMicrosoft() const;
    bool isLogitech() const;
    const String& getPath() const;
    const String& getName() const;
    const String& getManufacturer() const;
    const String& getSerialNumber() const;
    uint16_t getUsagePage() const;
    uint16_t getUsage() const;
    uint32_t getIdentifier() const;
    ~HIDRecord();
  };

  typedef list<HIDRecord*> HIDRecordList;

  //! \class HIDManager
  //! Manages a list of connected Human Interface Devices.
  class HIDManager: public PnPListener {
  protected:
    HIDRecordList mRecords;
    virtual void onPnPPlug( const GUID& deviceClass, const String& devicePath );
    virtual void onPnPUnplug( const GUID& deviceClass, const String& devicePath );
    void processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
      SP_DEVINFO_DATA& deviceData, const String& devicePath );
    void initialize();
  public:
    HIDManager();
    const HIDRecordList& getRecords() const;
    ~HIDManager();
  };

}