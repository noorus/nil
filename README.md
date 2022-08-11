Nice Input Library
==================
NIL is a gaming input library written in C++. It is somewhat battle-tested, but currently implemented for Windows only.

### Platform

Tested against Windows 10 and newer. Previous generation (before 2019) was tested against Windows Vista.  
Should work fine on Windows 8, possibly on Windows 7, with few modifications.

### Building

NIL is compiled with C++20, and by default builds as a statically linked library under Visual Studio 2022.  
It uses certain APIs from the Windows Driver Kit. WDK is nowadays included in the Windows SDK.

### Features

* Full multi-keyboard and multi-mice support. Every connected input device has a unique ID.
* Plug-and-Play device runtime addition & removal support.
* Singlethreaded, buffered and listener-based.
* Uses [Raw Input](http://msdn.microsoft.com/en-us/library/windows/desktop/ms645543%28v=vs.85%29.aspx) for mice & keyboards, [XInput](http://msdn.microsoft.com/en-us/library/windows/desktop/hh405053%28v=vs.85%29.aspx) for XBOX and emulated controllers, and [DirectInput](http://msdn.microsoft.com/en-us/library/windows/desktop/ee416842%28v=vs.85%29.aspx) for old-school gamepads.
* Optional support for [Logitech G-keys](https://logitech-en-amr.custhelp.com/app/answers/detail/a_id/21506), which are extra buttons on some Logitech Gaming keyboards.
* NO force feedback support. Not categorically against them, but not going to spend time on it.

### Pitfalls

See the [PITFALLS.md](PITFALLS.md) file for information on some potential implementation pitfalls.

### License

NIL is licensed under the **MIT** license.  
For full license text, see the LICENSE file.

Logitech SDKs under the *external* folder are property of Logitech.  
See the Logitech SDK documentation files for licensing specifics.
