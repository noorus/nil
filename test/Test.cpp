#include "nil.h"
#include "nilUtil.h"
#include <cmath>

HANDLE stopEvent = nullptr;

// #define NIL_TESTAPP_IGNORE_MOUSE

// Shared mouse listener
class DummyMouseListener: public nil::MouseListener {
public:
  void onMouseMoved( nil::Mouse* mouse, const nil::MouseState& state ) override
  {
    //
  }
  void onMouseButtonPressed( nil::Mouse* mouse, const nil::MouseState& state, size_t button ) override
  {
#   ifndef NIL_TESTAPP_IGNORE_MOUSE
      printf_s( "Mouse button pressed: %d (%s)\n", (int)button, mouse->getDevice()->getName().c_str() );
    #endif
  }
  void onMouseButtonReleased( nil::Mouse* mouse, const nil::MouseState& state, size_t button ) override
  {
#   ifndef NIL_TESTAPP_IGNORE_MOUSE
      printf_s( "Mouse button released: %d (%s)\n", (int)button, mouse->getDevice()->getName().c_str() );
#   endif
  }
  void onMouseWheelMoved( nil::Mouse* mouse, const nil::MouseState& state ) override
  {
    #ifndef NIL_TESTAPP_IGNORE_MOUSE
      printf_s( "Mouse wheel moved: %d (%s)\n", state.wheel.relative, mouse->getDevice()->getName().c_str() );
#   endif
  }
};

DummyMouseListener g_dummyMouseListener;

// Shared keyboard listener
class DummyKeyboardListener: public nil::KeyboardListener {
public:
  void onKeyPressed( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override
  {
    printf_s( "Key pressed: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
  void onKeyRepeat( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override
  {
    printf_s( "Key repeat: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
  void onKeyReleased( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override
  {
    printf_s( "Key released: 0x%X (%s)\n", keycode, keyboard->getDevice()->getName().c_str() );
  }
};

DummyKeyboardListener g_dummyKeyboardListener;

// Shared controller listener;
// A controller is any input device that is not a mouse nor a keyboard
class DummyControllerListener: public nil::ControllerListener {
public:
  void onControllerButtonPressed( nil::Controller* controller, const nil::ControllerState& state, size_t button ) override
  {
    printf_s( "Controller button %d pressed (%s)\n", (int)button, controller->getDevice()->getName().c_str() );
  }
  void onControllerButtonReleased( nil::Controller* controller, const nil::ControllerState& state, size_t button ) override
  {
    printf_s( "Controller button %d released (%s)\n", (int)button, controller->getDevice()->getName().c_str() );
  }
  void onControllerAxisMoved( nil::Controller* controller, const nil::ControllerState& state, size_t axis ) override
  {
    printf_s( "Controller axis %d moved: %f (%s)\n", (int)axis, state.axes[axis].absolute, controller->getDevice()->getName().c_str() );
  }
  void onControllerSliderMoved( nil::Controller* controller, const nil::ControllerState& state, size_t slider ) override
  {
    printf_s( "Controller slider %d moved (%s)\n", (int)slider, controller->getDevice()->getName().c_str() );
  }
  void onControllerPOVMoved( nil::Controller* controller, const nil::ControllerState& state, size_t pov ) override
  {
    printf_s( "Controller POV %d moved: 0x%08X (%s)\n", (int)pov, state.povs[pov].direction, controller->getDevice()->getName().c_str() );
  }
};

DummyControllerListener g_dummyControllerListener;

// This is the main system listener, which must always exist
class MyListener: public nil::SystemListener {
public:
  void onDeviceConnected( nil::Device* device ) override
  {
    printf_s( "Connected: %s\r\n", device->getName().c_str() );
    // Enable any device instantly when it is connected
    device->enable();
  }
  void onDeviceDisconnected( nil::Device* device ) override
  {
    printf_s( "Disconnected: %s\n", device->getName().c_str() );
  }
  void onMouseEnabled( nil::Device* device, nil::Mouse* instance ) override
  {
    printf_s( "Mouse enabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Add our listener for every mouse that is enabled
    instance->addListener( &g_dummyMouseListener );
  }
  void onKeyboardEnabled( nil::Device* device, nil::Keyboard* instance ) override
  {
    printf_s( "Keyboard enabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Add our listener for every keyboard that is enabled
    instance->addListener( &g_dummyKeyboardListener );
  }
  void onControllerEnabled( nil::Device* device, nil::Controller* instance ) override
  {
    printf_s( "Controller enabled: %s\r\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\r\n", device->getStaticID() );
    // Add our listener for every controller that is enabled
    instance->addListener( &g_dummyControllerListener );
  }
  void onMouseDisabled( nil::Device* device, nil::Mouse* instance ) override
  {
    printf_s( "Mouse disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
  void onKeyboardDisabled( nil::Device* device, nil::Keyboard* instance ) override
  {
    printf_s( "Keyboard disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
  void onControllerDisabled( nil::Device* device, nil::Controller* instance ) override
  {
    printf_s( "Controller disabled: %s\n", device->getName().c_str() );
    printf_s( "Static ID: 0x%08X\n", device->getStaticID() );
    // Removing listeners at this point is unnecessary,
    // as the device instance is destroyed anyway
  }
};

MyListener g_myListener;

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
    if ( !stopEvent )
      return EXIT_FAILURE;
    SetConsoleCtrlHandler( consoleHandler, TRUE );

    // Init system
    auto system = nil::System::create(
      GetModuleHandleW( nullptr ),
      GetConsoleWindow(),
      // Using background cooperation mode, because the default console window
      // is actually not owned by our process (it is owned by cmd.exe) and thus
      // we would not receive any mouse & keyboard events in foreground mode.
      // For applications that own their own window foreground mode works fine.
      nil::Cooperation::Background,
      &g_myListener );

    // Initialize
    system->initialize();

    // Enable all initially connected devices
    for ( auto& device : system->getDevices() )
      device->enable();

    // "60 fps"
    auto timeout = (DWORD)( ( 1.0 / 60.0 ) * 1000.0 );

    // Run main loop
    while ( WaitForSingleObject( stopEvent, timeout ) == WAIT_TIMEOUT )
    {
      // Update the system
      // This will trigger all the callbacks
      system->update();
    }

    // Done, shut down
    SetConsoleCtrlHandler( nullptr, FALSE );
    CloseHandle( stopEvent );
#ifndef _DEBUG
  }
  catch ( nil::Exception& e )
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