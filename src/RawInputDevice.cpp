#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

namespace nil {

  // RawInputDeviceInfo class

  RawInputDeviceInfo::RawInputDeviceInfo( HANDLE handle ): mRawInfo( nullptr )
  {
    UINT size = 0;
    if ( GetRawInputDeviceInfoW( handle, RIDI_DEVICEINFO, NULL, &size ) != 0 )
      NIL_EXCEPT_WINAPI( "GetRawInputDeviceInfoW failed" );

    mRawInfo = (RID_DEVICE_INFO*)malloc( (size_t)size );
    if ( !mRawInfo )
      NIL_EXCEPT( "Memory allocation failed" );

    if ( !GetRawInputDeviceInfoW( handle, RIDI_DEVICEINFO, mRawInfo, &size ) )
      NIL_EXCEPT_WINAPI( "GetRawInputDeviceInfoW failed" );
  }

  const Device::Type RawInputDeviceInfo::rawInfoResolveType() const
  {
    switch ( mRawInfo->dwType )
    {
      case RIM_TYPEMOUSE:
        return Device::Device_Mouse;
      break;
      case RIM_TYPEKEYBOARD:
        return Device::Device_Keyboard;
      break;
      default:
        return Device::Device_Controller;
      break;
    }
  }

  RawInputDeviceInfo::~RawInputDeviceInfo()
  {
    free( mRawInfo );
  }

  // RawInputDevice class

  RawInputDevice::RawInputDevice( System* system, DeviceID id,
  HANDLE rawHandle, wideString& rawPath ):
  RawInputDeviceInfo( rawHandle ),
  Device( system, id, rawInfoResolveType() ),
  mRawHandle( rawHandle ), mRawPath( rawPath )
  {
    SafeHandle deviceHandle( CreateFileW( mRawPath.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) );

    if ( deviceHandle.valid() )
    {
      // The HidD API documentation states the max. length is 256 characters
      wchar_t buffer[256];

      if ( HidD_GetProductString( deviceHandle, &buffer, 256 ) )
      {
        // Only replace auto-generated name if fetched one isn't empty
        utf8String tmpName = util::cleanupName( util::wideToUtf8( buffer ) );
        if ( !tmpName.empty() )
          mName = tmpName;
      }
    }
  }

  RawInputDevice::~RawInputDevice()
  {
  }

  const Device::Handler RawInputDevice::getHandler() const
  {
    return Device::Handler_RawInput;
  }

  const DeviceID RawInputDevice::getStaticID() const
  {
    // Static ID for RawInput devices:
    // 4 bits of handler ID, 28 bits of unique id (hashed raw path)
 
    DeviceID id = util::fnv_32a_buf(
      (void*)mRawPath.c_str(),
      mRawPath.length() * sizeof( wchar_t ),
      FNV1_32A_INIT );

    return ( ( id >> 4 ) | ( ( Handler_RawInput + 1 ) << 28 ) );
  }

  const HANDLE RawInputDevice::getRawHandle() const
  {
    return mRawHandle;
  }

  const wideString& RawInputDevice::getRawPath() const
  {
    return mRawPath;
  }

  const RID_DEVICE_INFO* RawInputDevice::getRawInfo() const
  {
    return mRawInfo;
  }

}