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

# if defined(NIL_EXCEPT) || defined(NIL_EXCEPT_WINAPI) || defined(NIL_EXCEPT_DINPUT)
#   error EXCEPT* maro already defined!
# else
#   define NIL_EXCEPT(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::Generic);}
#   define NIL_EXCEPT_WINAPI(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::WinAPI);}
#   define NIL_EXCEPT_DINPUT(hr,description) {throw nil::Exception(description,__FUNCTIONW__,hr,nil::Exception::DirectInput);}
# endif

  static GUID g_HIDInterfaceGUID;

  static wstring guidToStr( GUID guid )
  {
    OLECHAR* bstrGuid;
    StringFromCLSID( guid, &bstrGuid );
    wstring str = bstrGuid;
    ::CoTaskMemFree( bstrGuid );
    return str;
  }

  class COMString {
  protected:
    BSTR mStr;
  public:
    COMString( const wstring& str ): mStr( NULL )
    {
      mStr = SysAllocString( str.c_str() );
      if ( !mStr )
        NIL_EXCEPT( L"SysAllocString failed" );
    }
    operator BSTR() { return mStr; }
    ~COMString()
    {
      if ( mStr )
        SysFreeString( mStr );
    }
  };

  namespace util
  {
    extern inline wstring utf8ToWide( const string& in ) throw();
    extern inline string wideToUtf8( const wstring& in ) throw();
  }

}