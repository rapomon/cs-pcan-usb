/* PCAN-Basic event control functions and worker thread process

   Functions that enable and disable the use of an event used by the PCAN-Basic
   library to signal that received data is available for reading. When enabled,
   a worker thread is spawned that waits on this event and invokes a callback
   function when its state changes to signaled, which presumably calls 
   CAN_Read() to retrieve the newly received data.

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

#include <stdio.h>       // provide printf
#include <stdbool.h>     // provide boolean values
#include <windows.h>     // provide Win32 API constants and types
#include <PCANBasic.h>   // provide PCAN-Basic constants and types

#include "pcan.h"        // provide default callback function
#include "pcan_event_win32.h"
#include "pcan_helper.h" // provide pcanStatusLookup


// ----------------------------------- // -----------------------------------
// Definitions

// Names used both in CreateEvent calls in main thread and OpenEvent calls
// in worker thread
const char pcanEventReadName[] = "pcanEventRead";
const char pcanEventSpawnName[] = "pcanEventSpawn";
const char pcanEventExitName[] = "pcanEventExit";


// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables

// Handle for CAN bus read event, signaled my main thread (PCAN-Basic API) when
// a message has been received and waited on by worker thread
HANDLE pcanEventRead = INVALID_HANDLE_VALUE;

// Handle for worker thread spawn event, signaled by worker shortly after thread
// starts and waited on by main thread
HANDLE pcanEventSpawn = INVALID_HANDLE_VALUE;

// Handle for worker thread exit event, signaled by main thread during exit
// routine, and waited on by worker thread
HANDLE pcanEventExit = INVALID_HANDLE_VALUE;

// Worker thread handle, populated by pcanEventEnable
HANDLE pcanEventThread = INVALID_HANDLE_VALUE;

// Worker thread ID, populated by pcanEventEnable
DWORD pcanEventThreadID = 0;




// ----------------------------------- // -----------------------------------
// Local functions


DWORD WINAPI pcanEventThreadProc(LPVOID lpParam)
{
    int ret = 0;

    // Copy parameters passed from the main thread
    threadParameters_t threadParams = { 0 };
    memcpy(&threadParams, (threadParameters_t*)lpParam, sizeof(threadParams));

#ifdef PCAN_EVENT_WIN32_DEBUG
    pcanDumpThreadParameters(&threadParams);
#endif

    // Open thread-local handles for events
    HANDLE pcanEventRead = INVALID_HANDLE_VALUE;
    HANDLE pcanEventSpawn = INVALID_HANDLE_VALUE;
    HANDLE pcanEventExit = INVALID_HANDLE_VALUE;
    
    pcanEventRead = OpenEvent(EVENT_PERMISSIONS, // dwDesiredAccess
                              false, // bInheritHandle
                              pcanEventReadName);

    if (pcanEventRead == 0)
    {
        printf("pcanEventThreadProc: Error at OpenEvent(pcanEventRead): %i\n",
               GetLastError());
        return 1;
    }

    pcanEventSpawn = OpenEvent(EVENT_PERMISSIONS, // dwDesiredAccess
                               false, // bInheritHandle
                               pcanEventSpawnName);

    if (pcanEventSpawn == 0)
    {
        printf("pcanEventThreadProc: Error at OpenEvent(pcanEventSpawn): %i\n",
               GetLastError());
        return 1;
    }

    pcanEventExit = OpenEvent(EVENT_PERMISSIONS, // dwDesiredAccess
                              false, // bInheritHandle
                              pcanEventExitName);

    if (pcanEventExit == 0)
    {
        printf("pcanEventThreadProc: Error at OpenEvent(pcanEventExit): %i\n",
               GetLastError());
        return 1;
    }

    // Signal the spawn event, indicating that the worker thread is running
    ret = SetEvent(pcanEventSpawn);
    if (ret == 0)
    {
        printf("pcanEventThreadProc: Error at SetEvent: 0x%02X\n", GetLastError());
        return 1;
    }
    
    // Wait for either event to be signaled
    int threadExit = 0;
    HANDLE events[2] = { 0 };
    int eventIndex = 0;
    
    events[EVENT_INDEX_READ] = pcanEventRead;
    events[EVENT_INDEX_EXIT] = pcanEventExit;

    while (threadExit == 0) {
#ifdef PCAN_EVENT_WIN32_DEBUG
        printf("pcanEventThreadProc: Waiting for pcanEventRead or "
               "pcanEventExit...\n");
#endif
        ret = WaitForMultipleObjects(2, // nCount
                                     events, // lpHandles
                                     false, // bWaitAll
                                     INFINITE); // dwMilliseconds

        if (ret & WAIT_ABANDONED_0)
        {
            eventIndex = (ret ^ WAIT_ABANDONED_0);
            printf("pcanEventThreadProc: WAIT_ABANDONED_0: %i\n", eventIndex);
        }
        else if (ret == WAIT_TIMEOUT)
        {
            printf("pcanEventThreadProc: WAIT_TIMEOUT\n");
            return 1;
        }
        else if (ret == WAIT_FAILED)
        {
            printf("pcanEventThreadProc: WAIT_FAILED: %i\n", GetLastError());
            return 1;
        }
        else
        {
            // ret is (WAIT_OBJECT_0 | event_index), and WAIT_OBJECT_0 = 0,
            switch (ret)
            {
            case EVENT_INDEX_READ: // Execute callback
                (threadParams.callback)(threadParams.canChannel);
                break;
            case EVENT_INDEX_EXIT: // Clean up and exit thread
                threadExit = 1;
            }
        }
    }

#ifdef PCAN_EVENT_WIN32_DEBUG
    printf("pcanEventThreadProc: Exiting thread\n");
#endif
    CloseHandle(pcanEventRead);
    CloseHandle(pcanEventSpawn);
    CloseHandle(pcanEventExit);

    return 0;
}




// ----------------------------------- // -----------------------------------
// Public functions


int pcanEventEnable(TPCANHandle pcanChannel, void *callback)
{
    int ret = 0;

    // Create security attributes structure for events and worker thread
    SECURITY_DESCRIPTOR pcanSecurityDesc = { 0 };

    ret = InitializeSecurityDescriptor(&pcanSecurityDesc,
                                       SECURITY_DESCRIPTOR_REVISION);
    if (ret == 0)
    {
        printf("pcanEventEnable: Error at InitializeSecurityDescriptor: %i\n",
               GetLastError());
        return 1;
    }
    
    ret = SetSecurityDescriptorDacl(&pcanSecurityDesc,
                                    true,
                                    0,
                                    false);
    if (ret == 0)
    {
        printf("pcanEventEnable: Error at SetSecurityDescriptorDacl: %i\n",
               GetLastError());
        return 1;
    }

    SECURITY_ATTRIBUTES pcanSecurityAttr = { 0 };
    pcanSecurityAttr.nLength = sizeof(pcanSecurityAttr);
    pcanSecurityAttr.lpSecurityDescriptor = &pcanSecurityDesc;
    pcanSecurityAttr.bInheritHandle = true;

    // Create event object that the PCANBasic API will use to communicate
    // CAN read events with worker thread
    pcanEventRead = CreateEvent(&pcanSecurityAttr, // lpEventAttributes
                                false, // bManualReset
                                false, // bInitialState
                                "pcanEventRead");

    if (pcanEventRead == 0)
    {
        printf("pcanEventEnable: Error at CreateEvent (pcanEventRead): 0x%02X\n",
               GetLastError());
        return 1;
    }

    // Create event object that worker thread will use to indicate it has started
    pcanEventSpawn = CreateEvent(&pcanSecurityAttr, // lpEventAttributes
                                 false, // bManualReset
                                 false, // bInitialState
                                 "pcanEventSpawn");

    if (pcanEventSpawn == 0)
    {
        printf("pcanEventEnable: Error at CreateEvent (pcanEventSpawn): 0x%02X\n",
               GetLastError());
        return 1;
    }

    // Create event object that main thread will use to stop worker thread
    pcanEventExit = CreateEvent(&pcanSecurityAttr, // lpEventAttributes
                                false, // bManualReset
                                false, // bInitialState
                                "pcanEventExit");

    if (pcanEventExit == 0)
    {
        printf("pcanEventEnable: Error at CreateEvent (pcanEventExit): 0x%02X\n",
               GetLastError());
        return 1;
    }

    // Pack parameters into struct to be passed to worker thread
    threadParameters_t pcanEventThreadParams = {
        pcanChannel,
        callback
    };

    // Spawn the worker thread that will monitor the event 
    pcanEventThread = CreateThread(&pcanSecurityAttr, //lpThreadAttributes
                                   0, // dwStackSize
                                   &pcanEventThreadProc, // lpStartAddress
                                   &pcanEventThreadParams, // lpParameter
                                   0, // dwCreationFlags
                                   &pcanEventThreadID); // lpThreadId

    if (pcanEventThread == 0)
    {
        printf("pcanEventEnable: Error at CreateThread: 0x%02X\n", GetLastError());
        return 1;
    }

    // Wait for pcanEventSpawn to be set, indicating that the thread
    // was successfully started
#ifdef PCAN_EVENT_WIN32_DEBUG
    printf("pcanEventEnable: Waiting for worker thread to start...\n");
#endif

    ret = WaitForSingleObject(pcanEventSpawn, 1000);
    switch (ret)
    {
    case WAIT_OBJECT_0:
        break;
    case WAIT_ABANDONED:
        printf("pcanEventEnable: Error at WaitForSingleObject: WAIT_ABANDONED\n");
        return 1;
    case WAIT_TIMEOUT:
        printf("pcanEventEnable: Error at WaitForSingleObject: WAIT_TIMEOUT\n");
        return 1;
    case WAIT_FAILED:
        printf("pcanEventEnable: Error at WaitForSingleObject: WAIT_FAILED: 0x%02X\n",
               GetLastError());
        return 1;
    default:
        printf("pcanEventEnable: Error at WaitForSingleObject: ret = 0x%02X\n", ret);
        return 1;
    }

    // Set the PCAN library to use the receive event
    TPCANStatus pcanResult = 0;

    pcanResult = CAN_SetValue(pcanChannel, PCAN_RECEIVE_EVENT,
                              &pcanEventRead, sizeof(pcanEventRead));
    
    if (pcanResult != PCAN_ERROR_OK)
    {
        printf("pcanEventEnable: Error at CAN_SetValue: 0x%02X (%s)\n",
               pcanResult, pcanStatusLookup(pcanResult));
        return 1;
    }

    return 0;
}




int pcanEventDisable(TPCANHandle pcan_Channel)
{
    int ret = 0;

    // Disable event in PCAN library
    HANDLE dummy_event = INVALID_HANDLE_VALUE;
    TPCANStatus pcanResult = PCAN_ERROR_UNKNOWN;

    pcanResult = CAN_SetValue(pcan_Channel, PCAN_RECEIVE_EVENT,
                              &dummy_event, sizeof(dummy_event));

    // Stop the worker thread
    ret = SetEvent(pcanEventExit);
    if (ret == 0)
    {
        printf("pcanEventDisable: Error at SetEvent: 0x%02X\n",
               GetLastError());
        return 1;
    }

    // Wait for the worker thread to exit
    int pcanEventThreadExitCode = 0;
    
#ifdef PCAN_EVENT_WIN32_DEBUG
    printf("pcanEventDisable: Waiting for worker thread to exit...\n");
#endif

    ret = WaitForSingleObject(pcanEventThread, INFINITE);

    switch (ret)
    {
    case WAIT_OBJECT_0:
        ret = GetExitCodeThread(pcanEventThread, &pcanEventThreadExitCode);

        if (ret == 0)
        {
            printf("pcanEventDisable: Error at GetExitCodeThread: 0x%02X\n",
                   GetLastError());
            return 1;
        }

        if (pcanEventThreadExitCode == STILL_ACTIVE)
        {
            printf("pcanEventDisable: Thread still active.\n");
            return 1;
        }
        else
        {
#ifdef PCAN_EVENT_WIN32_DEBUG
            printf("pcanEventDisable: Worker thread exited with code %i\n",
                   pcanEventThreadExitCode);
#endif
        }
        break;
    case WAIT_ABANDONED:
        printf("pcanEventDisable: Error at WaitForSingleObject: WAIT_ABANDONDED\n");
        return 1;
    case WAIT_TIMEOUT:
        printf("pcanEventDisable: Error at WaitForSingleObject: WAIT_TIMEOUT\n");
        return 1;
    case WAIT_FAILED:
        printf("pcanEventDisable: Error at WaitForSingleObject: WAIT_FAILED: 0x%02X\n",
               GetLastError());
        return 1;
    default:
        printf("pcanEventDisable: Error at WaitForSingleObject: ret = 0x%02X\n", ret);
        return 1;
    }

    // Close all thread and event handles
    ret = CloseHandle(pcanEventRead);
    if (ret == 0)
    {
        printf("pcanEventDisable: Error at CloseHandle(pcanEventRead): %i\n",
               GetLastError());
        return 1;
    }

    ret = CloseHandle(pcanEventSpawn);
    if (ret == 0)
    {
        printf("pcanEventDisable: Error at CloseHandle(pcanEventSpawn): %i\n",
               GetLastError());
        return 1;
    }

    ret = CloseHandle(pcanEventExit);
    if (ret == 0)
    {
        printf("pcanEventDisable: Error at CloseHandle(pcanEventExit): %i\n",
               GetLastError());
        return 1;
    }

    ret = CloseHandle(pcanEventThread);
    if (ret == 0)
    {
        printf("pcanEventDisable: Error at CloseHandle(pcanEventThread): %i\n",
               GetLastError());
        return 1;
    }

    return 0;
}




void pcanDumpThreadParameters(threadParameters_t *threadParameters)
{
    printf("threadParameters {\n"
           "  canChannel  = 0x%02X\n"
           "  callback    = 0x%p\n"
           "}\n",
           threadParameters->canChannel,
           threadParameters->callback);

    return;
}
