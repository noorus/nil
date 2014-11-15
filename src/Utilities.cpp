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

    inline utf8String cleanupName( utf8String name ) throw()
    {
      boost::algorithm::trim_all( name );
      if ( boost::iequals( name, "?" ) )
        return utf8String();
      return name;
    }

    utf8String generateName( Device::Type deviceType, int index ) throw()
    {
      char name[64];
      switch ( deviceType )
      {
        case Device::Device_Mouse:
          sprintf_s( name, 64, "Mouse %d", index );
        break;
        case Device::Device_Keyboard:
          sprintf_s( name, 64, "Keyboard %d", index );
        break;
        case Device::Device_Controller:
          sprintf_s( name, 64, "Controller %d", index );
        break;
      }
      return name;
    }

    inline wideString utf8ToWide( const utf8String& in ) throw()
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return wideString();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return wideString( &conversion[0] );
    }

    inline utf8String wideToUtf8( const wideString& in ) throw()
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