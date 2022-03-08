/* PCAN-Basic helper functions

   Collection of functions that print TPCAN structures to stdout, look up 
   string representations of constants, and decode DLC values.

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#ifndef _PCAN_HELPER_H_
#define _PCAN_HELPER_H_

#include <stdint.h>      // provide uintX_t

#if defined _WIN32
#include <windows.h>     // provide Win32 types, constants, and functions
#include <PCANBasic.h>   // provide PCAN-Basic types, constants, and functions
#include "pcan_event_win32.h"   // provide threadParameters_t
#elif defined __APPLE__
#include <PCBUSB.h>      // provide PCAN-Basic types, constants, and functions
#include "pcan_helper_darwin.h" // provide TPCANChannelInformation
#include "pcan_event_darwin.h"  // provide threadParameters_t
#endif


// ----------------------------------- // -----------------------------------
// Definitions




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions




// ----------------------------------- // -----------------------------------
// Public functions

// Return pointer to string representation of a PCAN status code, given an
// status code defined in PCANBasic.h
const char *pcanStatusLookup(TPCANStatus status);

// Print the contents of a TPCANMsg structure to stdout in a human-readable
// format for debugging purposes
void pcanDumpMsg(TPCANMsg *msg);

// Print the contents of a TPCANMsgFD structure to stdout in a human-readable
// format for debugging purposes
void pcanDumpMsgFD(TPCANMsgFD *msg);

// Print the contents of a buffer (uint8_t array) to stdout in a human-readable
// format for debugging purposes
void pcanDumpBuffer(BYTE *buffer, size_t bufferLength);

// Return pointer to string representation of a PCAN message type code, given
// a message type code defined in PCANBasic.h
const char *pcanMsgtypeLookup(TPCANMessageType type);

// Print the contents of a TPCANTimestamp structure to stdout in a human-readable
// format for debugging purposes
void pcanDumpTimestamp(TPCANTimestamp *timestamp);

// Print a TPCANTimestampFD (uint64_t) to stdout in a human-readable format
// for debugging purposes
void pcanDumpTimestampFD(TPCANTimestampFD timestamp);

// Return pointer to string representation of a PCAN message parameter code,
// given a parameter code defined in PCANBasic.h
const char *pcanParameterLookup(TPCANParameter parameter);

// Return message size, in bytes, given a DLC code
uint8_t pcanDLCDecode(uint8_t dlc);

// Print the contents of a TPCANChannelInfo structure to stdout
// in a human-readable format for debugging purposes
void pcanDumpChannelInfo(TPCANChannelInformation *info);

// Return pointer to string representation of a PCAN device type,
// given a device type defined in PCANBasic.h
const char *pcanDeviceTypeLookup(TPCANDevice device);

// Return the API constant for a given integer CAN baud (5000 to 1000000), or 0
// for an undefined baud
int pcanTranslateBaud(uint32_t baudInt);


#endif // _PCAN_HELPER_H_

