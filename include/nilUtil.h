#pragma once
#include "nil.h"
#include <boost/noncopyable.hpp>

namespace nil {

# ifndef SAFE_DELETE
#   define SAFE_DELETE(p) {if(p){delete p;(p)=NULL;}}
# endif

# ifndef SAFE_RELEASE
#   define SAFE_RELEASE(p) {if(p){p->Release();(p)=NULL;}}
# endif

# ifndef SAFE_CLOSEHANDLE
#   define SAFE_CLOSEHANDLE(p) {if(p!=INVALID_HANDLE_VALUE){CloseHandle(p);(p)=INVALID_HANDLE_VALUE;}}
# endif

# if defined(NIL_EXCEPT) || defined(NIL_EXCEPT_WINAPI) || defined(NIL_EXCEPT_DINPUT)
#   error EXCEPT* maro already defined!
# else
#   define NIL_EXCEPT(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::Generic);}
#   define NIL_EXCEPT_WINAPI(description) {throw nil::Exception(description,__FUNCTIONW__,nil::Exception::WinAPI);}
#   define NIL_EXCEPT_DINPUT(hr,description) {throw nil::Exception(description,__FUNCTIONW__,hr,nil::Exception::DirectInput);}
# endif

  static GUID g_HIDInterfaceGUID = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

  //! \class SafeHandle
  //! Unique_ptr wrapper for WinAPI handles.
  class SafeHandle: public std::unique_ptr<std::remove_pointer<HANDLE>::type,void(*)( HANDLE )>
  {
    public:
      SafeHandle( HANDLE handle ): unique_ptr( handle, &SafeHandle::close )
      {
      }
      operator HANDLE()
      {
        return get();
      }
      const bool valid() const
      {
        return ( get() != INVALID_HANDLE_VALUE );
      }
    private:
      static void close( HANDLE handle )
      {
        if ( handle != INVALID_HANDLE_VALUE )
          CloseHandle( handle );
      }
  };

  //! \class ScopedSRWLock
  //! Automation for scoped acquisition and release of an SRWLOCK.
  //! \warning Lock must be initialized in advance!
  class ScopedSRWLock: boost::noncopyable
  {
    protected:
      PSRWLOCK mLock;
      bool mExclusive;
    public:
      ScopedSRWLock( PSRWLOCK lock, bool exclusive = true ):
      mLock( lock ), mExclusive( exclusive )
      {
        mExclusive ? AcquireSRWLockExclusive( mLock ) : AcquireSRWLockShared( mLock );
      }
      void unlock()
      {
        if ( mLock )
          mExclusive ? ReleaseSRWLockExclusive( mLock ) : ReleaseSRWLockShared( mLock );
        mLock = nullptr;
      }
      ~ScopedSRWLock()
      {
        unlock();
      }
  };

  namespace util
  {
    extern inline String cleanupName( String name ) throw();
    extern inline String utf8ToWide( const utf8String& in ) throw();
    extern inline utf8String wideToUtf8( const String& in ) throw();
    String generateName( Device::Type deviceType, int index ) throw();
  }

}