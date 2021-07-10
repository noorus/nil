#include "nilHID.h"
#include "nilUtil.h"

namespace nil {

  HIDManager::HIDManager()
  {
    initialize();
  }

  const HIDRecordList& HIDManager::getRecords() const
  {
    return mRecords;
  }

  void HIDManager::onPnPPlug( const GUID& deviceClass, const wideString& devicePath )
  {
    if ( deviceClass != g_HIDInterfaceGUID )
      return;

    for ( auto record : mRecords )
      if ( !_wcsicmp( record->getPath().c_str(), devicePath.c_str() ) )
        return;

    SafeHandle deviceHandle( CreateFileW( devicePath.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) );

    if ( deviceHandle.valid() )
    {
      auto record = new HIDRecord( devicePath, deviceHandle );
      mRecords.push_back( record );
    }
  }

  void HIDManager::onPnPUnplug( const GUID& deviceClass, const wideString& devicePath )
  {
    if ( deviceClass != g_HIDInterfaceGUID )
      return;

    for ( auto record : mRecords )
      if ( !_wcsicmp( record->getPath().c_str(), devicePath.c_str() ) )
      {
        mRecords.remove( record );
        delete record;
        return;
      }
  }

  void HIDManager::processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
  SP_DEVINFO_DATA& deviceData, const wideString& devicePath )
  {
    if ( interfaceData.InterfaceClassGuid != g_HIDInterfaceGUID )
      return;

    // Well, this string comparison is kind of nasty, but it seems
    // to be what everyone does. Nothing else is quite reliable enough.
    for ( auto record : mRecords )
      if ( !_wcsicmp( record->getPath().c_str(), devicePath.c_str() ) )
        return;

    SafeHandle deviceHandle( CreateFileW( devicePath.c_str(), 0,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL ) );

    if ( deviceHandle.valid() )
    {
      auto record = new HIDRecord( devicePath, deviceHandle );
      mRecords.push_back( record );
    }
  }

  void HIDManager::initialize()
  {
    SP_DEVICE_INTERFACE_DATA interfaceData = { sizeof( SP_DEVICE_INTERFACE_DATA ) };

    auto info = SetupDiGetClassDevsW( &g_HIDInterfaceGUID,
      nullptr, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

    if ( info == INVALID_HANDLE_VALUE )
      NIL_EXCEPT_WINAPI( "SetupDiGetClassDevsW failed" );

    for ( unsigned long i = 0; SetupDiEnumDeviceInterfaces(
    info, 0, &g_HIDInterfaceGUID, i, &interfaceData ); i++ )
    {
      unsigned long length = 0;
      SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, nullptr, 0, &length, nullptr );

      if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        NIL_EXCEPT_WINAPI( "SetupDiGetDeviceInterfaceDetailW failed" );

      auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)malloc( (size_t)length );
      if ( !detailData )
        NIL_EXCEPT( "Memory allocation failed" );
      detailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA_W );

      SP_DEVINFO_DATA deviceData = { sizeof( SP_DEVINFO_DATA ) };

      if ( SetupDiGetDeviceInterfaceDetailW( info, &interfaceData,
      detailData, length, nullptr, &deviceData ) )
      {
        processDevice( interfaceData, deviceData, detailData->DevicePath );
      }

      free( detailData );
    }

    SetupDiDestroyDeviceInfoList( info );
  }

  HIDManager::~HIDManager()
  {
    for ( auto record : mRecords )
      delete record;
  }

}