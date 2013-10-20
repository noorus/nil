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

#include <string>
#include <exception>

namespace nil {

  typedef std::string string;
  typedef std::wstring wstring;

  class System {
  protected:
  public:
    System();
    void test();
    ~System();
  };

}