#pragma once

#ifndef NTDDI_VERSION
# define NTDDI_VERSION NTDDI_VISTA
# define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbt.h>
#include <objbase.h>
#include <stdlib.h>

#include <initguid.h>
#include <devguid.h>
#include <devpkey.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>

#include <memory>
#include <stdint.h>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <queue>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

namespace nil {
  
  typedef int DeviceID;
  typedef uint32_t POVDirection;
  typedef unsigned int VirtualKeyCode;
  
  typedef float Real;

# define NIL_REAL_ZERO 0.0f
# define NIL_REAL_ONE 1.0f
# define NIL_REAL_MINUSONE -1.0f

  typedef std::string utf8String;
  typedef std::wstring String;

  using std::map;
  using std::list;
  using std::queue;
  using std::vector;
  using std::wstringstream;
  using boost::variant;

  //! \struct Vector2i
  //! Two-dimensional integer vector.
  struct Vector2i
  {
    public:
      int32_t x; //!< X-axis value
      int32_t y; //!< Y-axis value
      inline Vector2i(): x( 0 ), y( 0 ) {}
      inline explicit Vector2i( int32_t x_, int32_t y_ ): x( x_ ), y( y_ ) {}
      inline bool operator == ( const Vector2i& other ) const
      {
        return ( x == other.x && y == other.y );
      }
      inline bool operator != ( const Vector2i& other ) const
      {
        return ( x != other.x || y != other.y  );
      }
      inline Vector2i operator - ( const Vector2i& other ) const
      {
        return Vector2i( x - other.x, y - other.y );
      }
      const static Vector2i ZERO;
  };

  //! \struct Vector3i
  //! Three-dimensional integer vector.
  struct Vector3i
  {
    public:
      int32_t x; //!< X-axis value
      int32_t y; //!< Y-axis value
      int32_t z; //!< Z-axis value
      inline Vector3i(): x( 0 ), y( 0 ), z( 0 ) {}
      inline explicit Vector3i( int32_t x_, int32_t y_, int32_t z_ ):
      x( x_ ), y( y_ ), z( z_ ) {}
      inline bool operator == ( const Vector3i& other ) const
      {
        return ( x == other.x && y == other.y && z == other.z );
      }
      inline bool operator != ( const Vector3i& other ) const
      {
        return ( x != other.x || y != other.y || z != other.z  );
      }
      const static Vector3i ZERO;
  };

  //! \struct Color
  //! A color value.
  struct Color
  {
    public:
      Real r; //!< Red color component amount in {0..1}
      Real g; //!< Green color component amount in {0..1}
      Real b; //!< Blue color component amount in {0..1}
  };

  //! \struct Vector2f
  //! Two-dimensional floating point vector.
  struct Vector2f
  {
    public:
      Real x; //!< X-axis value
      Real y; //!< Y-axis value
      inline Vector2f(): x( NIL_REAL_ZERO ), y( NIL_REAL_ZERO ) {}
      inline explicit Vector2f( Real x_, Real y_ ): x( x_ ), y( y_ ) {}
      inline bool operator == ( const Vector2f& other ) const
      {
        return ( x == other.x && y == other.y );
      }
      inline bool operator != ( const Vector2f& other ) const
      {
        return ( x != other.x || y != other.y  );
      }
      const static Vector2f ZERO;
  };

  //! \struct Vector3f
  //! Three-dimensional floating point vector.
  struct Vector3f
  {
    public:
      Real x; //!< X-axis value
      Real y; //!< Y-axis value
      Real z; //!< Z-axis value
      inline Vector3f(): x( NIL_REAL_ZERO ), y( NIL_REAL_ZERO ), z( NIL_REAL_ZERO ) {}
      inline explicit Vector3f( Real x_, Real y_, Real z_ ):
      x( x_ ), y( y_ ), z( z_ ) {}
      inline bool operator == ( const Vector3f& other ) const
      {
        return ( x == other.x && y == other.y && z == other.z );
      }
      inline bool operator != ( const Vector3f& other ) const
      {
        return ( x != other.x || y != other.y || z != other.z  );
      }
      const static Vector3f ZERO;
  };

}