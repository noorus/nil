#include "nil.h"
#include "nilUtil.h"

HANDLE stopEvent = NULL;

class StupidListener: public nil::ControllerListener {
public:
  virtual void onButtonPressed( const nil::ControllerState& state, size_t button )
  {
    wprintf_s( L"Button %d pressed\r\n", button );
  }
  virtual void onButtonReleased( const nil::ControllerState& state, size_t button )
  {
    wprintf_s( L"Button %d released\r\n", button );
  }
  virtual void onAxisMoved( const nil::ControllerState& state, size_t axis )
  {
    wprintf_s( L"Axis %d moved: %f\r\n", axis, state.mAxes[axis].absolute );
  }
  virtual void onSliderMoved( const nil::ControllerState& state, size_t slider )
  {
    wprintf_s( L"Slider %d moved: \r\n", slider );
  }
  virtual void onPOVMoved( const nil::ControllerState& state, size_t pov )
  {
    wprintf_s( L"POV %d moved: \r\n", pov );
  }
};

StupidListener gStupidListener;

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

    DWORD timeout = (DWORD)( ( 1.0f / 60.0f ) * 1000.0f );

    while ( WaitForSingleObject( stopEvent, timeout ) == WAIT_TIMEOUT )
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