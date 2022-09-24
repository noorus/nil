Nice Input Library
==================
NIL is a gaming input library written in C++.  
It is somewhat battle-tested, but currently implemented for Windows only.

### Platform

Current version tested on Windows 10 and newer.  
Previous generation (before 2019) was tested against Windows Vista.  
Should work fine on Windows 8, possibly on Windows 7, with few modifications if any.

### Building

1. Open `nil.sln`
2. Press Build

By default NIL builds as a statically linked library under Visual Studio 2022.  
Clang-CL should also work fine, probably others, as long as some C++20 features are supported.  
NIL has **No dependencies**, except for the Windows SDK & Driver Kit.  

### Features

* Full multi-keyboard and multi-mice support. Every connected input device has a unique ID.
* Plug-and-Play device runtime connection & disconnection detection.
* Singlethreaded, buffered and listener-based.
* Uses [Raw Input](http://msdn.microsoft.com/en-us/library/windows/desktop/ms645543%28v=vs.85%29.aspx) for mice & keyboards, [XInput](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405053%28v=vs.85%29.aspx) for XBOX and emulated controllers, and [DirectInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ee416842%28v=vs.85%29.aspx) for old-school gamepads.
* No force feedback support. Too cumbersome to implement in a smartly abstracted way. If you're feeling up to the task, however, pull requests are welcome.

### Pitfalls

See the [PITFALLS.md](PITFALLS.md) file for information on some potential implementation pitfalls.

### License

NIL is licensed under the **MIT** license.  
For full license text, see the LICENSE file.

Logitech SDKs under the *external* folder are property of Logitech.  
See the Logitech SDK documentation files for licensing specifics.
