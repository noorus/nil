#include "nilHID.h"
#include "nilUtil.h"

namespace nil {

  HIDRecord::HIDRecord( const String& path ): mPath( path ),
    mHandle( INVALID_HANDLE_VALUE ), mIsXInput( false )
  {
    mHandle = CreateFileW( path.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

    if ( mHandle == INVALID_HANDLE_VALUE )
      NIL_EXCEPT_WINAPI( L"CreateFileW failed" );

    HIDD_ATTRIBUTES attributes = { sizeof( HIDD_ATTRIBUTES ) };

    if ( !HidD_GetAttributes( mHandle, &attributes ) )
      NIL_EXCEPT( L"HidD_GetAttributes failed" );

    mVendorID = attributes.VendorID;
    mProductID = attributes.ProductID;

    mIdentifier = MAKELONG( mVendorID, mProductID );

    PHIDP_PREPARSED_DATA preparsedData;
    if ( !HidD_GetPreparsedData( mHandle, &preparsedData ) )
      NIL_EXCEPT( L"HIdD_GetPreparsedData failed" );

    HidP_GetCaps( preparsedData, &mCapabilities );
    HidD_FreePreparsedData( preparsedData );

    wchar_t buffer[256];
    if ( HidD_GetProductString( mHandle, &buffer, 256 ) )
      mName = util::cleanupName( buffer );
    if ( HidD_GetManufacturerString( mHandle, &buffer, 256 ) )
      mManufacturer = util::cleanupName( buffer );
    if ( HidD_GetSerialNumberString( mHandle, &buffer, 256 ) )
      mSerialNumber = buffer;

    SAFE_CLOSEHANDLE( mHandle );

    identify();
  }

  void HIDRecord::identify()
  {
    if ( wcsstr( mPath.c_str(), L"ig_" ) || wcsstr( mPath.c_str(), L"IG_" ) )
    {
      mIsXInput = true;
    }
  }

  bool HIDRecord::isXInput() const
  {
    return mIsXInput;
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
    SAFE_CLOSEHANDLE( mHandle );
  }

}