#include "nil.h"
#include "nilUtil.h"

#pragma comment( lib, "hid.lib" )
#pragma comment( lib, "setupapi.lib" )

namespace nil {

  // HIDDevice class

  HIDDevice::HIDDevice( const String& path ): mPath( path ), mIsXInput( false )
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
    if ( HidD_GetPreparsedData( mHandle, &preparsedData ) )
    {
      HidP_GetCaps( preparsedData, &mCapabilities );
      HidD_FreePreparsedData( preparsedData );
    }

    CloseHandle( mHandle );
    mHandle = INVALID_HANDLE_VALUE;

    identify();
  }

  void HIDDevice::identify()
  {
    if ( wcsstr( mPath.c_str(), L"ig_" ) || wcsstr( mPath.c_str(), L"IG_" ) )
    {
      mIsXInput = true;
    }
  }

  bool HIDDevice::isXInput() const
  {
    return mIsXInput;
  }

  uint32_t HIDDevice::getIdentifier() const
  {
    return mIdentifier;
  }

  const String& HIDDevice::getPath() const
  {
    return mPath;
  }

  HIDDevice::~HIDDevice()
  {
    //
  }

  // HIDManager class

  HIDManager::HIDManager()
  {
    initialize();
  }

  const HIDDeviceList& HIDManager::getDevices() const
  {
    return mDevices;
  }

  void HIDManager::onPlug( const GUID& deviceClass, const String& devicePath )
  {
    if ( deviceClass != g_HIDInterfaceGUID )
      return;

    for ( auto device : mDevices )
      if ( !_wcsicmp( device->getPath().c_str(), devicePath.c_str() ) )
        return;

    try {
      auto device = new HIDDevice( devicePath );
      mDevices.push_back( device );
    } catch ( nil::Exception& e ) {
      // Ignore inaccessible devices
    }
  }

  void HIDManager::onUnplug( const GUID& deviceClass, const String& devicePath )
  {
    if ( deviceClass != g_HIDInterfaceGUID )
      return;

    for ( auto device : mDevices )
      if ( !_wcsicmp( device->getPath().c_str(), devicePath.c_str() ) )
      {
        mDevices.remove( device );
        delete device;
        return;
      }
  }

  void HIDManager::processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
  SP_DEVINFO_DATA& deviceData, const String& devicePath )
  {
    if ( interfaceData.InterfaceClassGuid != g_HIDInterfaceGUID )
      return;

    for ( auto device : mDevices )
      if ( !_wcsicmp( device->getPath().c_str(), devicePath.c_str() ) )
        return;

    try {
      auto device = new HIDDevice( devicePath );
      mDevices.push_back( device );
    } catch ( nil::Exception& e ) {
      // Ignore inaccessible devices
    }
  }

  void HIDManager::initialize()
  {
    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    auto info = SetupDiGetClassDevsW( &g_HIDInterfaceGUID,
      NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( info == INVALID_HANDLE_VALUE )
      NIL_EXCEPT_WINAPI( L"SetupDiGetClassDevsW failed" );

    for ( unsigned long i = 0; SetupDiEnumDeviceInterfaces(
    info, 0, &g_HIDInterfaceGUID, i, &interfaceData ); i++ )
    {
      unsigned long length = 0;
      SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, NULL, 0, &length, NULL );

      if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        NIL_EXCEPT_WINAPI( L"SetupDiGetDeviceInterfaceDetailW failed" );

      auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc( length );
      detailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA_W );

      SP_DEVINFO_DATA deviceData = { sizeof( SP_DEVINFO_DATA ) };

      if ( SetupDiGetDeviceInterfaceDetailW( info, &interfaceData,
      detailData, length, NULL, &deviceData ) )
      {
        processDevice( interfaceData, deviceData, detailData->DevicePath );
      }

      free( detailData );
    }

    SetupDiDestroyDeviceInfoList( info );
  }

  HIDManager::~HIDManager()
  {
    for ( auto device : mDevices )
      delete device;
  }

}