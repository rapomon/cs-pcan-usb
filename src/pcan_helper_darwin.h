#ifndef _PCAN_HELPER_DARWIN_
#define _PCAN_HELPER_DARWIN_

// Definitions brought over from PCANBasic.h that were missing in PCBUSB.h for macOS

// Describes an available PCAN channel
typedef struct tagTPCANChannelInformation
{
    TPCANHandle channel_handle;                 // PCAN channel handle
    TPCANDevice device_type;                    // Kind of PCAN device
    BYTE controller_number;                     // CAN-Controller number
    DWORD device_features;                      // Device capabilities flag
    char device_name[MAX_LENGTH_HARDWARE_NAME]; // Device name
    DWORD device_id;                            // Device number
    DWORD channel_condition;                    // Availability status of a PCAN-Channel
} TPCANChannelInformation;


#endif // _PCAN_HELPER_DARWIN_
