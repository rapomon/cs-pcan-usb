/* N-API interface for PEAK-System PCAN-Basic

   N-API callable functions that directly correspond to PCAN-Basic functions, 
   and additional wrapper functions

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#ifndef _PCAN_H_
#define _PCAN_H_


//#define PCAN_DEBUG
// Uncomment to enable debugging messages, which may affect timing or
// performance

//#define PCAN_NO_NAPI
// Uncomment to disable declarations for functions that use N-API.
// This is required when compiling test programs that rely solely on the Win32
// and PCAN-Basic APIs.


#ifndef PCAN_NO_NAPI
#include <node_api.h> // provide N-API types
#endif


// ----------------------------------- // -----------------------------------
// Definitions

#define CAN_INITIALIZE_ARGC (2)
#define CAN_INITIALIZEFD_ARGC (2)
#define CAN_UNINITIALIZE_ARGC (1)
#define CAN_RESET_ARGC (1)
#define CAN_GETSTATUS_ARGC (1)
#define CAN_READ_ARGC (1)
#define CAN_READFD_ARGC (1)
#define CAN_WRITE_ARGC (2)
#define CAN_WRITEFD_ARGC (2)
#define CAN_GETVALUE_ARGC (3)
#define CAN_SETVALUE_ARGC (3)
#define CAN_FILTERMESSAGES_ARGC (4)
#define CAN_GETERRORTEXT_ARGC (2)
#define CAN_ENABLEEVENT_ARGC (2)
#define CAN_DISABLEEVENT_ARGC (1)
#define CAN_ACCEPTANCEFILTER11BIT_ARGC (3)
#define CAN_ACCEPTANCEFILTER29BIT_ARGC (3)
#define CAN_CHANNELINFO_ARGC (0)
#define CAN_TRANSLATEBAUD_ARGC (1)


// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions


// Wrapper callback function that calls the napi_threadsafe_function specified
// in pcan_CAN_EnableEvent
void pcan_CAN_EventCallback(int channel);


// Initialize N-API module
#ifndef PCAN_NO_NAPI
napi_value Init(napi_env env, napi_value exports);
#endif


// Dummy finalize callback for the threadsafe function used by
// pcan_CAN_EventCallback
#ifndef PCAN_NO_NAPI
void pcan_CAN_EventFinalize(napi_env env, void* finalize_data, void* finalize_hint);
#endif




// ----------------------------------- // -----------------------------------
// Public functions

// Initialize CAN bus given a channel and baud
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANBaudrate Btr0Btr1 (uint32)
// Note that this does not accept additional arguments for non-PnP CAN adapters
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_Initialize(napi_env env, napi_callback_info info);
#endif


// Initialize CAN bus for flexible data rate (FD) given a channel and bitrate
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANBitrateFD BitrateFD (string)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_InitializeFD(napi_env env, napi_callback_info info);
#endif


// Uninitialize CAN bus given a channel that has been initialized
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_Uninitialize(napi_env env, napi_callback_info info);
#endif


// Reset CAN bus given a channel
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_Reset(napi_env env, napi_callback_info info);
#endif


// Get status of CAN bus given a channel
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_GetStatus(napi_env env, napi_callback_info info);
#endif


// Read from CAN bus RX buffer, copying data into message and timestamp
// structures
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns N-API object containing 'message' and 'timestamp' objects,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_Read(napi_env env, napi_callback_info info);
#endif


// Read from FD CAN bus RX buffer, copying data into message and
// timestamp structures
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns N-API object containing 'message' and 'timestamp' objects,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_ReadFD(napi_env env, napi_callback_info info);
#endif


// Write to CAN bus TX buffer, copying data from message structure
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANMsg *MessageBuffer (buffer)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_Write(napi_env env, napi_callback_info info);
#endif


// Write to FD CAN bus TX buffer, copying data from message structure
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANMsgFD *MessageBuffer (buffer)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_WriteFD(napi_env env, napi_callback_info info);
#endif


// Retrieve a parameter from the CAN library, copying it into a generic
// buffer. Buffer size requirement varies based on the argument requested.
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANParameter Parameter (uint32)
// - void *Buffer (buffer)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure. Parameter value is returned in Buffer.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_GetValue(napi_env env, napi_callback_info info);
#endif


// Set a parameter in the CAN library, copying from a buffer
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - TPCANParameter Parameter (uint32)
// - void *Buffer (buffer)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error si thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_SetValue(napi_env env, napi_callback_info info);
#endif


// Control the PCAN driver's simple message filtering function
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - DWORD FromID (uint32)
// - DWORD ToID (uint32)
// - TPCANMode Mode (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_FilterMessages(napi_env env, napi_callback_info info);
#endif


// Given a TPCANStatus error code, fill provided buffer with error text
// Arguments passed through N-API:
// - TPCANStatus Error (uint32)
// - WORD Language (uint32)
// Returns error text as string wrapped as napi_value. Error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_GetErrorText(napi_env env, napi_callback_info info);
#endif


// Given an N-API callback function, enable the message receive Win32 event
// and spawn a worker thread that calls the callback function when a new message
// is received.
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - function callback (N-API)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_EnableEvent(napi_env env, napi_callback_info info);
#endif


// Disable a previously enabled message receive Win32 event.
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_DisableEvent(napi_env env, napi_callback_info info);
#endif


// Manually set the acceptance filter (11-bit) for the CAN controller, given
// an acceptance code and acceptance mask.
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - DWORD AcceptanceCode (uint32)
// - DWORD AcceptanceMask (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_AcceptanceFilter11Bit(napi_env env, napi_callback_info info);
#endif


// Manually set the acceptance filter (29-bit) for the CAN controller, given
// an acceptance code and acceptance mask.
// Arguments passed through N-API:
// - TPCANHandle Channel (uint32)
// - DWORD AcceptanceCode (uint32)
// - DWORD AcceptanceMask (uint32)
// Returns TPCANStatus from PCAN-Basic API call, wrapped in napi_value,
// and error is thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_AcceptanceFilter29Bit(napi_env env, napi_callback_info info);
#endif


// Return list of available PCAN channels on the system
// Arguments passed through N-API:
// - (none)
// Returns N-API array of TPCANChannelInfo-like objects, or upon failure. Error
// may be thrown upon failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_ChannelInfo(napi_env env, napi_callback_info info);
#endif


// Return uint32 API constant corresponding to an integer CAN baud defined in
// PCANBasic.h, which can be passed to pcan_CAN_Initialize.
// Arguments passed through N-API:
// - uint32_t baud (uint32)
// Returns uint32 constant on success or 0 on failure.
#ifndef PCAN_NO_NAPI
napi_value pcan_CAN_TranslateBaud(napi_env env, napi_callback_info info);
#endif



#endif /* _PCAN_H_ */

