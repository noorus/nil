#include "nil.h"
#include "nilUtil.h"
#include <boost/algorithm/string/trim_all.hpp>

namespace Nil {

  namespace Util {

    uint32_t fnv_32a_buf( void* buf, size_t len, uint32_t hashval )
    {
      unsigned char* bp = (unsigned char*)buf;
      unsigned char* be = bp + len;
      while ( bp < be )
      {
        hashval ^= (uint32_t)*bp++;
        hashval *= 0x01000193;
      }
      return hashval;
    }

    inline String cleanupName( String name ) throw()
    {
      boost::algorithm::trim_all( name );
      if ( boost::iequals( name, L"?" ) )
        return String();
      return name;
    }

    String generateName( Device::Type deviceType, int index ) throw()
    {
      wchar_t name[64];
      switch ( deviceType )
      {
        case Device::Device_Mouse:
          swprintf_s( name, 64, L"Mouse %d", index );
        break;
        case Device::Device_Keyboard:
          swprintf_s( name, 64, L"Keyboard %d", index );
        break;
        case Device::Device_Controller:
          swprintf_s( name, 64, L"Controller %d", index );
        break;
      }
      return name;
    }

    inline String utf8ToWide( const utf8String& in ) throw()
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return String();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return String( &conversion[0] );
    }

    inline utf8String wideToUtf8( const String& in ) throw()
    {
      int length = WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, nullptr, 0, 0, FALSE );
      if ( length == 0 )
        return utf8String();
      vector<char> conversion( length );
      WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length, 0, FALSE );
      return utf8String( &conversion[0] );
    }

  }

}