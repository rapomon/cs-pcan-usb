===============================================================================
ReadMe.txt

PCAN-Basic V4.4.1.413
Copyright (c) 2020 PEAK-System Technik GmbH Darmstadt, Germany
All rights reserved.
===============================================================================

Maximize the Notepad Window to read this file more easily.


Contents:
---------
  * Introduction
  * System Requirements
  * Rights to use these files
  * Contents of this directory
  * Installation of PCAN hardware
  * How to contact PEAK-System Technik GmbH
  * LIFE SUPPORT APPLIANCES


Introduction
------------
The PCAN system of the company PEAK-System Technik GmbH consists of a
collection of Windows Device Drivers. These allow the Real-time connection of
Windows applications to all CAN busses that are physically connected to the PC
via a PCAN hardware.

PCAN-Basic is a simple programming interface to the PCAN system. Via one
Interface DLL it is possible to connect own applications to the Device drivers
and the PCAN hardware, to communicate with the CAN busses.

The provided drivers, the PCAN-Basic API, and the PCAN-View CAN bus Monitor
software are the feature-reduced versions of the larger software packages
PCAN-Developer or PCAN-Explorer. These can be aquired separately.


Rights to use these files
-------------------------
Please read the End User License Agreement of the company PEAK-System Technik GmbH at:
https://www.peak-system.com/quick/eula

PEAK-System Technik GmbH grants the right to the customer to use the files in
this software package as long as this is done in connection with original
hardware by PEAK-System or OEM hardware coming from PEAK-System. It is NOT
allowed to use any of these files (even not parts) with third-party hardware.

If you are not sure whether you have acquired an appropriate license with the
used hardware, please contact our technical support team (support@peak-system.com).


System Requirements
-------------------
- Operating systems: Windows 10/8.1
  (Driver for Linux could be download at www.peak-system.com/linux)


Contents of this directory
--------------------------
ReadMe.txt
    This text file.

LiesMich.txt
    This text file in German language.

PCANBasic_enu.chm
    The PCAN-Basic documentation in English.

PCANBasic_deu.chm
    The PCAN-Basic documentation in German.

PCAN-Parameter_Documentation.pdf
    Additional documentation about PCAN-Basic Get/Set parameters in English.

\Include
    The Header files for different programming languages and development
    environments.

\Win32
    - A 32-bit test executable file (EXE).
    - The 32-bit interface DLL.

    \BB_LIB
        32-bit LIB file for C++ Builder.

    \VC_LIB
        32-bit LIB file for Visual C/C++.

\x64
    - A 64-bit test executable file (EXE).
    - The 64-bit interface DLL.

    \VC_LIB
        64-bit LIB file for Visual C/C++.


\Samples
    Contains example files that demonstrate the use of the PCAN-Basic API in
    different programming languages and development environments
    (will be enhanced in the future).


Installation of PCAN hardware
-----------------------------
For information about the installation of PCAN hardware, please refer to the
user manual of the respective hardware. The hardware user manuals are located
in the folder <Product CD>\Pdf.

Please keep in mind that none of our setups install the PCANBasic.dll.
If you build an application that uses the PCANBasic.dll, you need to to copy
the PCANBasic.dll manually to your Windows System directory.

Windows 32-bit systems:
32-bit DLL > Windows\System32

Windows 64-bit systems:
32-bit DLL > Windows\SysWOW64
64-bit DLL > Windows\System32


How to contact PEAK-System Technik GmbH
---------------------------------------
If you have any questions concerning the installation of PCAN hardware, or
require information about other PEAK CAN products, then please contact us:

PEAK-System Technik GmbH
Otto-Roehm-Str. 69
D-64293 Darmstadt
Germany

Tel. +49 6151 / 8173-20
FAX  +49 6151 / 8173-29

support@peak-system.com
http://www.peak-system.com


LIFE SUPPORT APPLIANCES
-----------------------
These products are not designed for use in life support appliances, devices,
or systems where malfunction of these products can reasonably be expected to
result in personal injury. PEAK-System customers using or selling these
products for use in such applications do so at their own risk and agree to
fully indemnify PEAK-System for any damages resulting from such improper use
or sale.
