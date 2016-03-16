#include "nil.h"
#include "nilUtil.h"

namespace Nil {

  struct DirectInputErrorEntry {
  public:
    uint32_t code;
    wchar_t* description;
  };

  const int cErrorDescriptionCount = 32; //-V112

  const DirectInputErrorEntry cErrorDescriptions[cErrorDescriptionCount] =
  {
    { (uint32_t)DIERR_ACQUIRED, L"The operation cannot be performed while the device is acquired." },
    { (uint32_t)DIERR_ALREADYINITIALIZED, L"The object is already initialized." },
    { (uint32_t)DIERR_BADDRIVERVER, L"The object could not be created due to an incompatible driver." },
    { (uint32_t)DIERR_DEVICEFULL, L"The device is full." },
    { (uint32_t)DIERR_DEVICENOTREG, L"The device or device instance is not registered with DirectInput." },
    { (uint32_t)DIERR_EFFECTPLAYING, L"The device does not support updating an effect while it is still playing." },
    { (uint32_t)DIERR_GENERIC, L"An undetermined error occurred inside the DirectInput subsystem." },
    { (uint32_t)DIERR_HANDLEEXISTS, L"The device already has an event notification associated with it." },
    { (uint32_t)DIERR_HASEFFECTS, L"The device cannot be reinitialized because effects are attached to it." },
    { (uint32_t)DIERR_INCOMPLETEEFFECT, L"The effect could not be downloaded because essential information is missing." },
    { (uint32_t)DIERR_INPUTLOST, L"Access to the input device has been lost. It must be reacquired." },
    { (uint32_t)DIERR_INVALIDPARAM, L"An invalid parameter was passed to the function." },
    { (uint32_t)DIERR_MAPFILEFAIL, L"An error occurred reading or writing the action-mapping file for the device." },
    { (uint32_t)DIERR_MOREDATA, L"Not all the requested information fit into the buffer." },
    { (uint32_t)DIERR_NOAGGREGATION, L"The object does not support aggregation." },
    { (uint32_t)DIERR_NOINTERFACE, L"The object does not support the specified interface." },
    { (uint32_t)DIERR_NOTACQUIRED, L"The operation cannot be performed unless the device is acquired." },
    { (uint32_t)DIERR_NOTBUFFERED, L"The device is not buffered." },
    { (uint32_t)DIERR_NOTDOWNLOADED, L"The effect was not downloaded." },
    { (uint32_t)DIERR_NOTEXCLUSIVEACQUIRED, L"The operation cannot be performed unless the device is acquired in exclusive mode." },
    { (uint32_t)DIERR_NOTFOUND, L"The requested object does not exist." },
    { (uint32_t)DIERR_NOTINITIALIZED, L"The object has not been initialized." },
    { (uint32_t)DIERR_OLDDIRECTINPUTVERSION, L"The application requires a newer version of DirectInput." },
    { (uint32_t)DIERR_OTHERAPPHASPRIO, L"Another application has a higher priority level." },
    { (uint32_t)DIERR_OUTOFMEMORY, L"The DirectInput subsystem is out of memory." },
    { (uint32_t)DIERR_READONLY, L"The specified property is read-only." },
    { (uint32_t)DIERR_REPORTFULL, L"More information was requested than can be sent to the device." },
    { (uint32_t)DIERR_UNPLUGGED, L"The device is not plugged in." },
    { (uint32_t)DIERR_UNSUPPORTED, L"The function called is not supported at this time." },
    { (uint32_t)E_HANDLE, L"The HWND parameter is not a valid top-level window that belongs to the process." },
    { (uint32_t)E_PENDING, L"Data is not yet available." },
    { (uint32_t)E_POINTER, L"An invalid pointer was passed as a parameter." }
  };

  Exception::Exception()
  {
    // Stub
  }

  Exception::Exception( const utf8String& description, Type type ):
  mDescription( description ), mType( type )
  {
    handleAdditional();
  }

  Exception::Exception( const utf8String& description, const utf8String& source, Type type ):
  mDescription( description ), mSource( source ), mType( type )
  {
    handleAdditional();
  }

  Exception::Exception( const utf8String& description, const utf8String& source, HRESULT hr, Type type ):
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

      wchar_t* message = nullptr;
      FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, (DWORD)error.code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (wchar_t*)&message, 0, NULL );

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

  const utf8String& Exception::getFullDescription() const
  {
    if ( mFullDescription.empty() )
    {
      stringstream stream;

      stream << mDescription;

      if ( !mSource.empty() )
        stream << "\r\nIn function " << mSource;

      if ( mType == WinAPI )
      {
        const WinAPIError& error = boost::get<WinAPIError>( mAdditional );
        stream << "\r\nWinAPI error code " << std::hex << error.code << ":\r\n" << Util::wideToUtf8( error.description );
      }
      else if ( mType == DirectInput )
      {
        const WinAPIError& error = boost::get<WinAPIError>( mAdditional );
        stream << "\r\nDirectInput error code " << std::hex << error.code << ":\r\n" << Util::wideToUtf8( error.description );
      }

      mFullDescription = stream.str();
    }
    return mFullDescription;
  }

  const char* Exception::what() const
  {
    return mFullDescription.c_str();
  }

}