#pragma once
#include "nilConfig.h"

#include <memory>
#include <cstdint>
#include <exception>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <sstream>
#include <queue>
#include <variant>
#include <set>
#include <algorithm>
#include <numeric>

namespace nil {

  //! \addtogroup Nil
  //! @{

  //! \addtogroup Types
  //! @{

  using DeviceID = uint32_t; //!< A device ID type.
  using POVDirection = uint32_t; //!< A POV (D-pad) direction type.
  using VirtualKeyCode = unsigned int; //!< A virtual key code type.

  using Real = float; //!< Real number type.

# define NIL_REAL_ZERO 0.0f //!< Real number zero constant.
# define NIL_REAL_ONE 1.0f //!< Real number one constant.
# define NIL_REAL_MINUSONE -1.0f //!< Real number minus one constant.

  using utf8String = std::string; //!< UTF-8 string type.
  using wideString = std::wstring; //!< Wide string type.

  enum class Cooperation {
    Foreground,
    Background
  };

  using std::map;
  using std::list;
  using std::queue;
  using std::vector;
  using std::basic_string;
  using std::stringstream;
  using std::wstringstream;
  using std::unique_ptr;
  using std::shared_ptr;
  using std::move;
  using std::variant;
  using std::make_shared;
  using std::make_unique;
  using std::set;

  class System;

  using SystemPtr = shared_ptr<System>;

  //! \struct Color
  //! A color value.
  struct Color
  {
    Real r; //!< Red color component in {0..1}
    Real g; //!< Green color component in {0..1}
    Real b; //!< Blue color component in {0..1}
  };

  //! \struct Vector2i
  //! Two-dimensional integer vector.
  struct Vector2i
  {
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

    const static Vector2i ZERO; //!< Static default zero vector
  };

  //! \struct Vector3i
  //! Three-dimensional integer vector.
  struct Vector3i
  {
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

    const static Vector3i ZERO; //!< Static default zero vector
  };

  //! \struct Vector2f
  //! Two-dimensional floating point vector.
  struct Vector2f
  {
    Real x; //!< X-axis value
    Real y; //!< Y-axis value

    inline Vector2f(): x( NIL_REAL_ZERO ), y( NIL_REAL_ZERO ) {}

    inline explicit Vector2f( Real x_, Real y_ ): x( x_ ), y( y_ ) {}

    inline bool operator == ( const Vector2f& other ) const
    {
      return ( x == other.x && y == other.y ); //-V550
    }

    inline bool operator != ( const Vector2f& other ) const
    {
      return ( x != other.x || y != other.y  ); //-V550
    }

    const static Vector2f ZERO; //!< Static default zero vector
  };

  //! \struct Vector3f
  //! Three-dimensional floating point vector.
  struct Vector3f
  {
    Real x; //!< X-axis value
    Real y; //!< Y-axis value
    Real z; //!< Z-axis value

    inline Vector3f(): x( NIL_REAL_ZERO ), y( NIL_REAL_ZERO ), z( NIL_REAL_ZERO ) {}

    inline explicit Vector3f( Real x_, Real y_, Real z_ ):
    x( x_ ), y( y_ ), z( z_ ) {}

    inline bool operator == ( const Vector3f& other ) const
    {
      return ( x == other.x && y == other.y && z == other.z ); //-V550
    }

    inline bool operator != ( const Vector3f& other ) const
    {
      return ( x != other.x || y != other.y || z != other.z  ); //-V550
    }

    const static Vector3f ZERO; //!< Static default zero vector
  };

  //! @}

  //! @}

}