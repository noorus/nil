#include "nilConfig.h"

#include "nil.h"
#include "nilWindowsPNP.h"
#include "nilUtil.h"
#include <hidclass.h>

#ifdef NIL_PLATFORM_WINDOWS

namespace nil {

  namespace windows {

    HIDManager::HIDManager()
    {
      HidD_GetHidGuid( &g_HIDInterfaceGUID );
      initialize();
    }

    const HIDRecordList& HIDManager::getRecords() const
    {
      return records_;
    }

    void HIDManager::onPnPPlug( const GUID& deviceClass, const wideString& devicePath )
    {
      if ( deviceClass != g_HIDInterfaceGUID )
        return;

      for ( auto& record : records_ )
        if ( util::compareDevicePaths( record->getPath(), devicePath ) )
          return;

      SafeHandle deviceHandle( CreateFileW( devicePath.c_str(), 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr ) );

      if ( deviceHandle.valid() )
      {
        auto record = make_shared<HIDRecord>( devicePath, deviceHandle );
        records_.push_back( record );
      }
    }

    void HIDManager::onPnPUnplug( const GUID& deviceClass, const wideString& devicePath )
    {
      if ( deviceClass != g_HIDInterfaceGUID )
        return;

      for ( auto& record : records_ )
        if ( util::compareDevicePaths( record->getPath(), devicePath ) )
        {
          records_.remove( record );
          return;
        }
    }

    HIDRecordPtr HIDManager::getRecordByPath( const wideString& devicePath )
    {
      for ( auto& record : records_ )
        if ( util::compareDevicePaths( record->getPath(), devicePath ) )
          return record;

      return HIDRecordPtr();
    }

    void HIDManager::processDevice( SP_DEVICE_INTERFACE_DATA& interfaceData,
    SP_DEVINFO_DATA& deviceData, const wideString& devicePath )
    {
      UNREFERENCED_PARAMETER( deviceData );

      if ( interfaceData.InterfaceClassGuid != g_HIDInterfaceGUID )
        return;

      // Well, this string comparison is kind of nasty, but it seems
      // to be what everyone does. Nothing else is quite reliable enough.
      for ( auto& record : records_ )
        if ( util::compareDevicePaths( record->getPath(), devicePath ) )
          return;

      SafeHandle deviceHandle( CreateFileW( devicePath.c_str(), 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr ) );

      if ( deviceHandle.valid() )
      {
        auto record = make_shared<HIDRecord>( devicePath, deviceHandle );
        records_.push_back( record );
      }
    }

    void HIDManager::initialize()
    {
      auto info = SetupDiGetClassDevsW( &g_HIDInterfaceGUID,
        nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

      if ( info == INVALID_HANDLE_VALUE )
        NIL_EXCEPT_WINAPI( "SetupDiGetClassDevsW failed" );

      SP_DEVINFO_DATA deviceData {
        .cbSize = sizeof( SP_DEVINFO_DATA ) };

      for ( unsigned long dev = 0; SetupDiEnumDeviceInfo( info, dev, &deviceData ); ++dev )
      {
        SP_DEVICE_INTERFACE_DATA interfaceData = {
          .cbSize = sizeof( SP_DEVICE_INTERFACE_DATA ) };

        for ( unsigned long ifi = 0; SetupDiEnumDeviceInterfaces( info, &deviceData, &g_HIDInterfaceGUID, ifi, &interfaceData ); ++ifi )
        {
          unsigned long length = 0;
          SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, nullptr, 0, &length, nullptr );

          if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER || !length )
            NIL_EXCEPT_WINAPI( "No buffer size returned from SetupDiGetDeviceInterfaceDetailW" );

          vector<uint8_t> detailBuffer( length, 0 );
          auto detailData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>( detailBuffer.data() );
          detailData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA_W );

          if ( SetupDiGetDeviceInterfaceDetailW( info, &interfaceData, detailData, length, nullptr, &deviceData ) )
            processDevice( interfaceData, deviceData, detailData->DevicePath );
        }
      }

      SetupDiDestroyDeviceInfoList( info );
    }

    HIDManager::~HIDManager()
    {
    }

  }

}

#endif