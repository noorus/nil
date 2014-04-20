#pragma once
#include "nilTypes.h"

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! \struct WinAPIError
  //! \brief Container for a Windows API error description.
  struct WinAPIError
  {
    public:
      uint32_t code; //!< WinAPI error code
      String description; //!< WinAPI error description
  };

  //! \class Exception
  //! \brief Main Nil exception class.
  //! \sa std::exception
  class Exception: public std::exception
  {
    public:
      //! \brief Values that represent Type.
      enum Type: int
      {
        Generic = 0,  //!< An enum constant representing the generic option
        WinAPI, //!< Windows API-specific error
        DirectInput //!< DirectInput-specific error
      };
    private:
      //! \brief Default constructor.
      Exception();
    protected:
      Type mType; //!< Exception type
      String mDescription; //!< Exception description
      String mSource; //!< Exception source
      mutable String mFullDescription; //!< Full, extended description
      mutable utf8String mUTF8Description; //!< Full, extended description in UTF-8
      variant<WinAPIError> mAdditional; //!< Internal additional exception data

      //! \brief Handles additional exception data for WinAPI/DI exceptions.
      //! \param  hr HRESULT error code.
      void handleAdditional( HRESULT hr = 0 );
    public:
      //! \brief Constructor.
      //! \param  description The description.
      //! \param  type        (Optional) the type.
      Exception( const String& description, Type type = Generic );

      //! \brief Constructor.
      //! \param  description The description.
      //! \param  source      Source of exception.
      //! \param  type        (Optional) the type.
      Exception( const String& description, const String& source,
        Type type = Generic );

      //! \brief Constructor.
      //! \param  description The description.
      //! \param  source      Source of exception.
      //! \param  hr          Additional HRESULT for WinAPI/DI.
      //! \param  type        (Optional) the type.
      Exception( const String& description, const String& source,
        HRESULT hr, Type type = Generic );

      //! \brief Gets full, extended description of the exception.
      //! \return The full description.
      virtual const String& getFullDescription() const;

      //! \brief Gets the what.
      //! \return Compact description.
      virtual const char* what() const throw();
  };

  //! @}

}