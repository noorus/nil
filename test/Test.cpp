#include "nil.h"
#include "nilUtil.h"
#include <math.h>

HANDLE stopEvent = NULL;

BYTE costable[256]={
  0,0,0,0,0,0,0,0,0,1,1,1,2,2,2,3,
  3,4,4,5,5,6,7,7,8,9,9,10,11,12,13,13,
  14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
  31,32,33,34,35,36,38,39,40,41,42,44,45,46,47,49,
  50,51,52,53,55,56,57,58,60,61,62,63,64,66,67,68,
  69,70,71,72,73,74,76,77,78,79,80,81,82,82,83,84,
  85,86,87,88,88,89,90,91,91,92,93,93,94,94,95,95,
  96,96,97,97,98,98,98,98,99,99,99,99,99,99,99,99,
  99,99,99,99,99,99,99,99,98,98,98,98,97,97,96,96,
  95,95,94,94,93,93,92,91,91,90,89,88,88,87,86,85,
  84,83,82,82,81,80,79,78,77,76,75,73,72,71,70,69,
  68,67,66,64,63,62,61,60,58,57,56,55,53,52,51,50,
  49,47,46,45,44,42,41,40,39,38,36,35,34,33,32,31,
  29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,
  13,13,12,11,10,9,9,8,7,7,6,5,5,4,4,3,
  3,2,2,2,1,1,1,0,0,0,0,0,0,0,0,0
};

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

    nil::ExternalModule::InitReturn ret = system->getLogitechGKeys()->initialize();
    if ( ret == nil::ExternalModule::Initialization_OK )
      wprintf_s( L"G-keys initialized\r\n" );
    else
      wprintf_s( L"G-keys initialization failed with 0x%X\r\n", ret );

    ret = system->getLogitechLEDs()->initialize();
    if ( ret == nil::ExternalModule::Initialization_OK )
      wprintf_s( L"LEDs initialized\r\n" );
    else
      wprintf_s( L"LEDs initialization failed with 0x%X\r\n", ret );

    stopEvent = CreateEventW( 0, FALSE, FALSE, 0 );
    SetConsoleCtrlHandler( consoleHandler, TRUE );

    int x = 0;
    int y = 85;
    int z = 170;

    // "60 fps"
    DWORD timeout = (DWORD)( ( 1.0 / 60.0 ) * 1000.0 );

    while ( WaitForSingleObject( stopEvent, timeout ) == WAIT_TIMEOUT )
    {
      // Cycle some LED colors for fun
      if ( system->getLogitechLEDs()->isInitialized() )
      {
        nil::Color clr;
        clr.r = (float)costable[x] / 99.0f;
        clr.g = (float)costable[y] / 99.0f;
        clr.b = (float)costable[z] / 99.0f;
        system->getLogitechLEDs()->setLighting( clr );
        if ( x++ > 256 ) { x = 0; }
        if ( y++ > 256 ) { y = 0; }
        if ( z++ > 256 ) { z = 0; }
      }
      // Update the system
      system->update();
    }

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