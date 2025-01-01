Nice Input Library
==================
NIL is a gaming input library written in C++.  
It is somewhat battle-tested, but currently implements Windows support only.

### Platform

Tested on Windows 10 and newer.  
Previous generation (before 2019) was tested on Windows Vista.  
Should work fine on Windows 8, possibly on Windows 7, with few modifications if any.

### Building

1. Open `nil.sln`
2. Press Build

By default NIL builds as a statically linked library under Visual Studio 2022.  
Clang-CL and others should work fine as long as some C++20 features are supported.  
NIL has **no dependencies** other than the Windows SDK (and the DDK on very old installations.)

### Features

* Full multi-keyboard and multi-mice support. Every connected input device has a unique ID.
* Plug-and-Play device runtime connection & disconnection detection.
* Singlethreaded, buffered and listener-based.
* Uses [Raw Input](http://msdn.microsoft.com/en-us/library/windows/desktop/ms645543%28v=vs.85%29.aspx) for mice & keyboards, [XInput](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405053%28v=vs.85%29.aspx) for XBOX and emulated controllers, and [DirectInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ee416842%28v=vs.85%29.aspx) for old-school gamepads.

### Not-features
* Force feedback is not implemented, because it is hard to abstract in a sensible way between the different APIs. But if you're feeling up to the task, pull requests are welcome.

### Pitfalls

See the [PITFALLS.md](PITFALLS.md) file for information on some potential implementation pitfalls.

### License

NIL is licensed under the **MIT** license.  
For full license text, see the LICENSE file.
