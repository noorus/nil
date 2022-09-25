#pragma once
#include "nilConfig.h"

#include "nil.h"

namespace nil {

  //! \addtogroup Nil
  //! @{

  //! \addtogroup Utilities
  //! @{

  //! \def SAFE_RELEASE(p)
  //! Safely release and null-out a COM object.
# ifndef SAFE_RELEASE
#   define SAFE_RELEASE(p) {if(p){p->Release();(p)=NULL;}}
# endif

# if defined(NIL_EXCEPT) || defined(NIL_EXCEPT_WINAPI) || defined(NIL_EXCEPT_DINPUT)
#   error NIL_EXCEPT* macro already defined!
# else
  //! Fire a generic exception.
#   define NIL_EXCEPT(description) {throw nil::Exception(description,__FUNCTION__,nil::Exception::Generic);}
  //! Fire a WinAPI exception.
#   define NIL_EXCEPT_WINAPI(description) {throw nil::Exception(description,__FUNCTION__,nil::Exception::WinAPI);}
  //! Fire a DirectInput exception.
#   define NIL_EXCEPT_DINPUT(hr,description) {throw nil::Exception(description,__FUNCTION__,hr,nil::Exception::DirectInput);}
# endif

  // Initial known value is hardcoded here, but it gets replaced by what HidD_GetHidGuid returns later
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
      bool valid() const
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
  class ScopedSRWLock {
  protected:
    PSRWLOCK lock_; //!< The lock
    bool exclusive_; //!< Whether we're acquired in exclusive mode
    bool isLocked_; //!< Whether we're still locked
  public:
    //! \brief Constructor.
    //! \param  lock      The lock to acquire.
    //! \param  exclusive (Optional) true to acquire in exclusive mode, false for shared.
    _Acquires_lock_(lock_) ScopedSRWLock( PSRWLOCK lock, bool exclusive = true ):
    lock_( lock ), exclusive_( exclusive ), isLocked_( true )
    {
      exclusive_ ? AcquireSRWLockExclusive( lock_ ) : AcquireSRWLockShared( lock_ );
    }
    //! Call directly if you want to unlock before object leaves scope.
    _Releases_lock_(lock_) void unlock()
    {
      if ( isLocked_ )
        exclusive_ ? ReleaseSRWLockExclusive( lock_ ) : ReleaseSRWLockShared( lock_ );
      isLocked_ = false;
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

  namespace util
  {
    //! 32-bit Fowler/Noll/Vo-1a hash function.
    inline uint32_t fnv_32a_buf( void *buf, size_t len, uint32_t hashval )
    {
      unsigned char* bp = (unsigned char*)buf;
      unsigned char* be = bp + len;
      while ( bp < be )
      {
        hashval ^= (uint32_t)*bp++;
        hashval *= 0x01000193;
      }
      return hashval;
    }
    
    inline void ltrim( utf8String& s )
    {
      s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( unsigned char ch )
      {
        return !std::isspace( ch );
      }));
    }

    inline void rtrim( utf8String& s )
    {
      s.erase( std::find_if( s.rbegin(), s.rend(), []( unsigned char ch )
      {
        return !std::isspace( ch );
      }).base(), s.end() );
    }

    inline void trim( utf8String& s )
    {
      ltrim( s );
      rtrim( s );
    }

    //! Cleanup a device name.
    inline utf8String cleanupName( utf8String name ) throw()
    {
      trim( name );
      if ( name.compare( "?" ) != 0 )
        return "";
      return name;
    }

#ifdef NIL_PLATFORM_WINDOWS

    //! UTF-8 to wide string conversion.
    inline wideString utf8ToWide( const utf8String& in ) throw()
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return wideString();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return wideString( &conversion[0] );
    }

    //! Wide string to UTF-8 conversion.
    inline utf8String wideToUtf8( const wideString& in ) throw()
    {
      int length = WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, nullptr, 0, 0, FALSE );
      if ( length == 0 )
        return utf8String();
      vector<char> conversion( length );
      WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length, 0, FALSE );
      return utf8String( &conversion[0] );
    }

    template <typename T>
    inline void searchAndReplace( T& str, const T& search, const T& replace )
    {
      auto pos = str.find( search );
      while ( pos != T::npos )
      {
        str.replace( pos, search.size(), replace );
        pos = str.find( search, pos + replace.size() );
      }
    }

    template <typename T>
    inline void stringSplit( const basic_string<T>& str, vector<basic_string<T>>& sink, T delimiter )
    {
      size_t start = 0, end = 0;
      while ( ( end = str.find( delimiter, start ) ) != basic_string<T>::npos )
      {
        sink.push_back( str.substr( start, end - start ) );
        start = end + 1;
      }
      sink.push_back( str.substr( start ) );
    }

    template <typename T>
    inline basic_string<T> stringJoin( const vector<basic_string<T>>& parts, const basic_string<T>& delimiter )
    {
      return parts.empty() ? basic_string<T>() : std::accumulate(
        ++parts.begin(), parts.end(), *parts.begin(),
        [&delimiter]( const auto& a, const auto& b ) { return ( a + delimiter + b ); } );
    }

    template <typename T>
    inline void stringDeleteBetween( basic_string<T>& str, T in, T out )
    {
      while ( str.find( in ) != basic_string<T>::npos && str.find( out ) != basic_string<T>::npos )
      {
        auto op = str.find( in );
        auto cl = str.find( out, op );
        if ( op == basic_string<T>::npos || cl == basic_string<T>::npos )
          break;
        str.erase( op, cl - op + 1 );
      }
    }

    //! Auto-generate a name for a nameless device.
    inline utf8String generateName( Device::Type deviceType, int index ) throw()
    {
      char name[64] = { 0 };
      switch ( deviceType )
      {
        case Device::Device_Mouse:
          sprintf_s( name, 64, "Mouse %d", index );
        break;
        case Device::Device_Keyboard:
          sprintf_s( name, 64, "Keyboard %d", index );
        break;
        case Device::Device_Controller:
          sprintf_s( name, 64, "Controller %d", index );
        break;
      }
      return name;
    }

    inline bool compareDevicePaths( wideString a, wideString b )
    {
      stringDeleteBetween( a, L'{', L'}' );
      stringDeleteBetween( b, L'{', L'}' );

      std::transform( a.begin(), a.end(), a.begin(), ::towlower );
      std::transform( b.begin(), b.end(), b.begin(), ::towlower );

      constexpr auto mustStartWith = LR"(\\?\hid)";
      if ( a.substr( 0, 7 ) != mustStartWith || b.substr( 0, 7 ) != mustStartWith )
        return false;

      return ( a.find( b ) != wideString::npos || b.find( a ) != wideString::npos );
    }

#endif
  }

  //! @}

  //! @}

}