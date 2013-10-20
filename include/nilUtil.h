#pragma once
#include <windows.h>
#include <devguid.h>

namespace nil {

# ifndef SAFE_DELETE
#   define SAFE_DELETE(p) {if(p){delete p;(p)=NULL;}}
# endif

# ifndef SAFE_RELEASE
#   define SAFE_RELEASE(p) {if(p){p->Release();(p)=NULL;}}
# endif

# if defined(EXCEPT) || defined(EXCEPT_WINAPI)
#   error EXCEPT* maro already defined!
# else
#   define EXCEPT(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::Generic);}
#   define EXCEPT_WINAPI(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::WinAPI);}
# endif

  static GUID g_HIDInterfaceGUID = { 0x4D1E55B2L, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

  namespace util
  {
    extern inline wstring utf8ToWide( const string& in ) throw();
    extern inline string wideToUtf8( const wstring& in ) throw();
  }

}