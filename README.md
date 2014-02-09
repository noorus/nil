Nice Input Library
==================

NIL is a gaming input library for Windows, inspired by the Open Input System.  

### Building

NIL is written in C++11, and by default builds as a statically linked x64 library under Visual Studio 2012.  
NIL uses certain APIs from the Windows Driver Kit, so the WDK is required for compiling.

### Technology

NIL utilises three different input APIs to achieve best possible input for different types of devices:
* [Raw Input API](http://msdn.microsoft.com/en-us/library/windows/desktop/ms645543%28v=vs.85%29.aspx) for all mice & keyboard input, for zero lag and no special key weirdness
* [XInput API](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405053%28v=vs.85%29.aspx) for XBOX 360 controller input
* [DirectInput API](http://msdn.microsoft.com/en-us/library/windows/desktop/ee416842%28v=vs.85%29.aspx) for all other (legacy) joysticks & gamepads

NIL can tell apart every input device connected to the computer, including keyboards and mice.  
Multi-keyboard and multi-mice input support is, for once, a breeze.

NIL is fully based on Plug-and-Play support: It knows when devices are connected and disconnected from the computer, and remembers previously-connected devices when reconnected.

Additionally, NIL is capable of receiving direct input from the [G-keys](https://logitech-en-amr.custhelp.com/app/answers/detail/a_id/21506) of certain Logitech Gaming keyboards & mice.

NIL is single-threaded, buffered and fully listener-based.  
Supported platforms are Windows Vista and newer.  
Windows XP is not supported.

### License

NIL is licensed under the **MIT** license.  
For full license text, see the LICENSE file.

Logitech SDKs under the *external* folder are property of Logitech.  
See the Logitech SDK documentation files for licensing specifics.
