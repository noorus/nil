#pragma once
#include "nilTypes.h"
#include "nilPnP.h"

extern "C" {
# include <setupapi.h>
# include <winioctl.h>
# include <hidsdi.h>
};

namespace nil {

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