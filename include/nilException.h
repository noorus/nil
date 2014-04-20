#pragma once
#include "nilTypes.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  struct WinAPIError
  {
    public:
      uint32_t code;
      String description;
  };

  //! \class Exception
  //! Main Nil exception class.
  //! \sa std::exception
  class Exception: public std::exception
  {
    public:
      //! Type of exception.
      enum Type: int
      {
        Generic = 0,  //!< An enum constant representing the generic option
        WinAPI, //!< Windows API-specific error
        DirectInput //!< DirectInput-specific error
      };
    private:
      Exception();

      Type mType; //!< Exception type
      String mDescription; //!< Exception description
      String mSource; //!< Exception source
      mutable String mFullDescription; //!< Full, extended description
      mutable utf8String mUTF8Description; //!< Full, extended description in UTF-8
      variant<WinAPIError> mAdditional; //!< \b Internal Additional exception data

      //! \b Internal Handle additional exception data for WinAPI/DI exceptions.
      void handleAdditional( HRESULT hr = 0 );
    public:
      //! Generic constructor.
      Exception( const String& description, Type type = Generic );

      //! Constructor with source.
      Exception( const String& description, const String& source,
        Type type = Generic );

      //! Constructor with source and a WinAPI/DirectInput error code.
      Exception( const String& description, const String& source,
        HRESULT hr, Type type = Generic );

      //! Get the full, extended description of the exception.
      virtual const String& getFullDescription() const;

      //! Get the std::exception compatible exception description.
      virtual const char* what() const throw();
  };

  //! @}

}