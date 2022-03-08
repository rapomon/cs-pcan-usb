/* N-API interface for PEAK-System PCAN-Basic

   N-API callable functions that directly correspond to PCAN-Basic functions, 
   and additional wrapper functions

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/
   

#include <assert.h>      // provide assert
#include <node_api.h>    // provide N-API types, constants, and functions
#include <stdio.h>       // provide printf
#include <string.h>      // provide memcpy
#include <stdlib.h>      // provide malloc and free

#if defined _WIN32
#include <windows.h>     // provide Win32 types, constants, and functions
#include <PCANBasic.h>   // provide PCAN-Basic types, constants, and functions
#include "pcan_event_win32.h"  // provide pcanEventEnable and pcanEventDisable
#elif defined __APPLE__
#include <PCBUSB.h>      // provide PCAN-Basic types, constants, and functions
#include "pcan_event_darwin.h" // provide pcanEventEnable and pcanEventDisable
#endif

#include "pcan.h"
#include "pcan_helper.h" // provide pcanDLCDecode
#include "napi_helper.h" // provide DECLARE_NAPI_METHOD


// ----------------------------------- // -----------------------------------
// Definitions


// Length of N-API module descriptor list, used in Init
enum { descriptors_len = 19 };




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables

// Data read callback function, created in main thread and called from worker
// thread
napi_threadsafe_function pcanCallback = { 0 };




// ----------------------------------- // -----------------------------------
// Local functions


// Register N-API module
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);




napi_value Init(napi_env env, napi_value exports)
{
    napi_status status = napi_generic_failure;

    // Turn off buffering on stdout
    setvbuf(stdout, NULL, _IONBF, 0);

    // Build array of property descriptors
    // If modifying this list, remember to update descriptors_len, above 
    napi_property_descriptor descriptors[descriptors_len] = {
        // PCAN-Basic functions
        DECLARE_NAPI_METHOD("Initialize", pcan_CAN_Initialize),
        DECLARE_NAPI_METHOD("InitializeFD", pcan_CAN_InitializeFD),
        DECLARE_NAPI_METHOD("Uninitialize", pcan_CAN_Uninitialize),
        DECLARE_NAPI_METHOD("Reset", pcan_CAN_Reset),
        DECLARE_NAPI_METHOD("GetStatus", pcan_CAN_GetStatus),
        DECLARE_NAPI_METHOD("Read", pcan_CAN_Read),
        DECLARE_NAPI_METHOD("ReadFD", pcan_CAN_ReadFD),
        DECLARE_NAPI_METHOD("Write", pcan_CAN_Write),
        DECLARE_NAPI_METHOD("WriteFD", pcan_CAN_Write),
        DECLARE_NAPI_METHOD("GetValue", pcan_CAN_GetValue),
        DECLARE_NAPI_METHOD("SetValue", pcan_CAN_SetValue),
        DECLARE_NAPI_METHOD("FilterMessages", pcan_CAN_FilterMessages),
        DECLARE_NAPI_METHOD("GetErrorText", pcan_CAN_GetErrorText),
        // Additional functions
        DECLARE_NAPI_METHOD("EnableEvent", pcan_CAN_EnableEvent),
        DECLARE_NAPI_METHOD("DisableEvent", pcan_CAN_DisableEvent),
        DECLARE_NAPI_METHOD("AcceptanceFilter29Bit", pcan_CAN_AcceptanceFilter29Bit),
        DECLARE_NAPI_METHOD("AcceptanceFilter11Bit", pcan_CAN_AcceptanceFilter11Bit),
        DECLARE_NAPI_METHOD("ChannelInfo", pcan_CAN_ChannelInfo),
        DECLARE_NAPI_METHOD("TranslateBaud", pcan_CAN_TranslateBaud),
    };

    status = napi_define_properties(env, exports, descriptors_len, descriptors);
    assert(status == napi_ok);

    return exports;
}




void pcan_CAN_EventCallback(int channel)
{
    napi_status status = napi_generic_failure;

#ifdef PCAN_DEBUG
    printf("pcan_CAN_EventCallback()\n");
#endif

    status = napi_call_threadsafe_function(pcanCallback, 0, true);
    assert(status == napi_ok);

    return;
}




void pcan_CAN_EventFinalize(napi_env env, void* finalize_data, void* finalize_hint)
{
#ifdef PCAN_DEBUG
    printf("pcan_CAN_EventFinalize\n");
#endif
}




// ----------------------------------- // -----------------------------------
// Public functions


napi_value pcan_CAN_Initialize(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_INITIALIZE_ARGC;
    napi_value argv[CAN_INITIALIZE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_INITIALIZE_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments."
                              "This implementation of CAN_Initialize accepts Channel and "
                              "Btr0Btr1.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    uint32_t pcanBtr0Btr1;
    status = napi_get_value_uint32(env, argv[1], &pcanBtr0Btr1);
    assert(status == napi_ok);

    // Initialize CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Initialize(pcanChannel, pcanBtr0Btr1, 0, 0, 0);
   
    // Print results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Initialize:\n"
           "  pcanStatus = 0x%02X (%s)\n"
           "  pcanChannel = 0x%02X\n"
           "  pcanBtr0Btr1 = 0x%02X\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcanBtr0Btr1);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_Initialize: Error at CAN_Initialize.");
        return 0;
    }
    
    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_InitializeFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_INITIALIZEFD_ARGC;
    napi_value argv[CAN_INITIALIZEFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_INITIALIZEFD_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    char pcanBitrateFD[256] = { 0 };
    size_t pcanBitrateFDResult = 0;
    status = napi_get_value_string_utf8(env, argv[1], pcanBitrateFD,
                                        sizeof(pcanBitrateFD), &pcanBitrateFDResult);
    assert(status == napi_ok);

    // Initialize CAN bus, with flexible data rate capabilities
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_InitializeFD(pcanChannel, pcanBitrateFD);
    
    // Print results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_InitializeFD:\n"
           "  pcanStatus = 0x%02X (%s)\n"
           "  pcanChannel = 0x%02X\n"
           "  pcanBitrateFD = \"%s\"\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcanBitrateFD);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_InitializeFD: Error at CAN_InitializeFD.");
        return 0;
    }

    // Create N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_Uninitialize(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_UNINITIALIZE_ARGC;
    napi_value argv[CAN_UNINITIALIZE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_UNINITIALIZE_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Uninitialize CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Uninitialize(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Uninitialize: 0x%02X (%s)\n",
           pcanStatus, pcanStatusLookup(pcanStatus));
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_Uninitialize: Error at CAN_Uninitialize.");
        return 0;
    }

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_Reset(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_RESET_ARGC;
    napi_value argv[CAN_RESET_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_RESET_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Reset CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Reset(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Reset: 0x%02X (%s)\n",
           pcanStatus, pcanStatusLookup(pcanStatus));
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_Reset: Error at CAN_Reset.");
        return 0;
    }

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_GetStatus(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_GETSTATUS_ARGC;
    napi_value argv[CAN_GETSTATUS_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETSTATUS_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Get CAN status
    TPCANStatus pcan_can_status;
    pcan_can_status = CAN_GetStatus(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetStatus: 0x%02X (%s)\n",
           pcan_can_status, pcanStatusLookup(pcan_can_status));
#endif

    // Create a N-API value for the status and return it
    napi_value can_status;
    status = napi_create_uint32(env, pcan_can_status, &can_status);
    assert(status == napi_ok);

    return can_status;
}




napi_value pcan_CAN_Read(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and arguments list
    size_t argc = CAN_READ_ARGC;
    napi_value argv[CAN_READ_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_READ_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Read from CAN bus
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 };
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Read(pcanChannel, &msg, &timestamp);
    
    // Print the result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Read: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsg(&msg);
    pcanDumpTimestamp(&timestamp);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_Read");
        return 0;
    }

    // Create parent objects for read data
    napi_value pcanMessageBuffer;
    status = napi_create_object(env, &pcanMessageBuffer);
    assert(status == napi_ok);

    napi_value pcanTimestampBuffer;
    status = napi_create_object(env, &pcanTimestampBuffer);
    assert(status == napi_ok);

    // Copy read values into objects passed from N-API
    // MessageBuffer
    napi_value pcanMessageBufferID; // uint32
    napi_value pcanMessageBufferMsgtype; // uint32
    napi_value pcanMessageBufferLen; // uint32
    napi_value pcanMessageBufferData; // buffer
    void* pcanMessageBufferDataResult;
    
    status = napi_create_uint32(env, msg.ID, &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "id", pcanMessageBufferID);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.MSGTYPE, &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "msgtype", pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.LEN, &pcanMessageBufferLen);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "len", pcanMessageBufferLen);
    assert(status == napi_ok);

    status = napi_create_buffer_copy(env, msg.LEN, (void*)&(msg.DATA),
                                     (void*)&pcanMessageBufferDataResult,
                                     &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "data", pcanMessageBufferData);
    assert(status == napi_ok);

    // TimestampBuffer
    napi_value pcanTimestampBufferMillis; // uint32
    napi_value pcanTimestampBufferMillisOverflow; // uint32
    napi_value pcanTimestampBufferMicros; // uint32

    status = napi_create_uint32(env, timestamp.millis, &pcanTimestampBufferMillis);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanTimestampBuffer, "millis",
                                     pcanTimestampBufferMillis);

    status = napi_create_uint32(env, timestamp.millis_overflow,
                                &pcanTimestampBufferMillisOverflow);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanTimestampBuffer, "millis_overflow",
                                     pcanTimestampBufferMillisOverflow);

    status = napi_create_uint32(env, timestamp.micros, &pcanTimestampBufferMicros);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanTimestampBuffer, "micros",
                                     pcanTimestampBufferMicros);

    // Create a N-API value for the complete read data and return it
    napi_value pcanReadData;
    status = napi_create_object(env, &pcanReadData);
    assert(status == napi_ok);

    status = napi_set_named_property(env, pcanReadData, "message", pcanMessageBuffer);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanReadData, "timestamp", pcanTimestampBuffer);
    assert(status == napi_ok);
    
    return pcanReadData;
}




napi_value pcan_CAN_ReadFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_READFD_ARGC;
    napi_value argv[CAN_READFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_READFD_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Read from CAN bus
    TPCANMsgFD msg = { 0 };
    TPCANTimestampFD timestamp = 0;
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;

    pcanStatus = CAN_ReadFD(pcanChannel, &msg, &timestamp);

    // Dump results to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_ReadFD: 0x%02X (%s)\n", pcanStatus,
           pcanStatusLookup(pcanStatus));
    pcanDumpMsgFD(&msg);
    pcanDumpTimestampFD(timestamp);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_ReadFD");
        return 0;
    }

    // Create parent objects for read data
    napi_value pcanMessageBuffer;
    status = napi_create_object(env, &pcanMessageBuffer);
    assert(status == napi_ok);

    napi_value pcanTimestampBuffer;
    status = napi_create_object(env, &pcanTimestampBuffer);
    assert(status == napi_ok);

    // Copy read values into objects passed from N-API
    // argv[1] MessageBuffer
    napi_value pcanMessageBufferID; // uint32
    napi_value pcanMessageBufferMsgtype; // uint32
    napi_value pcanMessageBufferDLC; // uint32
    napi_value pcanMessageBufferData; // buffer
    void* pcanMessageBufferDataResult;
    
    status = napi_create_uint32(env, msg.ID, &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "id", pcanMessageBufferID);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.MSGTYPE, &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "msgtype", pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    
    status = napi_create_uint32(env, msg.DLC, &pcanMessageBufferDLC);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "dlc", pcanMessageBufferDLC);
    assert(status == napi_ok);

    status = napi_create_buffer_copy(env, pcanDLCDecode(msg.DLC), (void*)&(msg.DATA), (void*)&pcanMessageBufferDataResult, &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanMessageBuffer, "data", pcanMessageBufferData);
    assert(status == napi_ok);

    // argv[2] TimestampFD
    napi_value pcanTimestampFDValue; // uint64

    status = napi_create_uint32(env, timestamp, &pcanTimestampFDValue);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanTimestampBuffer, "timestamp", pcanTimestampFDValue);
    assert(status == napi_ok);

    // Create an N-API value for the complete read data and return it
    napi_value pcanReadDataFD;
    status = napi_create_object(env, &pcanReadDataFD);
    assert(status == napi_ok);

    status = napi_set_named_property(env, pcanReadDataFD, "message", pcanMessageBuffer);
    assert(status == napi_ok);
    status = napi_set_named_property(env, pcanReadDataFD, "timestamp", pcanTimestampBuffer);
    assert(status == napi_ok);
    
    return pcanReadDataFD;
}




napi_value pcan_CAN_Write(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_WRITE_ARGC;
    napi_value argv[CAN_WRITE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_WRITE_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBuffer
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
        napi_throw_type_error(env, 0, "Argument 1 (MessageBuffer) is not an object.");
        return 0;
    }

    // Copy message contents from N-API object to TPCANMsg struct
    TPCANMsg msg = { 0 };

    napi_value pcanMessageBufferID;
    status = napi_get_named_property(env, argv[1], "id", &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferID, &(msg.ID));
    assert(status == napi_ok);

    napi_value pcanMessageBufferMsgtype;
    status = napi_get_named_property(env, argv[1], "msgtype", &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferMsgtype, (uint32_t*)&(msg.MSGTYPE));
    assert(status == napi_ok);

    napi_value pcanMessageBufferLen;
    status = napi_get_named_property(env, argv[1], "len", &pcanMessageBufferLen);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferLen, (uint32_t*)&(msg.LEN));
    assert(status == napi_ok);

    napi_value pcanMessageBufferData;
    BYTE *msgTmpData;
    size_t msgTmpLen = 0;
    status = napi_get_named_property(env, argv[1], "data", &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_get_buffer_info(env, pcanMessageBufferData, (void**)&msgTmpData, &msgTmpLen);
    assert(status == napi_ok);

    if (msgTmpLen != msg.LEN)
    {
        napi_throw_error(env, 0, "Mismatch between LEN and actual data buffer length");
        return 0;
    }

    memcpy(msg.DATA, msgTmpData, msg.LEN);

    // Write to CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_Write(pcanChannel, &msg);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_Write: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsg(&msg);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_Write");
        return 0;
    }

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);
    
    return result;
}




napi_value pcan_CAN_WriteFD(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_WRITEFD_ARGC;
    napi_value argv[CAN_WRITEFD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_WRITEFD_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] MessageBufferFD
    napi_valuetype pcanMessageBufferType;
    status = napi_typeof(env, argv[1], &pcanMessageBufferType);
    assert(status == napi_ok);

    if (pcanMessageBufferType != napi_object)
    {
        napi_throw_type_error(env, 0, "Argument 1 (MessageBufferFD) is not an object.");
        return 0;
    }

    // Copy message contents from N-API object to TPCANMsgFD struct
    TPCANMsgFD msg = { 0 };

    napi_value pcanMessageBufferID;
    status = napi_get_named_property(env, argv[1], "id", &pcanMessageBufferID);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferID, &(msg.ID));
    assert(status == napi_ok);

    napi_value pcanMessageBufferMsgtype;
    status = napi_get_named_property(env, argv[1], "msgtype", &pcanMessageBufferMsgtype);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferMsgtype, (uint32_t*)&(msg.MSGTYPE));
    assert(status == napi_ok);

    napi_value pcanMessageBufferDLC;
    status = napi_get_named_property(env, argv[1], "dlc", &pcanMessageBufferDLC);
    assert(status == napi_ok);
    status = napi_get_value_uint32(env, pcanMessageBufferDLC, (uint32_t*)&(msg.DLC));
    assert(status == napi_ok);

    napi_value pcanMessageBufferData;
    BYTE *msgTmpData;
    size_t msgTmpLen = 0;
    status = napi_get_named_property(env, argv[1], "data", &pcanMessageBufferData);
    assert(status == napi_ok);
    status = napi_get_buffer_info(env, pcanMessageBufferData, (void**)&msgTmpData, &msgTmpLen);
    assert(status == napi_ok);

    if (msgTmpLen != pcanDLCDecode(msg.DLC))
    {
        napi_throw_error(env, 0, "Mismatch between DLC and actual data buffer length");
        return 0;
    }

    memcpy(msg.DATA, msgTmpData, pcanDLCDecode(msg.DLC));

    // Write to CAN bus
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_WriteFD(pcanChannel, &msg);
    
    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_WriteFD: 0x%02X (%s)\n", pcanStatus, pcanStatusLookup(pcanStatus));
    pcanDumpMsgFD(&msg);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_WriteFD");
        return 0;
    }

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_GetValue(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_GETVALUE_ARGC;
    napi_value argv[CAN_GETVALUE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETVALUE_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments. \n"
                              "The N-API implementation of CAN_GetValue accepts Channel, "
                              "Parameter, and Buffer.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] Parameter
    uint32_t pcanParameter;
    status = napi_get_value_uint32(env, argv[1], &pcanParameter);
    assert(status == napi_ok);

    // argv[2] Buffer
    napi_valuetype pcanBufferType;
    status = napi_typeof(env, argv[2], &pcanBufferType);
    assert(status == napi_ok);

    if (pcanBufferType != napi_object)
    {
        napi_throw_type_error(env, 0, "Argument 2 (Buffer) is not an object.");
        return 0;
    }

    // Argument 3 - BufferLength (obtained from pcanBuffer)
    BYTE *pcanBuffer = 0;
    size_t pcanBufferLength = 0;
    status = napi_get_buffer_info(env, argv[2], (void**)&pcanBuffer, &pcanBufferLength);
    assert(status == napi_ok);

    // Get requested parameter from CAN channel
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_GetValue(pcanChannel, pcanParameter, pcanBuffer, pcanBufferLength);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetValue: \n"
           "  pcanStatus       = 0x%02X (%s)\n"
           "  pcanChannel      = 0x%02X\n"
           "  pcanParameter    = 0x%02X (%s)\n"
           "  pcanBufferLength = 0x%02zX\n"
           "  pcanBuffer       = ",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcanParameter, pcanParameterLookup(pcanParameter),
           pcanBufferLength);
    pcanDumpBuffer(pcanBuffer, pcanBufferLength);
    printf("\n");
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_GetValue");
        return 0;
    }

    // Put value into the buffer object passed from N-API
    napi_value pcanBufferValue;
    status = napi_create_buffer(env, pcanBufferLength, (void**)&pcanBuffer, &pcanBufferValue);
    assert(status == napi_ok);

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_SetValue(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_SETVALUE_ARGC;
    napi_value argv[CAN_SETVALUE_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_SETVALUE_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments. \n"
                              "The N-API implementation of CAN_SetValue accepts Channel, "
                              "Parameter, and Buffer.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] Parameter
    uint32_t pcanParameter;
    status = napi_get_value_uint32(env, argv[1], &pcanParameter);
    assert(status == napi_ok);

    // argv[2] Buffer
    napi_valuetype pcanBufferType;
    status = napi_typeof(env, argv[2], &pcanBufferType);
    assert(status == napi_ok);

    if (pcanBufferType != napi_object)
    {
        napi_throw_type_error(env, 0, "Argument 2 (Buffer) is not an object.");
        return 0;
    }

    // Argument 3 - BufferLength (obtained from pcanBuffer)
    BYTE *pcanBuffer = 0;
    size_t pcanBufferLength = 0;
    
    status = napi_get_buffer_info(env, argv[2], (void**)&pcanBuffer, &pcanBufferLength);
    assert(status == napi_ok);

    // Set value for CAN channel
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, pcanParameter, pcanBuffer, pcanBufferLength);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_SetValue: \n"
           "  pcanStatus       = 0x%02X (%s)\n"
           "  pcanChannel      = 0x%02X\n"
           "  pcanParameter    = 0x%02X (%s)\n"
           "  pcanBufferLength = 0x%02zX\n"
           "  pcanBuffer       = ",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcanParameter, pcanParameterLookup(pcanParameter),
           pcanBufferLength);
    pcanDumpBuffer(pcanBuffer, pcanBufferLength);
    printf("\n");
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_SetValue");
        return 0;
    }

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




#if defined _WIN32
napi_value pcan_CAN_FilterMessages(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Retrieve callback info and argument list
    size_t argc = CAN_FILTERMESSAGES_ARGC;
    napi_value argv[CAN_FILTERMESSAGES_ARGC];
    
    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_FILTERMESSAGES_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] FromID
    uint32_t pcanFromID;
    status = napi_get_value_uint32(env, argv[1], &pcanFromID);
    assert(status == napi_ok);

    // argv[2] ToID
    uint32_t pcanToID;
    status = napi_get_value_uint32(env, argv[2], &pcanToID);
    assert(status == napi_ok);

    // argv[3] Mode
    uint32_t pcanMode;
    status = napi_get_value_uint32(env, argv[3], &pcanMode);
    assert(status == napi_ok);

    // Set or amend the CAN message filter ranges
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_FilterMessages(pcanChannel, pcanFromID, pcanToID, pcanMode);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_FilterMessages: \n"
           "  pcanStatus  = 0x%02X (%s)\n"
           "  pcanChannel = 0x%02X\n"
           "  pcanFromID  = 0x%08X\n"
           "  pcanToID    = 0x%08X\n"
           "  pcanMode    = 0x%02X\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcanFromID,
           pcanToID,
           pcanMode);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_FilterMessages");
        return 0;
    }

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}


#elif defined __APPLE__
napi_value pcan_CAN_FilterMessages(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;
 
    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, 0, &result);
    assert(status == napi_ok);

    return result;
}


#endif




napi_value pcan_CAN_GetErrorText(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;
    
    // Retrieve callback info and argument list
    size_t argc = CAN_GETERRORTEXT_ARGC;
    napi_value argv[CAN_GETERRORTEXT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_GETERRORTEXT_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Error
    TPCANStatus pcanError;
    status = napi_get_value_uint32(env, argv[0], &pcanError);
    assert(status == napi_ok);

    // argv[1] Language
    uint32_t pcanLanguage;
    status = napi_get_value_uint32(env, argv[1], &pcanLanguage);
    assert(status == napi_ok);

    // Buffer
    char pcanBuffer[256];

    // Call API to get error text
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_GetErrorText(pcanError, pcanLanguage, pcanBuffer);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_GetErrorText:\n"
           "  pcanStatus    = %i (%s)\n"
           "  pcanError     = %i\n"
           "  pcanLanguage  = %i\n"
           "  pcanBuffer    = \"%s\"\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanError,
           pcanLanguage,
           pcanBuffer);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_GetErrorText");
        return 0;
    }

    // Create an N-API value for the result and return it
    napi_value pcanBufferValue;
    status = napi_create_string_utf8(env, pcanBuffer, NAPI_AUTO_LENGTH, &pcanBufferValue);
    assert(status == napi_ok);

    return pcanBufferValue;
}




napi_value pcan_CAN_EnableEvent(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ENABLEEVENT_ARGC;
    napi_value argv[CAN_ENABLEEVENT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ENABLEEVENT_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] Callback
    napi_valuetype pcanCallbackType;
    status = napi_typeof(env, argv[1], &pcanCallbackType);
    assert(status == napi_ok);

    if (pcanCallbackType != napi_function)
    {
        napi_throw_type_error(env, 0, "Argument 1 (Callback) is not a function.");
        return 0;
    }

    // Create resource name string, used below
    napi_value asyncResourceName;
    status = napi_create_string_utf8(env, "pcanCallback",
                                     NAPI_AUTO_LENGTH, &asyncResourceName);
    assert(status == napi_ok);

    // Create thread-safe function to use as a callback
    status = napi_create_threadsafe_function(env,
                                             argv[1], // func
                                             0, // async_resource
                                             asyncResourceName,
                                             0, // max_queue_size
                                             1, // initial_thread_count
                                             0, // thread_finalize_data
                                             pcan_CAN_EventFinalize,
                                             0, // context
                                             0, // call_js_cb
                                             &pcanCallback); // result
    assert(status == napi_ok);

    // Enable CAN Bus receive event
    int pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = pcanEventEnable(pcanChannel, pcan_CAN_EventCallback);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_EnableEvent: 0x%02X\n", pcanStatus);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_EnableEvent");
        return 0;
    }

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




napi_value pcan_CAN_DisableEvent(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_DISABLEEVENT_ARGC;
    napi_value argv[CAN_DISABLEEVENT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_DISABLEEVENT_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // Disable CAN Bus receive event
    int pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = pcanEventDisable(pcanChannel);

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_DisableEvent: 0x%02X\n", pcanStatus);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_DisableEvent");
        return 0;
    }

    // Try to release the threadsafe function
    status = napi_unref_threadsafe_function(env, pcanCallback);
    assert(status == napi_ok);
    status = napi_release_threadsafe_function(pcanCallback, napi_tsfn_abort);
    assert(status == napi_ok);

    // Create a N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;
}




#if defined _WIN32
napi_value pcan_CAN_AcceptanceFilter11Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ACCEPTANCEFILTER11BIT_ARGC;
    napi_value argv[CAN_ACCEPTANCEFILTER11BIT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ACCEPTANCEFILTER11BIT_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] AcceptanceCode
    uint32_t pcan_AcceptanceCode;
    status = napi_get_value_uint32(env, argv[1], &pcan_AcceptanceCode);
    assert(status == napi_ok);

    // argv[2] AcceptanceMask
    uint32_t pcan_AcceptanceMask;
    status = napi_get_value_uint32(env, argv[2], &pcan_AcceptanceMask);
    assert(status == napi_ok);

    // Build 64-bit acceptance filter value
    uint64_t pcan_AcceptanceFilter = 0;
    pcan_AcceptanceFilter |= ((uint64_t)pcan_AcceptanceCode << 0x20);
    pcan_AcceptanceFilter |= ((uint64_t)(~pcan_AcceptanceMask));
    // Note that pcan_AcceptanceMask's bits are flipped from "don't care"
    // to "do care"

    // Set the acceptance filter through the API
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, PCAN_ACCEPTANCE_FILTER_11BIT,
                               &pcan_AcceptanceFilter, sizeof(pcan_AcceptanceFilter));

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_AcceptanceFilter11Bit:\n"
           "  pcanStatus            = 0x%02X (%s)\n"
           "  pcanChannel           = 0x%02X\n"
           "  pcan_AcceptanceCode   = 0x%08X\n"
           "  pcan_AcceptanceMask   = 0x%08X\n"
           "  pcan_AcceptanceFilter = 0x%016llX\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcan_AcceptanceCode,
           pcan_AcceptanceMask,
           pcan_AcceptanceFilter);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_AcceptanceFilter11Bit");
        return 0;
    }

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;    
}


#elif defined __APPLE__
napi_value pcan_CAN_AcceptanceFilter11Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, 0, &result);
    assert(status == napi_ok);

    return result;    
}


#endif




#if defined _WIN32
napi_value pcan_CAN_AcceptanceFilter29Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_ACCEPTANCEFILTER29BIT_ARGC;
    napi_value argv[CAN_ACCEPTANCEFILTER29BIT_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_ACCEPTANCEFILTER29BIT_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Channel
    uint32_t pcanChannel;
    status = napi_get_value_uint32(env, argv[0], &pcanChannel);
    assert(status == napi_ok);

    // argv[1] AcceptanceCode
    uint32_t pcan_AcceptanceCode;
    status = napi_get_value_uint32(env, argv[1], &pcan_AcceptanceCode);
    assert(status == napi_ok);

    // argv[2] AcceptanceMask
    uint32_t pcan_AcceptanceMask;
    status = napi_get_value_uint32(env, argv[2], &pcan_AcceptanceMask);
    assert(status == napi_ok);

    // Build 64-bit acceptance filter value
    uint64_t pcan_AcceptanceFilter = 0;
    pcan_AcceptanceFilter |= ((uint64_t)pcan_AcceptanceCode << 0x20);
    pcan_AcceptanceFilter |= ((uint64_t)(~pcan_AcceptanceMask));
    // Note that pcan_AcceptanceMask's bits are flipped from "don't care"
    // to "do care"

    // Set the acceptance filter through the API
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    pcanStatus = CAN_SetValue(pcanChannel, PCAN_ACCEPTANCE_FILTER_29BIT,
                               &pcan_AcceptanceFilter, sizeof(pcan_AcceptanceFilter));

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_AcceptanceFilter29Bit:\n"
           "  pcanStatus            = 0x%02X (%s)\n"
           "  pcanChannel           = 0x%02X\n"
           "  pcan_AcceptanceCode   = 0x%08X\n"
           "  pcan_AcceptanceMask   = 0x%08X\n"
           "  pcan_AcceptanceFilter = 0x%016llX\n",
           pcanStatus, pcanStatusLookup(pcanStatus),
           pcanChannel,
           pcan_AcceptanceCode,
           pcan_AcceptanceMask,
           pcan_AcceptanceFilter);
#endif

    // Throw error, if any
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_AcceptanceFilter29Bit");
        return 0;
    }

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, pcanStatus, &result);
    assert(status == napi_ok);

    return result;    
}


#elif defined __APPLE__
napi_value pcan_CAN_AcceptanceFilter29Bit(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Create an N-API value for the result and return it
    napi_value result;
    status = napi_create_uint32(env, 0, &result);
    assert(status == napi_ok);

    return result;    
}


#endif




napi_value pcan_CAN_ChannelInfo(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;
    int i; // for loop variable

    // Get callback info and argument list
    size_t argc = CAN_CHANNELINFO_ARGC;
    napi_value argv;

    status = napi_get_cb_info(env, info, &argc, &argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_CHANNELINFO_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Get channel count
    TPCANStatus pcanStatus = PCAN_ERROR_UNKNOWN;
    int32_t pcanAttachedChannelsCount = 0;

    pcanStatus = CAN_GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS_COUNT,
                              &pcanAttachedChannelsCount, sizeof(pcanAttachedChannelsCount));
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_ChannelInfo: Error at CAN_GetValue(PCAN_ATTACHED_CHANNELS_COUNT).");
        return 0;
    }
    
    // Check that there are > 0 channels available
    if (pcanAttachedChannelsCount == 0)
    {
        napi_throw_error(env, "PCAN_ERROR_NOCHANNELS", "No PCAN channels available");
        return 0;
    }

#ifdef PCAN_DEBUG
    printf("pcan_CAN_ChannelInfo: %i channel(s) available\n", pcanAttachedChannelsCount);
#endif

    // Create empty channel list
    // Use of malloc and free here violates CSLLC Rule 163
    TPCANChannelInformation *pcanAttachedChannels = 0;
    size_t pcanAttachedChannels_sz;
    pcanAttachedChannels_sz = sizeof(TPCANChannelInformation) * pcanAttachedChannelsCount;
    pcanAttachedChannels = malloc(pcanAttachedChannels_sz);

    if (pcanAttachedChannels == 0)
    {
        napi_throw_error(env, 0, "Error allocating memory for TPCANChannelInformation.");
        return 0;
    }

    memset(pcanAttachedChannels, 0, pcanAttachedChannels_sz);

    // Get channel list
    pcanStatus = CAN_GetValue(PCAN_NONEBUS, PCAN_ATTACHED_CHANNELS,
                              pcanAttachedChannels, pcanAttachedChannels_sz);
    if (pcanStatus != PCAN_ERROR_OK)
    {
        napi_throw_error(env, pcanStatusLookup(pcanStatus),
                         "pcan_CAN_ChannelInfo: Error at CAN_GetValue(PCAN_ATTACHED_CHANNELS).");
        return 0;
    }

    // Print result to console
#ifdef PCAN_DEBUG
    for (i = 0; i < pcanAttachedChannelsCount; i++)
    {
        pcanDumpChannelInfo(&(pcanAttachedChannels[i]));
    }
#endif

    // Create parent N-API object for channel info array
    napi_value channelInfo;
    status = napi_create_array(env, &channelInfo);
    assert(status == napi_ok);

    napi_value channelInfoElement;
    napi_value channelInfoElementChannelHandle;
    napi_value channelInfoElementDeviceType;
    napi_value channelInfoElementControllerNumber;
    napi_value channelInfoElementDeviceFeatures;
    napi_value channelInfoElementDeviceName;
    napi_value channelInfoElementDeviceID;
    napi_value channelInfoElementChannelCondition;

    for (i = 0; i < pcanAttachedChannelsCount; i++)
    {
        // Create N-API object for array element
        status = napi_create_object(env, &channelInfoElement);
        assert(status == napi_ok);

        // Assign properties
        // - Channel handle
        status = napi_create_uint32(env, pcanAttachedChannels[i].channel_handle,
                                   &channelInfoElementChannelHandle);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "channel_handle",
                                         channelInfoElementChannelHandle);
        assert(status == napi_ok);

        // - Device type
        status = napi_create_uint32(env, pcanAttachedChannels[i].device_type,
                                    &channelInfoElementDeviceType);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "device_type",
                                         channelInfoElementDeviceType);
        assert(status == napi_ok);

        // - Controller number
        status = napi_create_uint32(env, pcanAttachedChannels[i].controller_number,
                                    &channelInfoElementControllerNumber);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "controller_number",
                                          channelInfoElementControllerNumber);
        assert(status == napi_ok);

        // - Device features
        status = napi_create_uint32(env, pcanAttachedChannels[i].device_features,
                                    &channelInfoElementDeviceFeatures);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "device_features",
                                          channelInfoElementDeviceFeatures);
        assert(status == napi_ok);
        
        // - Device name
        status = napi_create_string_utf8(env, pcanAttachedChannels[i].device_name,
                                         NAPI_AUTO_LENGTH, &channelInfoElementDeviceName);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "device_name",
                                          channelInfoElementDeviceName);

        // - Device ID
        status = napi_create_uint32(env, pcanAttachedChannels[i].device_id,
                                    &channelInfoElementDeviceID);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "device_id",
                                          channelInfoElementDeviceID);

        // - Channel condition
        status = napi_create_uint32(env, pcanAttachedChannels[i].channel_condition,
                                    &channelInfoElementChannelCondition);
        assert(status == napi_ok);
        status = napi_set_named_property(env, channelInfoElement, "channel_condition",
                                          channelInfoElementChannelCondition);

        // Add complete element to array
        status = napi_set_element(env, channelInfo, i, channelInfoElement);
        assert(status == napi_ok);
    }

    // Free dynamically allocated memory
    free(pcanAttachedChannels);

    // Return the N-API object
    return channelInfo;
}




napi_value pcan_CAN_TranslateBaud(napi_env env, napi_callback_info info)
{
    napi_status status = napi_generic_failure;

    // Get callback info and argument list
    size_t argc = CAN_TRANSLATEBAUD_ARGC;
    napi_value argv[CAN_TRANSLATEBAUD_ARGC];

    status = napi_get_cb_info(env, info, &argc, argv, 0, 0);
    assert(status == napi_ok);

    if (argc != CAN_TRANSLATEBAUD_ARGC)
    {
        napi_throw_type_error(env, 0, "Incorrect number of arguments.");
        return 0;
    }

    // Retrieve arguments
    // argv[0] Integer baud
    uint32_t baudInt;
    status = napi_get_value_uint32(env, argv[0], &baudInt);
    assert(status == napi_ok);

    // Translate into a PCAN-Basic constant
    TPCANBaudrate baudAPI;
    baudAPI = pcanTranslateBaud(baudInt);

    if (baudAPI == 0)
    {
        napi_throw_error(env, 0, "Unknown CAN baud specified.");
        return 0;
    }

    // Print result to console
#ifdef PCAN_DEBUG
    printf("pcan_CAN_TranslateBaud: baudInt = %i, baudAPI = 0x%04X\n",
           baudInt, baudAPI);
#endif

    // Create N-API value to return
    napi_value baudResult;
    status = napi_create_uint32(env, baudAPI, &baudResult);
    assert(status == napi_ok);

    return baudResult;
}

