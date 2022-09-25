#include "nilConfig.h"

#include "nil.h"
#include "nilUtil.h"

namespace nil {

  Exception::Exception( const utf8String& description, Type type ):
  description_( description ), type_( type )
  {
    handleAdditional();
  }

  Exception::Exception( const utf8String& description, const utf8String& source, Type type ):
  description_( description ), type_( type ), source_( source )
  {
    handleAdditional();
  }

  const utf8String& Exception::getFullDescription() const
  {
    if ( fullDescription_.empty() )
    {
      stringstream stream;

      stream << description_;

      if ( !source_.empty() )
        stream << "\r\nIn function " << source_;

#ifdef NIL_PLATFORM_WINDOWS

      if ( type_ == WinAPI )
      {
        const WinAPIError& error = std::get<WinAPIError>( additional_ );
        stream << "\r\nWinAPI error code " << std::hex << error.code << ":\r\n" << util::wideToUtf8( error.description );
      }
      else if ( type_ == DirectInput )
      {
        const WinAPIError& error = std::get<WinAPIError>( additional_ );
        stream << "\r\nDirectInput error code " << std::hex << error.code << ":\r\n" << util::wideToUtf8( error.description );
      }

#endif

      fullDescription_ = stream.str();
    }
    return fullDescription_;
  }

  const char* Exception::what() const throw( )
  {
    return fullDescription_.c_str();
  }

#ifdef NIL_PLATFORM_WINDOWS

  const map<uint32_t, wideString> c_dinputErrors = {
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

  Exception::Exception( const utf8String& description, const utf8String& source, HRESULT hr, Type type ):
  description_( description ), type_( type ), source_( source )
  {
    handleAdditional( hr );
  }

  void Exception::handleAdditional( HRESULT hr )
  {
    WinAPIError error;
    if ( type_ == WinAPI )
    {
      error.code = GetLastError();

      wchar_t* message = nullptr;
      FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, (DWORD)error.code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (wchar_t*)&message, 0, nullptr );

      if ( message )
      {
        error.description = message;
        LocalFree( message );
      }

      additional_ = error;
    }
    else if ( type_ == DirectInput )
    {
      error.code = hr;

      if ( c_dinputErrors.find( error.code ) != c_dinputErrors.end() )
        error.description = c_dinputErrors.at( error.code );

      additional_ = error;
    }
  }

#endif

}