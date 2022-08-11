#pragma once
#include "nilConfig.h"

#include "nilTypes.h"

namespace nil {

  //! \addtogroup Nil
  //! @{

  struct WinAPIError
  {
    uint32_t code;
    wideString description;
  };

  //! \class Exception
  //! Main Nil exception class.
  //! \sa std::exception
  class Exception: public std::exception {
  public:
    //! Type of exception.
    enum Type: int
    {
      Generic = 0,  //!< An enum constant representing the generic option
      WinAPI, //!< Windows API-specific error
      DirectInput //!< DirectInput-specific error
    };
  private:
    Exception() = default;

    utf8String description_; //!< Exception description
    Type type_; //!< Exception type
    utf8String source_; //!< Exception source
    mutable utf8String fullDescription_; //!< Full, extended description
    variant<WinAPIError> additional_; //!< \b Internal Additional exception data

    //! \b Internal Handle additional exception data for WinAPI/DI exceptions.
    void handleAdditional( HRESULT hr = 0 );
  public:
    //! Generic constructor.
    Exception( const utf8String& description, Type type = Generic );

    //! Constructor with source.
    Exception( const utf8String& description, const utf8String& source,
      Type type = Generic );

    //! Constructor with source and a WinAPI/DirectInput error code.
    Exception( const utf8String& description, const utf8String& source,
      HRESULT hr, Type type = Generic );

    //! Get the full, extended description of the exception.
    virtual const utf8String& getFullDescription() const;

    //! Get the std::exception compatible exception description.
    const char* what() const throw() override;
  };

  //! @}

}