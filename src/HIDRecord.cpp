#include "nilHID.h"
#include "nilUtil.h"

namespace nil {

  HIDRecord::HIDRecord( const wideString& path, HANDLE handle ):
  path_( path ), isXInput_( false ), isRDP_( false ), available_( false )
  {
    HIDD_ATTRIBUTES attributes = { sizeof( HIDD_ATTRIBUTES ) };

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

      wchar_t buffer[256] = { 0 };
      if ( HidD_GetProductString( handle, &buffer, 256 ) )
        name_ = util::cleanupName( util::wideToUtf8( buffer ) );
      if ( HidD_GetManufacturerString( handle, &buffer, 256 ) )
        manufacturer_ = util::cleanupName( util::wideToUtf8( buffer ) );
      if ( HidD_GetSerialNumberString( handle, &buffer, 256 ) )
        serial_ = util::wideToUtf8( buffer );
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

  bool HIDRecord::isMicrosoft() const
  {
    return ( usbVid_ == USBVendor_Microsoft );
  }

  bool HIDRecord::isLogitech() const
  {
    return ( usbVid_ == USBVendor_Logitech );
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

  HIDRecord::~HIDRecord()
  {
  }

}