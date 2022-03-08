# cs-pcan-usb Win32 Test Program

This directory contains a test program that only uses the PCAN-Basic library and Win32 API. It initializes the CAN USB adapter, and prints received messages to stdout for 10 seconds before quitting.

As the main module code is updated, this test fall behind and be prone to breakage.

To build main.exe, run `build_main.bat`, which has compiler, include, and library paths hard-coded. If all paths are correct, it will compile the program with `PCAN_NO_NAPI` defined.
