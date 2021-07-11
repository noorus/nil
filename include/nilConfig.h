#pragma once

#ifdef _MSC_VER
# define NIL_PLATFORM_WINDOWS
#else
# error Unknown platform!
#endif

#ifdef NIL_PLATFORM_WINDOWS
# ifndef NTDDI_VERSION
#  define NTDDI_VERSION NTDDI_WIN10
#  define _WIN32_WINNT _WIN32_WINNT_WIN10
# endif
# include <sdkddkver.h>
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <dbt.h>
# include <objbase.h>
# include <cstdlib>
# include <cassert>

# include <initguid.h>
# include <devguid.h>
# include <devpkey.h>

# define DIRECTINPUT_VERSION 0x0800
# include <dinput.h>
# include <xinput.h>
#endif