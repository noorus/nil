#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  struct DirectInputErrorEntry {
  public:
    long code;
    wchar_t* description;
  };

  const int cErrorDescriptionCount = 32; //-V112

  const DirectInputErrorEntry cErrorDescriptions[cErrorDescriptionCount] =
  {
    { DIERR_ACQUIRED, L"The operation cannot be performed while the device is acquired." },
    { DIERR_ALREADYINITIALIZED, L"The object is already initialized." },
    { DIERR_BADDRIVERVER, L"The object could not be created due to an incompatible driver." },
    { DIERR_DEVICEFULL, L"The device is full." },
    { DIERR_DEVICENOTREG, L"The device or device instance is not registered with DirectInput." },
    { DIERR_EFFECTPLAYING, L"The device does not support updating an effect while it is still playing." },
    { DIERR_GENERIC, L"An undetermined error occurred inside the DirectInput subsystem." },
    { DIERR_HANDLEEXISTS, L"The device already has an event notification associated with it." },
    { DIERR_HASEFFECTS, L"The device cannot be reinitialized because effects are attached to it." },
    { DIERR_INCOMPLETEEFFECT, L"The effect could not be downloaded because essential information is missing." },
    { DIERR_INPUTLOST, L"Access to the input device has been lost. It must be reacquired." },
    { DIERR_INVALIDPARAM, L"An invalid parameter was passed to the function." },
    { DIERR_MAPFILEFAIL, L"An error occurred reading or writing the action-mapping file for the device." },
    { DIERR_MOREDATA, L"Not all the requested information fit into the buffer." },
    { DIERR_NOAGGREGATION, L"The object does not support aggregation." },
    { DIERR_NOINTERFACE, L"The object does not support the specified interface." },
    { DIERR_NOTACQUIRED, L"The operation cannot be performed unless the device is acquired." },
    { DIERR_NOTBUFFERED, L"The device is not buffered." },
    { DIERR_NOTDOWNLOADED, L"The effect was not downloaded." },
    { DIERR_NOTEXCLUSIVEACQUIRED, L"The operation cannot be performed unless the device is acquired in exclusive mode." },
    { DIERR_NOTFOUND, L"The requested object does not exist." },
    { DIERR_NOTINITIALIZED, L"The object has not been initialized." },
    { DIERR_OLDDIRECTINPUTVERSION, L"The application requires a newer version of DirectInput." },
    { DIERR_OTHERAPPHASPRIO, L"Another application has a higher priority level." },
    { DIERR_OUTOFMEMORY, L"The DirectInput subsystem is out of memory." },
    { DIERR_READONLY, L"The specified property is read-only." },
    { DIERR_REPORTFULL, L"More information was requested than can be sent to the device." },
    { DIERR_UNPLUGGED, L"The device is not plugged in." },
    { DIERR_UNSUPPORTED, L"The function called is not supported at this time." },
    { E_HANDLE, L"The HWND parameter is not a valid top-level window that belongs to the process." },
    { E_PENDING, L"Data is not yet available." },
    { E_POINTER, L"An invalid pointer was passed as a parameter." }
  };

  Exception::Exception()
  {
    // Stub
  }

  Exception::Exception( const String& description, Type type ):
  mDescription( description ), mType( type )
  {
    handleAdditional();
  }

  Exception::Exception( const String& description, const String& source, Type type ):
  mDescription( description ), mSource( source ), mType( type )
  {
    handleAdditional();
  }

  Exception::Exception( const String& description, const String& source, HRESULT hr, Type type ):
  mDescription( description ), mSource( source ), mType( type )
  {
    handleAdditional( hr );
  }

  void Exception::handleAdditional( HRESULT hr )
  {
    WinAPIError error;
    if ( mType == WinAPI )
    {
      error.code = GetLastError();

      LPWSTR message = nullptr;
      FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error.code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPWSTR)&message, 0, NULL );

      if ( message )
      {
        error.description = message;
        LocalFree( message );
      }

      mAdditional = error;
    }
    else if ( mType == DirectInput )
    {
      error.code = hr;
      for ( unsigned long i = 0; i < cErrorDescriptionCount; i++ )
      {
        if ( cErrorDescriptions[i].code == error.code )
        {
          error.description = cErrorDescriptions[i].description;
          break;
        }
      }
      mAdditional = error;
    }
  }

  const String& Exception::getFullDescription() const
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
      else if ( mType == DirectInput )
      {
        const WinAPIError& error = boost::get<WinAPIError>( mAdditional );
        stream << L"\r\nDirectInput error code " << std::hex << error.code << L":\r\n" << error.description;
      }

      mFullDescription = stream.str();
    }
    return mFullDescription;
  }

  const char* Exception::what() const
  {
    mUTF8Description = Util::wideToUtf8( getFullDescription() );
    return mUTF8Description.c_str();
  }

}