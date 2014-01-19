#include "nil.h"
#include "nilUtil.h"
#include <boost/algorithm/string/trim_all.hpp>

namespace nil {

  namespace util {

    inline String cleanupName( String name ) throw()
    {
      if ( boost::iequals( name, L"?" ) )
        return String();
      boost::algorithm::trim_all( name );
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