#pragma once
#include "nil.h"
#include <boost/noncopyable.hpp>

namespace Nil {

  //! \addtogroup Nil
  //! @{

  //! \addtogroup Utilities
  //! @{

  //! \def SAFE_DELETE(p)
  //! \brief Safely delete and null-out a pointer to an object.
# ifndef SAFE_DELETE
#   define SAFE_DELETE(p) {if(p){delete p;(p)=NULL;}}
# endif

  //! \def SAFE_RELEASE(p)
  //! \brief Safely release and null-out a COM object.
# ifndef SAFE_RELEASE
#   define SAFE_RELEASE(p) {if(p){p->Release();(p)=NULL;}}
# endif

  //! \def SAFE_CLOSEHANDLE(p)
  //! \brief Safely close and invalidate a Windows handle value.
# ifndef SAFE_CLOSEHANDLE
#   define SAFE_CLOSEHANDLE(p) {if(p!=INVALID_HANDLE_VALUE){CloseHandle(p);(p)=INVALID_HANDLE_VALUE;}}
# endif

# if defined(NIL_EXCEPT) || defined(NIL_EXCEPT_WINAPI) || defined(NIL_EXCEPT_DINPUT)
#   error NIL_EXCEPT* macro already defined!
# else
  //! \brief A generic exception.
#   define NIL_EXCEPT(description) {throw Nil::Exception(description,__FUNCTIONW__,Nil::Exception::Generic);}
  //! \brief A WinAPI exception.
#   define NIL_EXCEPT_WINAPI(description) {throw Nil::Exception(description,__FUNCTIONW__,Nil::Exception::WinAPI);}
  //! \brief A DirectInput exception.
#   define NIL_EXCEPT_DINPUT(hr,description) {throw Nil::Exception(description,__FUNCTIONW__,hr,Nil::Exception::DirectInput);}
# endif

  static GUID g_HIDInterfaceGUID = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

  //! \class SafeHandle
  //! \brief Unique_ptr wrapper for WinAPI handles.
  class SafeHandle: public std::unique_ptr<std::remove_pointer<HANDLE>::type,void(*)( HANDLE )>
  {
    public:
      //! \brief Constructor.
      //! \param  handle Handle of the handle.
      SafeHandle( HANDLE handle ): unique_ptr( handle, &SafeHandle::close )
      {
      }
      //! \brief HANDLE casting operator.
      operator HANDLE()
      {
        return get();
      }

      //! \brief Valids this SafeHandle.
      //! \return true if it succeeds, false if it fails.
      const bool valid() const
      {
        return ( get() != INVALID_HANDLE_VALUE );
      }
    private:
      //! \brief Closes the given handle.
      //! \param  handle Handle of the handle.
      static void close( HANDLE handle )
      {
        if ( handle != INVALID_HANDLE_VALUE )
          CloseHandle( handle );
      }
  };

  //! \class ScopedSRWLock
  //! \brief Automation for scoped acquisition and release of an SRWLOCK.
  //! \warning Lock must be initialized in advance!
  class ScopedSRWLock: boost::noncopyable
  {
    protected:
      PSRWLOCK mLock; //!< The lock
      bool mExclusive;  //!< true to exclusive
    public:
      //! \brief Constructor.
      //! \param  lock      The lock.
      //! \param  exclusive (Optional) true to exclusive.
      ScopedSRWLock( PSRWLOCK lock, bool exclusive = true ):
      mLock( lock ), mExclusive( exclusive )
      {
        mExclusive ? AcquireSRWLockExclusive( mLock ) : AcquireSRWLockShared( mLock );
      }
      //! \brief Unlocks this ScopedSRWLock.
      void unlock()
      {
        if ( mLock )
          mExclusive ? ReleaseSRWLockExclusive( mLock ) : ReleaseSRWLockShared( mLock );
        mLock = nullptr;
      }
      //! \brief Destructor.
      ~ScopedSRWLock()
      {
        unlock();
      }
  };

  //! Fowler/Noll/Vo hash initialization value
# define FNV1_32_INIT ((uint32_t)0x811c9dc5)
  //! Fowler/Noll/Vo hash initialization value
# define FNV1_32A_INIT FNV1_32_INIT

  namespace Util
  {
    //! \brief 32-bit Fowler/Noll/Vo-1a-hash function.
    //! \param [in,out] buf If non-null, the buffer.
    //! \param  len         The length.
    //! \param  hashval     The hashval.
    //! \return An uint32_t.
    extern uint32_t fnv_32a_buf( void* buf, size_t len, uint32_t hashval );

    //! \brief Cleanup a device name.
    //! \param  name The name.
    //! \return A String.
    extern inline String cleanupName( String name ) throw();

    //! \brief UTF-8 to wide string conversion.
    //! \param  in The string.
    //! \return A String.
    extern inline String utf8ToWide( const utf8String& in ) throw();

    //! \brief Wide string to UTF-8 conversion.
    //! \param  in The string.
    //! \return An utf8String.
    extern inline utf8String wideToUtf8( const String& in ) throw();

    //! \brief Auto-generate a name for a nameless device.
    //! \param  deviceType Type of the device.
    //! \param  index      Zero-based device index.
    //! \return The name.
    String generateName( Device::Type deviceType, int index ) throw();
  }

  //! @}

  //! @}

}