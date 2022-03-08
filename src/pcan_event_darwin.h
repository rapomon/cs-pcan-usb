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

#include <PCBUSB.h> // provide PCAN-Basic constants and types
#include <pthread.h>


//#define PCAN_EVENT_DARWIN_DEBUG
// Uncomment to enable debugging messages, which may affect timing or
// performance

// ----------------------------------- // -----------------------------------
// Definitions

// Structure of parameters passed from main thread to worker thread
typedef struct threadParameters_s
{
    int canChannel;
    void (*callback) (int);
    int pipeRead;
    int pipeSpawn;
    int pipeExit;
} threadParameters_t;




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions

// Worker thread process, passed to pthread_create by pcanEventEnable
void *pcanEventThreadProc(void *lpParam);




// ----------------------------------- // -----------------------------------
// Public functions

// Enable event signaling data is received on CAN bus
// Accepts callback function that is called when event is signaled:
// void (*callback) (int)
// where the int argument is the CAN channel, which is passed to CAN_Initialize
// and others as TPCANHandle Channel
int pcanEventEnable(TPCANHandle pcan_Channel, void *callback);

// Disable the previously enabled event
int pcanEventDisable(TPCANHandle pcan_Channel);

// Print the contents of a threadParameters_t structure to stdout in a
// human-readable format for debugging purposes
void pcanDumpThreadParameters(threadParameters_t *threadParameters);




#endif /* _PCAN_EVENT_H_ */

