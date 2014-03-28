#include "nilHID.h"
#include "nilUtil.h"

namespace Nil {

  HIDRecord::HIDRecord( const String& path, HANDLE handle ):
  mPath( path ), mIsXInput( false ), mIsRDP( false )
  {
    HIDD_ATTRIBUTES attributes = { sizeof( HIDD_ATTRIBUTES ) };

    if ( !HidD_GetAttributes( handle, &attributes ) )
      NIL_EXCEPT( L"HidD_GetAttributes failed" );

    mVendorID = attributes.VendorID;
    mProductID = attributes.ProductID;

    mIdentifier = MAKELONG( mVendorID, mProductID );

    PHIDP_PREPARSED_DATA preparsedData;
    if ( !HidD_GetPreparsedData( handle, &preparsedData ) )
      NIL_EXCEPT( L"HIdD_GetPreparsedData failed" );

    HidP_GetCaps( preparsedData, &mCapabilities );
    HidD_FreePreparsedData( preparsedData );

    wchar_t buffer[256];
    if ( HidD_GetProductString( handle, &buffer, 256 ) )
      mName = Util::cleanupName( buffer );
    if ( HidD_GetManufacturerString( handle, &buffer, 256 ) )
      mManufacturer = Util::cleanupName( buffer );
    if ( HidD_GetSerialNumberString( handle, &buffer, 256 ) )
      mSerialNumber = buffer;

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

  const String& HIDRecord::getPath() const
  {
    return mPath;
  }

  const String& HIDRecord::getManufacturer() const
  {
    return mManufacturer;
  }

  const String& HIDRecord::getName() const
  {
    return mName;
  }

  const String& HIDRecord::getSerialNumber() const
  {
    return mSerialNumber;
  }

  HIDRecord::~HIDRecord()
  {
  }

}