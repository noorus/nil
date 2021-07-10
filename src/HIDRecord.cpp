#include "nilHID.h"
#include "nilUtil.h"

namespace nil {

  HIDRecord::HIDRecord( const wideString& path, HANDLE handle ):
  mPath( path ), mIsXInput( false ), mIsRDP( false ), mAvailable( false )
  {
    HIDD_ATTRIBUTES attributes = { sizeof( HIDD_ATTRIBUTES ) };

    if ( HidD_GetAttributes( handle, &attributes ) )
    {
      mAvailable = true;
      mVendorID = attributes.VendorID;
      mProductID = attributes.ProductID;

      mIdentifier = MAKELONG( mVendorID, mProductID );

      PHIDP_PREPARSED_DATA preparsedData;
      if ( !HidD_GetPreparsedData( handle, &preparsedData ) )
        NIL_EXCEPT( "HidD_GetPreparsedData failed" );

      if ( HidP_GetCaps( preparsedData, &mCapabilities ) != HIDP_STATUS_SUCCESS )
        NIL_EXCEPT( "HidP_GetCaps failed" );
      HidD_FreePreparsedData( preparsedData );

      wchar_t buffer[256];
      if ( HidD_GetProductString( handle, &buffer, 256 ) )
        mName = util::cleanupName( util::wideToUtf8( buffer ) );
      if ( HidD_GetManufacturerString( handle, &buffer, 256 ) )
        mManufacturer = util::cleanupName( util::wideToUtf8( buffer ) );
      if ( HidD_GetSerialNumberString( handle, &buffer, 256 ) )
        mSerialNumber = util::wideToUtf8( buffer );
    }

    identify();
  }

  void HIDRecord::identify()
  {
    if ( wcsstr( mPath.c_str(), L"ig_" ) || wcsstr( mPath.c_str(), L"IG_" ) )
    {
      mIsXInput = true;
    }
    if ( wcsstr( mPath.c_str(), L"RDP_MOU" ) || wcsstr( mPath.c_str(), L"RDP_KBD" ) )
    {
      mIsRDP = true;
    }
  }

  bool HIDRecord::isAvailable() const
  {
    return mAvailable;
  }

  bool HIDRecord::isXInput() const
  {
    return mIsXInput;
  }

  bool HIDRecord::isRDP() const
  {
    return mIsRDP;
  }

  bool HIDRecord::isMicrosoft() const
  {
    return ( mVendorID == USBVendor_Microsoft );
  }

  bool HIDRecord::isLogitech() const
  {
    return ( mVendorID == USBVendor_Logitech );
  }

  uint32_t HIDRecord::getIdentifier() const
  {
    return mIdentifier;
  }

  uint16_t HIDRecord::getUsagePage() const
  {
    return mCapabilities.UsagePage;
  }

  uint16_t HIDRecord::getUsage() const
  {
    return mCapabilities.Usage;
  }

  const wideString& HIDRecord::getPath() const
  {
    return mPath;
  }

  const utf8String& HIDRecord::getManufacturer() const
  {
    return mManufacturer;
  }

  const utf8String& HIDRecord::getName() const
  {
    return mName;
  }

  const utf8String& HIDRecord::getSerialNumber() const
  {
    return mSerialNumber;
  }

  HIDRecord::~HIDRecord()
  {
  }

}