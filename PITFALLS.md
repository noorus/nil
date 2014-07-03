There are a few possible pitfalls to make note of when using Nil on Windows:

* The XInput API is not buffered or event-based, it is poll-only. If you don't update the system fast enough, you could miss entire button presses on the XBOX gamepads. At least 30 FPS is recommended.
* The RawInput API only supports up to five buttons per mouse. I don't have a mouse with more than five native buttons, so it is to be investigated how such could be supported in Nil.
* Using the background cooperation mode (that is, global input) could bring up warnings from antivirus software when using your program. This is because global keyboard input can be used to implement keyloggers. I suggest sticking to foreground cooperation mode only, unless you really need global input.

Note that these pitfalls are not unique to Nil in any way, but rather are due to limitations in the underlying APIs. All other input systems suffer from the same limitations and problems.
