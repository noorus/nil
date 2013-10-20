#pragma once

#ifndef NTDDI_VERSION
# define NTDDI_VERSION NTDDI_VISTA
# define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif
#include <sdkddkver.h>
#include <windows.h>
#include <dbt.h>
#include <devguid.h>
#include <hidclass.h>

#include <stdlib.h>
#include <stdint.h>

#include <exception>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <boost/variant.hpp>

namespace nil {

  typedef std::string string;
  typedef std::wstring wstring;

  using std::list;
  using std::vector;
  using std::wstringstream;
  using boost::variant;

  //! \class WinAPIError
  //! Container for a Windows API error description.
  struct WinAPIError {
  public:
    uint32_t code;
    wstring description;
  };

  //! \class Exception
  //! Main exception class. Descendant of std::exception.
  class Exception: public std::exception {
  public:
    enum Type {
      Generic = 0,
      WinAPI
    };
  private:
    Exception();
  protected:
    Type mType;
    wstring mDescription;
    wstring mSource;
    mutable wstring mFullDescription;
    mutable string mUTF8Description;
    variant<WinAPIError> mAdditional;
    void handleAdditional();
  public:
    Exception( const wstring& description, Type type = Generic );
    Exception( const wstring& description, const wstring& source,
      Type type = Generic );
    virtual const wstring& getFullDescription() const;
    virtual const char* what() const throw();
  };

  //! \class System
  //! The input system.
  class System {
  protected:
  public:
    System();
    ~System();
  };

}