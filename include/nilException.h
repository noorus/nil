#pragma once
#include "nilTypes.h"

namespace nil {

  //! \class WinAPIError
  //! Container for a Windows API error description.
  struct WinAPIError
  {
    public:
      uint32_t code;
      String description;
  };

  //! \class Exception
  //! Main NIL exception class.
  class Exception: public std::exception
  {
    public:
      enum Type: int
      {
        Generic = 0, //!< Generic NIL error
        WinAPI, //!< Windows API-specific error
        DirectInput //!< DirectInput-specific error
      };
    private:
      Exception();
    protected:
      Type mType;
      String mDescription;
      String mSource;
      mutable String mFullDescription;
      mutable utf8String mUTF8Description;
      variant<WinAPIError> mAdditional;
      void handleAdditional( HRESULT hr = 0 );
    public:
      Exception( const String& description, Type type = Generic );
      Exception( const String& description, const String& source,
        Type type = Generic );
      Exception( const String& description, const String& source,
        HRESULT hr, Type type = Generic );
      virtual const String& getFullDescription() const;
      virtual const char* what() const throw();
  };

}