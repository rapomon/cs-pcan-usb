/* PCAN-Basic event control functions and worker thread process

   Functions that enable and disable the use of an event used by the PCAN-Basic
   library to signal that received data is available for reading. When enabled,
   a worker thread is spawned that waits on this event and invokes a callback
   function when its state changes to signaled, which presumably calls 
   CAN_Read() to retrieve the newly received data.

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

#ifndef _PCAN_EVENT_H_
#define _PCAN_EVENT_H_

#include <windows.h>   // provide Win32 API constants and types
#include <PCANBasic.h> // provide PCAN-Basic constants and types


//#define PCAN_EVENT_WIN32_DEBUG
// Uncomment to enable debugging messages, which may affect timing or
// performance

// ----------------------------------- // -----------------------------------
// Definitions

// Structure of parameters passed from main thread to worker thread
typedef struct threadParameters_s
{
    int canChannel;
    void (*callback) (int);
} threadParameters_t;

// Security access requested for event objects and thread
#define EVENT_PERMISSIONS (EVENT_MODIFY_STATE | SYNCHRONIZE)

// Indices of events when packed into struct passed to WaitForMultipleObjects
// in pcanEventThreadProc
#define EVENT_INDEX_READ   (0)
#define EVENT_INDEX_EXIT   (1)




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions

// Worker thread process, passed to CreateThread by pcanEventEnable
DWORD WINAPI pcanEventThreadProc(LPVOID lpParam);




// ----------------------------------- // -----------------------------------
// Public functions

// Enable Win32 event signaling data is received on CAN bus
// Accepts callback function that is called when event is signaled:
// void (*callback) (int)
// where the int argument is the CAN channel, which is passed to CAN_Initialize
// and others as TPCANHandle Channel
int pcanEventEnable(TPCANHandle pcan_Channel, void *callback);

// Disable the previously enabled Win32 event
int pcanEventDisable(TPCANHandle pcan_Channel);

// Print the contents of a threadParameters_t structure to stdout in a
// human-readable format for debugging purposes, for Windows
void pcanDumpThreadParameters(threadParameters_t *threadParameters);


#endif /* _PCAN_EVENT_H_ */

