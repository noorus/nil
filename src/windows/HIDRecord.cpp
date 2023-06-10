#include "nilConfig.h"

#include "nilWindowsPNP.h"
#include "nilUtil.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  namespace windows {

    // clang-format off

    const vector<KnownDeviceRecord> c_predefinedControllers = {
      { USBVendor_Sony, 0x05C4, KnownDevice_DualShock4, "DualShock 4", 18 },
      { USBVendor_Sony, 0x09CC, KnownDevice_DualShock4, "DualShock 4", 18 },
      { USBVendor_Sony, 0x0CE6, KnownDevice_DualSense, "DualSense", 9 }
    };

    const map<uint16_t, utf8String> c_vendorNameMap = {
      { USBVendor_Microsoft, "Microsoft" },
      { USBVendor_Logitech, "Logitech" },
      { USBVendor_Sony, "Sony" },
      { USBVendor_Razer, "Razer" },
      { USBVendor_Nacon, "Nacon" },
      { USBVendor_Kingston, "HyperX" },
      { USBVendor_Corsair, "Corsair" },
      { USBVendor_Apple, "Apple" },
      { USBVendor_Alienware, "Alienware" },
      { USBVendor_Metadot, "Das" },
      { USBVendor_Ducky, "Ducky" },
      { USBVendor_Roccat, "Roccat" },
      { USBVendor_SteelSeries, "SteelSeries" },
    };

    const map<HIDConnectionType, utf8String> c_hidConnectionTypeNameMap = {
      { HIDConnection_Unknown, "Unknown" },
      { HIDConnection_USB, "USB" },
      { HIDConnection_Bluetooth, "Bluetooth" }
    };

    // clang-format on

    inline const KnownDeviceRecord* resolveKnownDevice( uint16_t vid, uint16_t pid )
    {
      for ( const auto& predef : c_predefinedControllers )
      {
        if ( vid == predef.vid && pid == predef.pid )
          return &predef;
      }

      return nullptr;
    }

    template <class T>
    unsigned long bufSizeBytes( const vector<T>& container )
    {
      return static_cast<unsigned long>( container.size() * sizeof( T ) );
    }

    thread_local static vector<wchar_t> s_wideBuffer( 256, L'\0' );

    HIDRecord::HIDRecord( const wideString& path, HANDLE handle ):
    path_( path )
    {
      HIDD_ATTRIBUTES attributes = { .Size = sizeof( HIDD_ATTRIBUTES ) };

      if ( HidD_GetAttributes( handle, &attributes ) )
      {
        available_ = true;
        usbVid_ = attributes.VendorID;
        usbPid_ = attributes.ProductID;

        hidIdent_ = MAKELONG( usbVid_, usbPid_ );

        PHIDP_PREPARSED_DATA preparsedData;
        if ( !HidD_GetPreparsedData( handle, &preparsedData ) )
          NIL_EXCEPT( "HidD_GetPreparsedData failed" );

        if ( HidP_GetCaps( preparsedData, &caps_ ) != HIDP_STATUS_SUCCESS )
          NIL_EXCEPT( "HidP_GetCaps failed" );

        HidD_FreePreparsedData( preparsedData );

        if ( HidD_GetProductString( handle, s_wideBuffer.data(), bufSizeBytes( s_wideBuffer ) ) )
          name_ = util::cleanupName( util::wideToUtf8( s_wideBuffer.data() ) );

        if ( HidD_GetManufacturerString( handle, s_wideBuffer.data(), bufSizeBytes( s_wideBuffer ) ) )
          manufacturer_ = util::cleanupName( util::wideToUtf8( s_wideBuffer.data() ) );

        if ( HidD_GetSerialNumberString( handle, s_wideBuffer.data(), bufSizeBytes( s_wideBuffer ) ) )
          serial_ = util::wideToUtf8( s_wideBuffer.data() );

        knownDevice_ = resolveKnownDevice( usbVid_, usbPid_ );

        if ( knownDevice_ )
        {
          manufacturer_ = c_vendorNameMap.at( knownDevice_->vid );
          name_ = knownDevice_->name;
          if ( serial_.empty() && knownDevice_->reportID_Serial && connectionType() == HIDConnection_USB )
          {
            vector<uint8_t> buffer( caps_.FeatureReportByteLength, 0 );
            buffer[0] = knownDevice_->reportID_Serial;
            if ( HidD_GetFeature( handle, buffer.data(), static_cast<unsigned long>( buffer.size() ) ) )
            {
              swprintf_s( s_wideBuffer.data(), s_wideBuffer.size(), L"%02X%02X%02X%02X%02X%02X",
                buffer[6], buffer[5], buffer[4], buffer[3], buffer[2], buffer[1] );
              serial_ = util::wideToUtf8( s_wideBuffer.data() );
            }
          }
        }
        else
        {
          if ( manufacturer_.empty() && c_vendorNameMap.find( usbVid_ ) != c_vendorNameMap.end() )
            manufacturer_ = c_vendorNameMap.at( usbVid_ );
        }

        // wprintf_s( L"HIDRecord: vid 0x%04X pid 0x%04X %S %S serial %S (%S)\r\n", usbVid_, usbPid_, manufacturer_.c_str(), name_.c_str(), serial_.c_str(), c_hidConnectionTypeNameMap.at( connectionType() ).c_str() );
      }

      identify();
    }

    void HIDRecord::identify()
    {
      if ( wcsstr( path_.c_str(), L"ig_" ) || wcsstr( path_.c_str(), L"IG_" ) )
      {
        isXInput_ = true;
      }
      if ( wcsstr( path_.c_str(), L"RDP_MOU" ) || wcsstr( path_.c_str(), L"RDP_KBD" ) )
      {
        isRDP_ = true;
      }
    }

    HIDConnectionType HIDRecord::connectionType() const
    {
      if ( caps_.InputReportByteLength == 64 )
        return HIDConnection_USB;
      if ( caps_.InputReportByteLength == 78 )
        return HIDConnection_Bluetooth;
      return HIDConnection_Unknown;
    }

    KnownDeviceType HIDRecord::knownDeviceType() const
    {
      if ( !knownDevice_ )
        return KnownDevice_Unknown;
      return knownDevice_->type;
    }

    utf8String HIDRecord::makePrettyName( const utf8String& raw, int typedIndex ) const
    {
      vector<utf8String> parts;

      if ( !manufacturer_.empty() )
        parts.push_back( manufacturer_ );

      parts.push_back( name_.empty() ? raw : name_ );

      if ( knownDevice_ )
      {
        parts.push_back( std::to_string( typedIndex ) );
        parts.push_back( c_hidConnectionTypeNameMap.at( connectionType() ) );
      }

      return util::stringJoin( parts, utf8String( " " ) );
    }

    bool HIDRecord::isAvailable() const
    {
      return available_;
    }

    bool HIDRecord::isXInput() const
    {
      return isXInput_;
    }

    bool HIDRecord::isRDP() const
    {
      return isRDP_;
    }

    uint32_t HIDRecord::getIdentifier() const
    {
      return hidIdent_;
    }

    uint16_t HIDRecord::getUsagePage() const
    {
      return caps_.UsagePage;
    }

    uint16_t HIDRecord::getUsage() const
    {
      return caps_.Usage;
    }

    const wideString& HIDRecord::getPath() const
    {
      return path_;
    }

    const utf8String& HIDRecord::getManufacturer() const
    {
      return manufacturer_;
    }

    const utf8String& HIDRecord::getName() const
    {
      return name_;
    }

    const utf8String& HIDRecord::getSerialNumber() const
    {
      return serial_;
    }

  }

}

#endif