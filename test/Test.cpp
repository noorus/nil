#include "nil.h"
#include "nilUtil.h"

HANDLE stopEvent = NULL;

BOOL WINAPI consoleHandler( DWORD ctrl )
{
  if ( ctrl == CTRL_C_EVENT || ctrl == CTRL_CLOSE_EVENT )
  {
    SetEvent( stopEvent );
    return TRUE;
  }
  return FALSE;
}

int wmain( int argc, wchar_t* argv[], wchar_t* envp[] )
{
#ifndef _DEBUG
  try
  {
#endif
    nil::System* system = new nil::System(
      GetModuleHandleW( nullptr ), GetConsoleWindow() );

    stopEvent = CreateEventW( 0, FALSE, FALSE, 0 );
    SetConsoleCtrlHandler( consoleHandler, TRUE );

    while ( WaitForSingleObject( stopEvent, 250 ) == WAIT_TIMEOUT )
      system->update();

    SetConsoleCtrlHandler( NULL, FALSE );
    CloseHandle( stopEvent );

    delete system;
#ifndef _DEBUG
  }
  catch ( nil::Exception& e )
  {
    wprintf_s( L"Exception: %s\r\n", e.getFullDescription() );
    return EXIT_FAILURE;
  }
  catch ( std::exception& e )
  {
    wprintf_s( L"Exception: %S\r\n", e.what() );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    wprintf_s( L"Unknown exception\r\n" );
    return EXIT_FAILURE;
  }
#endif

  return EXIT_SUCCESS;
}