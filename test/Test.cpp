#include "nil.h"
#include "nilUtil.h"
#include <math.h>

HANDLE stopEvent = NULL;

// Cosine wave table for color cycling; not really needed for anything
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

// Shared mouse listener
class DummyMouseListener: public Nil::MouseListener {
public:
  virtual void onMouseMoved( Nil::Mouse* mouse, const Nil::MouseState& state )
  {
    //
  }
  virtual void onMouseButtonPressed( Nil::Mouse* mouse, const Nil::MouseState& state, size_t button )
  {
    printf_s( "Mouse button pressed: %d (%s)\n", (int)button, mouse->getDevice()->getName().c_str() );
  }
  virtual void onMouseButtonReleased( Nil::Mouse* mouse, const Nil::MouseState& state, size_t button )
  {
    printf_s( "Mouse button released: %d (%s)\n", (int)button, mouse->getDevice()->getName().c_str() );
  }
  virtual void onMouseWheelMoved( Nil::Mouse* mouse, const Nil::MouseState& state )
  {
    printf_s( "Mouse wheel moved: %d (%s)\n", state.mWheel.relative, mouse->getDevice()->getName().c_str() );
  }
};

DummyMouseListener gDummyMouseListener;

// Shared keyboard listener
class DummyKeyboardListener: public Nil::KeyboardListener {
public:
  virtual void onKeyPressed( Nil::Keyboard* keyboard, const Nil::VirtualKeyCode keycode )
  {
    printf_s( "Key pressed: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
  virtual void onKeyRepeat( Nil::Keyboard* keyboard, const Nil::VirtualKeyCode keycode )
  {
    printf_s( "Key repeat: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
  virtual void onKeyReleased( Nil::Keyboard* keyboard, const Nil::VirtualKeyCode keycode )
  {
    printf_s( "Key released: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
};

DummyKeyboardListener gDummyKeyboardListener;

// Shared controller listener; 
// A controller is any input device that is not a mouse nor a keyboard
class DummyControllerListener: public Nil::ControllerListener {
public:
  virtual void onControllerButtonPressed( Nil::Controller* controller, const Nil::ControllerState& state, size_t button )
  {
    printf_s( "Controller button %d pressed (%s)\n", (int)button, controller->getDevice()->getName().c_str() );
  }
  virtual void onControllerButtonReleased( Nil::Controller* controller, const Nil::ControllerState& state, size_t button )
  {
    printf_s( "Controller button %d released (%s)\n", (int)button, controller->getDevice()->getName().c_str() );
  }
  virtual void onControllerAxisMoved( Nil::Controller* controller, const Nil::ControllerState& state, size_t axis )
  {
    printf_s( "Controller axis %d moved: %f (%s)\n", (int)axis, state.mAxes[axis].absolute, controller->getDevice()->getName().c_str() );
  }
  virtual void onControllerSliderMoved( Nil::Controller* controller, const Nil::ControllerState& state, size_t slider )
  {
    printf_s( "Controller slider %d moved (%s)\n", (int)slider, controller->getDevice()->getName().c_str() );
  }
  virtual void onControllerPOVMoved( Nil::Controller* controller, const Nil::ControllerState& state, size_t pov )
  {
    printf_s( "Controller POV %d moved: 0x%08X (%s)\n", (int)pov, state.mPOVs[pov].direction, controller->getDevice()->getName().c_str() );
  }
};

DummyControllerListener gDummyControllerListener;

// This is a listener for Logitech's proprietary G-keys on
// support keyboard & mice
class DummyGKeyListener: public Nil::Logitech::GKeyListener {
public:
  virtual void onGKeyPressed( Nil::Logitech::GKey key )
  {
    printf_s( "G-Key pressed: %d\n", key );
  }
  virtual void onGKeyReleased( Nil::Logitech::GKey key )
  {
    printf_s( "G-Key released: %d\n", key );
  }
};

DummyGKeyListener gDummyGKeyListener;

// This is the main system listener, which must always exist
class MyListener: public Nil::SystemListener {
public:
  virtual void onDeviceConnected( Nil::Device* device )
  {
    printf_s( "Connected: %s\r\n", device->getName().c_str() );
    // Enable any device instantly when it is connected
    device->enable();
  }
  virtual void onDeviceDisconnected( Nil::Device* device )
  {
    printf_s( "Disconnected: %s\n", device->getName().c_str() );
  }
  virtual void onMouseEnabled( Nil::Device* device, Nil::Mouse* instance )
  {
    printf_s( "Mouse enabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Add our listener for every mouse that is enabled
    instance->addListener( &gDummyMouseListener );
  }
  virtual void onKeyboardEnabled( Nil::Device* device, Nil::Keyboard* instance )
  {
    printf_s( "Keyboard enabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Add our listener for every keyboard that is enabled
    instance->addListener( &gDummyKeyboardListener );
  }
  virtual void onControllerEnabled( Nil::Device* device, Nil::Controller* instance )
  {
    printf_s( "Controller enabled: %s\r\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\r\n", device->getStaticID() );
    // Add our listener for every controller that is enabled
    instance->addListener( &gDummyControllerListener );
  }
  virtual void onMouseDisabled( Nil::Device* device, Nil::Mouse* instance )
  {
    printf_s( "Mouse disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
  virtual void onKeyboardDisabled( Nil::Device* device, Nil::Keyboard* instance )
  {
    printf_s( "Keyboard disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
  virtual void onControllerDisabled( Nil::Device* device, Nil::Controller* instance )
  {
    printf_s( "Controller disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
};

MyListener gMyListener;

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
    // Create quit event & set handler
    stopEvent = CreateEventW( 0, FALSE, FALSE, 0 );
    SetConsoleCtrlHandler( consoleHandler, TRUE );

    // Init system
    Nil::System* system = new Nil::System(
      GetModuleHandleW( nullptr ),
      GetConsoleWindow(),
      // Using background cooperation mode, because the default console window
      // is actually not owned by our process (it is owned by cmd.exe) and thus
      // we would not receive any mouse & keyboard events in foreground mode.
      // For applications that own their own window foreground mode works fine.
      Nil::Cooperation_Background,
      &gMyListener );

    // Init Logitech G-keys subsystem, if available
    Nil::ExternalModule::InitReturn ret = system->getLogitechGKeys()->initialize();
    if ( ret == Nil::ExternalModule::Initialization_OK )
    {
      printf_s( "G-keys initialized\n" );
      system->getLogitechGKeys()->addListener( &gDummyGKeyListener );
    } else
      printf_s( "G-keys initialization failed with 0x%X\n", ret );

    // Init Logitech LED subsytem, if available
    ret = system->getLogitechLEDs()->initialize();
    if ( ret == Nil::ExternalModule::Initialization_OK )
      printf_s( "LEDs initialized\r\n" );
    else
      printf_s( "LEDs initialization failed with 0x%X\n", ret );

    // Enable all initially connected devices
    for ( auto device : system->getDevices() )
      device->enable();

    // Color cycling helpers
    int x = 0;
    int y = 85;
    int z = 170;

    // "60 fps"
    DWORD timeout = (DWORD)( ( 1.0 / 60.0 ) * 1000.0 );

    // Run main loop
    while ( WaitForSingleObject( stopEvent, timeout ) == WAIT_TIMEOUT )
    {
      // Cycle some LED colors for fun
      if ( system->getLogitechLEDs()->isInitialized() )
      {
        Nil::Color clr;
        clr.r = (float)costable[x] / 99.0f;
        clr.g = (float)costable[y] / 99.0f;
        clr.b = (float)costable[z] / 99.0f;
        system->getLogitechLEDs()->setLighting( clr );
        if ( x++ > 256 ) { x = 0; }
        if ( y++ > 256 ) { y = 0; }
        if ( z++ > 256 ) { z = 0; }
      }
      // Update the system
      // This will trigger all the callbacks
      system->update();
    }

    // Done, shut down
    SetConsoleCtrlHandler( NULL, FALSE );
    CloseHandle( stopEvent );

    delete system;
#ifndef _DEBUG
  }
  catch ( Nil::Exception& e )
  {
    printf_s( "Exception: %s\n", e.getFullDescription().c_str() );
    return EXIT_FAILURE;
  }
  catch ( std::exception& e )
  {
    printf_s( "Exception: %s\n", e.what() );
    return EXIT_FAILURE;
  }
  catch ( ... )
  {
    printf_s( "Unknown exception\n" );
    return EXIT_FAILURE;
  }
#endif

  return EXIT_SUCCESS;
}