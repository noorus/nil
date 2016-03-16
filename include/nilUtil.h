#pragma once
#include "nil.h"
#include <boost/noncopyable.hpp>

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! \addtogroup Utilities
  //! @{

  //! \def SAFE_DELETE(p)
  //! Safely delete and null-out a pointer to an object.
# ifndef SAFE_DELETE
#   define SAFE_DELETE(p) {if(p){delete p;(p)=NULL;}}
# endif

  //! \def SAFE_RELEASE(p)
  //! Safely release and null-out a COM object.
# ifndef SAFE_RELEASE
#   define SAFE_RELEASE(p) {if(p){p->Release();(p)=NULL;}}
# endif

  //! \def SAFE_CLOSEHANDLE(p)
  //! Safely close and invalidate a Windows handle.
# ifndef SAFE_CLOSEHANDLE
#   define SAFE_CLOSEHANDLE(p) {if(p!=INVALID_HANDLE_VALUE){CloseHandle(p);(p)=INVALID_HANDLE_VALUE;}}
# endif

# if defined(NIL_EXCEPT) || defined(NIL_EXCEPT_WINAPI) || defined(NIL_EXCEPT_DINPUT)
#   error NIL_EXCEPT* macro already defined!
# else
  //! Fire a generic exception.
#   define NIL_EXCEPT(description) {throw Nil::Exception(description,__FUNCTION__,Nil::Exception::Generic);}
  //! Fire a WinAPI exception.
#   define NIL_EXCEPT_WINAPI(description) {throw Nil::Exception(description,__FUNCTION__,Nil::Exception::WinAPI);}
  //! Fire a DirectInput exception.
#   define NIL_EXCEPT_DINPUT(hr,description) {throw Nil::Exception(description,__FUNCTION__,hr,Nil::Exception::DirectInput);}
# endif

  static GUID g_HIDInterfaceGUID = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

  //! \class SafeHandle
  //! Unique_ptr wrapper for WinAPI handles.
  //! \sa std::unique_ptr
  class SafeHandle: public std::unique_ptr<std::remove_pointer<HANDLE>::type,void(*)( HANDLE )>
  {
    public:
      //! Wrapper constructor for a given WinAPI HANDLE.
      SafeHandle( HANDLE handle ): unique_ptr( handle, &SafeHandle::close )
      {
      }
      //! Quickie HANDLE conversion operator.
      operator HANDLE()
      {
        return get();
      }
      //! Is this handle valid?
      const bool valid() const
      {
        return ( get() != INVALID_HANDLE_VALUE );
      }
    private:
      //! My handle closing callback for unique_ptr.
      static void close( HANDLE handle )
      {
        if ( handle != INVALID_HANDLE_VALUE )
          CloseHandle( handle );
      }
  };

  //! \class ScopedSRWLock
  //! Automation for scoped acquisition and release of an SRWLOCK.
  //! \warning Lock must be initialized in advance!
  class ScopedSRWLock: boost::noncopyable {
  protected:
    PSRWLOCK mLock; //!< The lock
    bool mExclusive; //!< Whether we're acquired in exclusive mode
    bool mLocked; //!< Whether we're still locked
  public:
    //! \brief Constructor.
    //! \param  lock      The lock to acquire.
    //! \param  exclusive (Optional) true to acquire in exclusive mode, false for shared.
    ScopedSRWLock( PSRWLOCK lock, bool exclusive = true ):
      mLock( lock ), mExclusive( exclusive ), mLocked( true )
    {
      mExclusive ? AcquireSRWLockExclusive( mLock ) : AcquireSRWLockShared( mLock );
    }
    //! Call directly if you want to unlock before object leaves scope.
    _Requires_lock_held_(mLock) void unlock()
    {
      if ( mLocked )
        mExclusive ? ReleaseSRWLockExclusive( mLock ) : ReleaseSRWLockShared( mLock );
      mLocked = false;
    }
    //! Destructor.
    ~ScopedSRWLock()
    {
      unlock();
    }
  };

  //! 32-bit Fowler/Noll/Vo hash initialization value
# define FNV1_32_INIT ((uint32_t)0x811c9dc5)
  //! 32-bit Fowler/Noll/Vo hash initialization value
# define FNV1_32A_INIT FNV1_32_INIT

  namespace Util
  {
    //! 32-bit Fowler/Noll/Vo-1a hash function.
    extern uint32_t fnv_32a_buf( void* buf, size_t len, uint32_t hashval );
    //! Cleanup a device name.
    extern inline utf8String cleanupName( utf8String name ) throw();
    //! UTF-8 to wide string conversion.
    extern inline wideString utf8ToWide( const utf8String& in ) throw();
    //! Wide string to UTF-8 conversion.
    extern inline utf8String wideToUtf8( const wideString& in ) throw();
    //! Auto-generate a name for a nameless device.
    utf8String generateName( Device::Type deviceType, int index ) throw();
  }

  //! @}

  //! @}

}