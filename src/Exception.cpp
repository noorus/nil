#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Exception::Exception()
  {
    // Stub
  }

  Exception::Exception( const wstring& description, Type type ):
  mDescription( description ), mType( type )
  {
    // Stub
  }

  Exception::Exception( const wstring& description, const wstring& source, Type type ):
  mDescription( description ), mSource( source ), mType( type )
  {
    // Stub
  }

  void Exception::handleAdditional()
  {
    WinAPIError error;

    if ( mType == WinAPI )
      error.code = GetLastError();

    if ( mType == WinAPI )
    {
      LPWSTR message = nullptr;
      FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error.code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPWSTR)&message, 0, NULL );
      error.description = message;
      LocalFree( message );
      mAdditional = error;
    }
  }

  const wstring& Exception::getFullDescription() const
  {
    if ( mFullDescription.empty() )
    {
      wstringstream stream;

      stream << mDescription;

      if ( !mSource.empty() )
        stream << L"\r\nIn function " << mSource;

      if ( mType == WinAPI )
      {
        const WinAPIError& error = boost::get<WinAPIError>( mAdditional );
        stream << L"\r\nWinAPI error code " << std::hex << error.code << L":\r\n" << error.description;
      }

      mFullDescription = stream.str();
    }
    return mFullDescription;
  }

  const char* Exception::what() const
  {
    mUTF8Description = util::wideToUtf8( getFullDescription() );
    return mUTF8Description.c_str();
  }

}