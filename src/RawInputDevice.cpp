#include "nil.h"
#include "nilUtil.h"

namespace nil {

  RawInputDevice::RawInputDevice( System* system, DeviceID id,
  HANDLE rawHandle, String& rawPath ):
  Device( system, id, Device_Mouse ),
  mRawHandle( rawHandle ), mRawPath( rawPath ), mRawInfo( nullptr )
  {
    UINT size = 0;
    if ( GetRawInputDeviceInfoW( mRawHandle, RIDI_DEVICEINFO, NULL, &size ) != 0 )
      NIL_EXCEPT_WINAPI( L"GetRawInputDeviceInfoW failed" );

    mRawInfo = (RID_DEVICE_INFO*)malloc( size );
    if ( !mRawInfo )
      NIL_EXCEPT( L"Memory allocation failed" );

    GetRawInputDeviceInfoW( mRawHandle, RIDI_DEVICEINFO, mRawInfo, &size );

    switch ( mRawInfo->dwType )
    {
      case RIM_TYPEMOUSE:
        mType = Device_Mouse;
      break;
      case RIM_TYPEKEYBOARD:
        mType = Device_Keyboard;
      break;
    }

    // Auto-generate a name an type-specific index
    initAfterTyped();

    auto handle = CreateFileW( mRawPath.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

    if ( handle != INVALID_HANDLE_VALUE )
    {
      wchar_t buffer[256];
      if ( HidD_GetProductString( handle, &buffer, 256 ) )
      {
        // Only replace auto-generated name if fetched one isn't empty
        String tmpName = util::cleanupName( buffer );
        if ( !tmpName.empty() )
          mName = tmpName;
      }
      CloseHandle( handle );
    }
  }

  RawInputDevice::~RawInputDevice()
  {
    if ( mRawInfo )
      free( mRawInfo );
  }

  const Device::Handler RawInputDevice::getHandler() const
  {
    return Device::Handler_RawInput;
  }

  const HANDLE RawInputDevice::getRawHandle() const
  {
    return mRawHandle;
  }

  const String& RawInputDevice::getRawPath() const
  {
    return mRawPath;
  }

  const RID_DEVICE_INFO* RawInputDevice::getRawInfo() const
  {
    return mRawInfo;
  }

}