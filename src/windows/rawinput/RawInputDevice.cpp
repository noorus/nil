#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"
#include "nilWindows.h"

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  // RawInputDeviceInfo class

  RawInputDeviceInfo::RawInputDeviceInfo( HANDLE handle )
  {
    UINT size = 0;
    if ( GetRawInputDeviceInfoW( handle, RIDI_DEVICEINFO, nullptr, &size ) != 0 )
      NIL_EXCEPT_WINAPI( "GetRawInputDeviceInfoW failed" );

    rawDeviceInfo_.resize( size, 0 );

    if ( !GetRawInputDeviceInfoW( handle, RIDI_DEVICEINFO, rawDeviceInfo_.data(), &size ) )
      NIL_EXCEPT_WINAPI( "GetRawInputDeviceInfoW failed" );
  }

  const RID_DEVICE_INFO* RawInputDeviceInfo::getRawInfo() const
  {
    return reinterpret_cast<const RID_DEVICE_INFO*>( rawDeviceInfo_.data() );
  }

  Device::Type RawInputDeviceInfo::rawInfoResolveType() const
  {
    switch ( getRawInfo()->dwType )
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
  }

  // RawInputDevice class

  RawInputDevice::RawInputDevice( SystemPtr system, DeviceID id, HANDLE rawHandle, wideString& rawPath,
  windows::HIDRecordPtr hid ): RawInputDeviceInfo( rawHandle ), Device( system, id, rawInfoResolveType() ),
  rawHandle_( rawHandle ), rawPath_( rawPath ), hidRecord_( hid )
  {
    SafeHandle deviceHandle( CreateFileW( rawPath_.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr ) );

    if ( deviceHandle.valid() )
    {
      // The HidD API documentation states the max. length is 256 characters
      wchar_t buffer[256];

      if ( HidD_GetProductString( deviceHandle, &buffer, 256 ) )
      {
        buffer[255] = '\0';
        // Only replace auto-generated name if fetched one isn't empty
        utf8String tmpName = util::cleanupName( util::wideToUtf8( buffer ) );
        if ( !tmpName.empty() )
          name_ = tmpName;
      }
    }

    if ( hid )
      name_ = hid->makePrettyName( name_, typedIndex_ );
  }

  RawInputDevice::~RawInputDevice()
  {
  }

  Device::Handler RawInputDevice::getHandler() const
  {
    return Device::Handler_RawInput;
  }

  DeviceID RawInputDevice::getStaticID() const
  {
    // Static ID for RawInput devices:
    // 4 bits of handler ID, 28 bits of unique id (hashed raw path)

    DeviceID id = util::fnv_32a_buf(
      (void*)rawPath_.c_str(),
      rawPath_.length() * sizeof( wchar_t ),
      FNV1_32A_INIT );

    return ( ( id >> 4 ) | ( ( Handler_RawInput + 1 ) << 28 ) );
  }

  HANDLE RawInputDevice::getRawHandle() const
  {
    return rawHandle_;
  }

  const wideString& RawInputDevice::getRawPath() const
  {
    return rawPath_;
  }

  windows::HIDRecordPtr RawInputDevice::getHIDReccord() const
  {
    return hidRecord_;
  }

}

#endif