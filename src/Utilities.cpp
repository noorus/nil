#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  namespace util {

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

    inline void ltrim( utf8String& s )
    {
      s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( unsigned char ch )
      {
        return !std::isspace( ch );
      }));
    }

    inline void rtrim( utf8String& s )
    {
      s.erase( std::find_if( s.rbegin(), s.rend(), []( unsigned char ch )
      {
        return !std::isspace( ch );
      }).base(), s.end() );
    }

    inline void trim( utf8String& s )
    {
      ltrim( s );
      rtrim( s );
    }

    inline utf8String cleanupName( utf8String name ) throw()
    {
      trim( name );
      if ( name.compare( "?" ) != 0 )
        return utf8String();
      return name;
    }

    utf8String generateName( Device::Type deviceType, int index ) throw()
    {
      char name[64] = { 0 };
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

#ifdef NIL_PLATFORM_WINDOWS

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

#endif

  }

}