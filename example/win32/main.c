/* PCAN-Basic Test program

   Pure Win32 test of the PCAN-Basic library

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#include <stdio.h>       // provide printf
#include <stdint.h>      // provide uintX_t
#include <windows.h>     // provide Win32 API types, constants, functions

#include "PCANBasic.h"   // provide PCAN-Basic types, constants, functions
#include "pcan_event.h"  // provide pcanEnableEvent and pcanDisableEvent
#include "pcan_helper.h" // provide pcanStatusLookup and pcanDumpMsg
#include "main.h"


// ----------------------------------- // -----------------------------------
// Definitions




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions


int main(void)
{
    int channel = 0x51;
    TPCANStatus pcanResult;

    pcanResult = CAN_Initialize(channel, 0x001C, 0, 0, 0);
    printf("CAN_Initialize: 0x%02X (%s)\n", pcanResult, pcanStatusLookup(pcanResult));

    uint32_t pcanAcceptanceCode = 0x81;
    uint32_t pcanAcceptanceMask = 0xF8;

    uint64_t pcanAcceptanceFilter = 0;
    pcanAcceptanceFilter |= ((uint64_t)pcanAcceptanceCode << 0x20);
    pcanAcceptanceFilter |= ((uint64_t)(~pcanAcceptanceMask));

//    printf("pcan_AcceptanceFilter = %016llX\n", pcanAcceptanceFilter);

//    pcanResult = CAN_SetValue(channel, PCAN_ACCEPTANCE_FILTER_29BIT,
//                             &pcanAcceptanceFilter, sizeof(pcanAcceptanceFilter));
//    printf("CAN_SetValue (PCAN_ACCEPTANCE_FILTER_29BIT): 0x%02X (%s)\n",
//         pcanResult, pcanStatusLookup(pcanResult));

    pcanResult = CAN_GetStatus(channel);
    printf("CAN_GetStatus: 0x%02X (%s)\n", pcanResult, pcanStatusLookup(pcanResult));
    
    pcanResult = pcanEventEnable(channel, &callback);
    printf("pcan_EnableEvent: %i \n", pcanResult);

    Sleep(10000); // Run for 10 seconds before quitting

    pcanResult = pcanEventDisable(channel);
    printf("pcan_DisableEvent: %i \n", pcanResult);
    
    pcanResult = CAN_Reset(channel);
    printf("CAN_Reset: 0x%02X (%s)\n", pcanResult, pcanStatusLookup(pcanResult));

    pcanResult = CAN_Uninitialize(channel);
    printf("CAN_Uninitialize: 0x%02X (%s)\n", pcanResult, pcanStatusLookup(pcanResult));

    return 0;
}
    



void callback(int channel)
{
    TPCANStatus status = PCAN_ERROR_OK;
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 };
    
    do {
        ZeroMemory(&msg, sizeof(msg));
        ZeroMemory(&timestamp, sizeof(timestamp));

        status = CAN_Read(channel, &msg, &timestamp);

        if (status == PCAN_ERROR_OK) {
            //printf("---\n");
            pcanDumpMsg(&msg);
            //pcan_dump_timestamp(&timestamp);
        }
    } while (status == PCAN_ERROR_OK);
    
    return;
}

