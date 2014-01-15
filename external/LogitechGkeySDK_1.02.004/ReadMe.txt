Logitech Gaming G-key SDK
Copyright (C) 2012 Logitech Inc. All Rights Reserved


Introduction
--------------------------------------------------------------------------

This package enables developers to easily add support in their games
for checking G-key and extra mouse button presses on compatible Logitech 
gaming mice and keyboards.


Contents of the package
--------------------------------------------------------------------------

- Logitech Gaming G-key SDK header, libs and dlls, 32 and 64 bit
- Demo executable
- Documentation
- Sample program using the SDK


The environment for use of the package
--------------------------------------------------------------------------

Visual Studio 2008 or 2010 to build and run the sample program


List of currently supported devices
--------------------------------------------------------------------------

Mice

- G300
- G400
- G600

Keyboards

- G11
- G13
- G15 v1
- G15 v2
- G103
- G105
- G105 Call Of Duty
- G110
- G510
- G19
- G710

Headsets:

- G35
- G930


Disclaimer
--------------------------------------------------------------------------

This is work in progress. If you find anything wrong with either
documentation or code, please let us know so we can improve on it.


Where to start
--------------------------------------------------------------------------

For a demo program to change lighting on devices:

Execute Demo/DisplayGkeys.exe.

Or:

1. Go to Samples/DisplayGkeys folder and open the project in
   Visual Studio (DisplayGkeys.sln for VS2010, DisplayGkeys2008.sln for
   VS2008).

2. Compile and run.

3. Plug in one or multiple compatible mice or keyboards at any time.


To implement game controller support in your game:

1. Include the following header file in your game:

- Include/LogitechGkey.h

2. Include the following library and dll in your game:

- Lib\x86\LogitechGkey.lib and Lib\x86\LogitechGkey.dll or
- Lib\x64\LogitechGkey.lib and Lib\x64\LogitechGkey.dll

3. Alternatively you may only add the LogitechGkey.dll to your game and use
   GetProcAddress to access its functions (see sample program).

4. Read and follow instructions from Doc/LogitechGamingGkeySDK.pdf


For questions/problems/suggestions email to:
cjuncker@logitech.com
vtucker@logitech.com
